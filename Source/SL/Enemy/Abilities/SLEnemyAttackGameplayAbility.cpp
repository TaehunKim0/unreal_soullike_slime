// Fill out your copyright notice in the Description page of Project Settings.


#include "SL/Enemy/Abilities/SLEnemyAttackGameplayAbility.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "SL/AnimNotify/AnimNotifyState_AttackTrace.h"
#include "SL/Interface/IDamageable.h"
#include "SL/Util/SLUtils.h"

void USLEnemyAttackGameplayAbility::ActivateAbility(FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	AlreadyHitActors.Empty();
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void USLEnemyAttackGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                               const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                               bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	StopTracking();
}

void USLEnemyAttackGameplayAbility::StartTracking(float Interval)
{
	UpdateWarpTarget();
	GetWorld()->GetTimerManager().SetTimer(TrackingTimerHandle, this, &USLEnemyAttackGameplayAbility::UpdateWarpTarget, Interval, true);
}

void USLEnemyAttackGameplayAbility::UpdateWarpTarget()
{
	AActor* OwningActor = GetOwningActorFromActorInfo();
	AActor* TargetActor = SLUtil::GetActorFromBlackboard(this, TEXT("TargetActor"));

	if (OwningActor && TargetActor)
	{
		FVector TargetLoc = TargetActor->GetActorLocation();
		FVector OwnerLoc = OwningActor->GetActorLocation();
		FVector Dir = (TargetLoc - OwnerLoc).GetSafeNormal2D();

		FVector WarpTargetLoc = TargetLoc - (Dir * WarpOffsetDistance);
		SLUtil::UpdateWarpTarget(OwningActor, WarpTargetName, WarpTargetLoc);
        
		OwningActor->SetActorRotation(Dir.Rotation());
	}
}

void USLEnemyAttackGameplayAbility::ApplyHit(const UAnimNotifyState_AttackTrace* Notify)
{
	if (!Notify) return;

	AActor* OwningActor = GetOwningActorFromActorInfo();
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (!OwningActor || !SourceASC) return;

	USkeletalMeshComponent* MeshComp = OwningActor->FindComponentByClass<USkeletalMeshComponent>();
	if (!MeshComp) return;

	FTransform MeshTransform = MeshComp->GetComponentTransform();
	FVector CenterLocation = MeshTransform.TransformPosition(Notify->TraceOffset);
	FQuat MeshQuat = MeshTransform.GetRotation();
	FQuat TraceRotation = MeshQuat * Notify->TraceRotation.Quaternion();
	
	FCollisionShape Shape;
	switch (Notify->TraceType)
	{
	case EMeleeTraceType::Sphere:
		Shape = FCollisionShape::MakeSphere(Notify->TraceRadius);
		break;

	case EMeleeTraceType::Capsule:
		Shape = FCollisionShape::MakeCapsule(Notify->TraceRadius, Notify->TraceDistance * 0.5f);
		TraceRotation = TraceRotation * FRotator(0.f, 90.f, 90.f).Quaternion();
		break;

	case EMeleeTraceType::Box:
		Shape = FCollisionShape::MakeBox(FVector(Notify->TraceDistance * 0.5f, Notify->TraceRadius, Notify->TraceRadius));
		break;
	}

	TArray<FHitResult> HitResults;
	bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults, 
		CenterLocation, 
		CenterLocation, 
		TraceRotation, 
		ECC_GameTraceChannel1, 
		Shape, 
		SLUtil::GetIgnoreParams(OwningActor)
	);

#if WITH_EDITOR
	if (Notify->bDrawDebug)
	{
		SLUtil::DrawDebugAttackShape(GetWorld(), CenterLocation, CenterLocation, TraceRotation, Notify->TraceType, Shape, bHit);
	}
#endif

    if (!bHit) return;
    for (const FHitResult& Hit : HitResults)
    {
        AActor* HitActor = Hit.GetActor();
        if (!HitActor || AlreadyHitActors.Contains(HitActor)) continue;

        if (SLUtil::CheckAndHandleParry(OwningActor, HitActor, Hit, Notify->bCanNeutralize))
        {
            StopTracking();
            K2_EndAbility();
            return;
        }

        if (HitActor->Implements<UDamageable>())
        {
            AlreadyHitActors.Add(HitActor);

            if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor))
            {
                FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
                ContextHandle.AddHitResult(Hit);

            	if (Notify->bPlayerKnockback)
            	{
            		FGameplayEventData Payload;
            		Payload.Instigator = OwningActor;
            		Payload.Target = HitActor;
            		Payload.ContextHandle = ContextHandle;

            		if (TargetASC)
            		{
            			TargetASC->HandleGameplayEvent(FGameplayTag::RequestGameplayTag(TEXT("Event.Hero.Knockback")), &Payload);
            		}
            	}
            	
                FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), ContextHandle);
                if (SpecHandle.IsValid())
                {
                    SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
                }
            }
        }
    }
}

void USLEnemyAttackGameplayAbility::PlayMontageAndWaitHitEvent(UAnimMontage* InMontage, FGameplayTag InEventTag)
{
	if (!InMontage) { K2_EndAbility(); return; }
	
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
	   this, NAME_None, InMontage, 1.0f, NAME_None, false, 1.0f
	);
	MontageTask->OnCompleted.AddDynamic(this, &USLEnemyAttackGameplayAbility::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &USLEnemyAttackGameplayAbility::OnMontageInterrupted);
	MontageTask->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, 
		FGameplayTag::RequestGameplayTag(InEventTag.GetTagName()),
		nullptr, 
		false,
		false
	);

	WaitEventTask->EventReceived.AddDynamic(this, &USLEnemyAttackGameplayAbility::OnHitEventReceived);
	WaitEventTask->ReadyForActivation();
}

void USLEnemyAttackGameplayAbility::StopTracking()
{
	GetWorld()->GetTimerManager().ClearTimer(TrackingTimerHandle);
    
	if (AAIController* AIC = SLUtil::GetAIControllerFromAbility(this))
	{
		AIC->SetFocus(nullptr);
	}
}

void USLEnemyAttackGameplayAbility::OnMontageCompleted()
{
	K2_EndAbility();
}

void USLEnemyAttackGameplayAbility::OnMontageInterrupted()
{
	K2_EndAbility();
}

void USLEnemyAttackGameplayAbility::OnHitEventReceived(FGameplayEventData Payload)
{
	StopTracking();
	if (const UAnimNotifyState_AttackTrace* Notify = Cast<UAnimNotifyState_AttackTrace>(Payload.OptionalObject))
	{
		ApplyHit(Notify);
	}
}