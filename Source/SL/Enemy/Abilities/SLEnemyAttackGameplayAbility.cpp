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
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	AlreadyHitActors.Empty();
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
	GetWorld()->GetTimerManager().SetTimer(TrackingTimerHandle, this, &USLEnemyAttackGameplayAbility::UpdateWarpTarget, Interval, true);
	UpdateWarpTarget();
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
	// 고정 위치 Sweep (Start == End)
	bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults, 
		CenterLocation, 
		CenterLocation, 
		TraceRotation, 
		ECC_GameTraceChannel1, 
		Shape, 
		SLUtil::GetIgnoreParams(OwningActor)
	);

	// 디버그 드로잉 (실제 판정 시점에만 그림)
#if WITH_EDITOR
	if (Notify->bDrawDebug)
	{
		SLUtil::DrawDebugAttackShape(GetWorld(), CenterLocation, CenterLocation, TraceRotation, Notify->TraceType, Shape, bHit);
	}
#endif

    if (bHit)
    {
        for (const FHitResult& Hit : HitResults)
        {
            AActor* HitActor = Hit.GetActor();
            if (!HitActor || AlreadyHitActors.Contains(HitActor)) continue;

            // 4. 패리 체크 (노티파이의 넉백 옵션 전달)
            if (SLUtil::CheckAndHandleParry(OwningActor, HitActor, Hit, Notify->bCanKnockback))
            {
                // 패리 성공 시 즉시 추적 중단 및 어빌리티 취소
                StopTracking();
                K2_CancelAbility();
                return;
            }

            // 5. 중복 히트 방지 및 데미지 적용
            if (HitActor->Implements<UDamageable>())
            {
                AlreadyHitActors.Add(HitActor);

                if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor))
                {
                    FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
                    ContextHandle.AddHitResult(Hit);
                    
                    FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), ContextHandle);
                    if (SpecHandle.IsValid())
                    {
                        SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
                    }
                }
            }
        }
    }
}

void USLEnemyAttackGameplayAbility::PlayMontageAndWaitHitEvent(class UAnimMontage* InMontage,
                                                               FGameplayTag InEventTag)
{
	if (InMontage)
	{
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
	else
	{
		K2_EndAbility();
	}
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