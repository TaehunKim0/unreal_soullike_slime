// Fill out your copyright notice in the Description page of Project Settings.


#include "SL/Enemy/Abilities/GA_RunAndVerticalSlash.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_MoveToLocation.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "SL/Interface/IDamageable.h"
#include "SL/Util/SLUtils.h"

void UGA_RunAndVerticalSlash::ActivateAbility(FGameplayAbilitySpecHandle Handle,
                                              const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                              const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
    
	AActor* TargetActor = SLUtil::GetActorFromBlackboard(this, TEXT("TargetActor")); 
	if (!TargetActor)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AActor* OwningActor = GetOwningActorFromActorInfo();

	if (AAIController* AIC =SLUtil::GetAIControllerFromAbility(this))
	{
		AIC->SetFocus(TargetActor);

		// 2. 목적지 계산
		FVector DirToTarget = (TargetActor->GetActorLocation() - OwningActor->GetActorLocation()).GetSafeNormal2D();
		float AttackRangeOffset = 150.0f; // 적절한 공격 거리로 조정
		FVector TargetLocation = TargetActor->GetActorLocation() - (DirToTarget * AttackRangeOffset);

		// 3. AI MoveTo 실행 (Pathfinding 사용)
		FAIMoveRequest MoveReq;
		MoveReq.SetGoalLocation(TargetLocation);
		MoveReq.SetAcceptanceRadius(100.f);
		MoveReq.SetAllowPartialPath(true);

		FPathFollowingRequestResult MoveResult = AIC->MoveTo(MoveReq);
		
		// 1. 이미 도착했거나 요청이 즉시 끝난 경우
		if (MoveResult.Code == EPathFollowingRequestResult::AlreadyAtGoal)
		{
			StartAttackMontage(MoveResult.MoveId, FPathFollowingResult(EPathFollowingResult::Success));
			return;
		}
		// 2. 경로를 아예 못 찾는 경우 (에러 처리)
		else if (MoveResult.Code == EPathFollowingRequestResult::Failed)
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}
		
		if (UPathFollowingComponent* PFollowComp = AIC->GetPathFollowingComponent())
		{
			PFollowComp->OnRequestFinished.RemoveAll(this);
			PFollowComp->OnRequestFinished.AddUObject(this, &UGA_RunAndVerticalSlash::StartAttackMontage);
		}
	}
}

bool UGA_RunAndVerticalSlash::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UGA_RunAndVerticalSlash::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	StopTracking();
}

void UGA_RunAndVerticalSlash::OnHitEventReceived(FGameplayEventData Payload)
{
	StopTracking();
	PerformMeleeTrace();
}

void UGA_RunAndVerticalSlash::StartAttackMontage( FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	AActor* OwningActor = GetOwningActorFromActorInfo();
	AActor* TargetActor = SLUtil::GetActorFromBlackboard(this, TEXT("TargetActor"));

	if (AAIController* AIC = SLUtil::GetAIControllerFromAbility(this))
	{
		if (UPathFollowingComponent* PFollowComp = AIC->GetPathFollowingComponent())
		{
			PFollowComp->OnRequestFinished.RemoveAll(this);
		}
	}

	if (Result.Flags == EPathFollowingResult::Aborted)
	{
		// 이동이 중단되었다면 어빌리티 종료 (또는 그냥 공격 진행)
		// K2_EndAbility(); 
		// return;
	}
	
	if (TargetActor && OwningActor)
	{
		// 1. 즉시 방향 정렬
		FVector TargetLoc = TargetActor->GetActorLocation();
		FVector OwnerLoc = OwningActor->GetActorLocation();
		FVector CurrentDir = (TargetLoc - OwnerLoc).GetSafeNormal2D();

		FVector WarpTargetLoc = TargetLoc - (CurrentDir * 110.0f);

		OwningActor->SetActorRotation(CurrentDir.Rotation());

		SLUtil::UpdateWarpTarget(OwningActor, TEXT("Target"), WarpTargetLoc);
		GetWorld()->GetTimerManager().SetTimer(TrackingTimerHandle, this, &UGA_RunAndVerticalSlash::UpdateTrackingTarget, 0.01f, true);
	}
	
	if (AttackMontage)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		   this, NAME_None, AttackMontage, 1.0f, NAME_None, false, 1.0f
		);
		MontageTask->OnCompleted.AddDynamic(this, &UGA_RunAndVerticalSlash::OnMontageCompleted);
		MontageTask->OnBlendOut.AddDynamic(this, &UGA_RunAndVerticalSlash::OnMontageCompleted);
		MontageTask->OnInterrupted.AddDynamic(this, &UGA_RunAndVerticalSlash::OnMontageInterrupted);
		MontageTask->OnCancelled.AddDynamic(this, &UGA_RunAndVerticalSlash::OnMontageInterrupted);
		MontageTask->ReadyForActivation();

		UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this, 
			FGameplayTag::RequestGameplayTag(TEXT("Event.Enemy.Montage.RunAndVerticalSlash")),
			nullptr, 
			false,
			false
		);

		WaitEventTask->EventReceived.AddDynamic(this, &UGA_RunAndVerticalSlash::OnHitEventReceived);
		WaitEventTask->ReadyForActivation();
	}
	else
	{
		K2_EndAbility();
	}
}

void UGA_RunAndVerticalSlash::PerformMeleeTrace()
{
	AActor* OwningActor = GetOwningActorFromActorInfo();
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	
	if (!OwningActor) return;

	TArray<FHitResult> HitResults;
	
	FVector Right = OwningActor->GetActorRightVector();
	FVector Center = OwningActor->GetActorLocation() + (OwningActor->GetActorForwardVector() * 150.0f);

	FVector Start = Center - (Right * 150.0f);
	FVector End = Center + (Right * 150.0f);
	float Radius = 100.0f; // 휘두르는 궤적의 두께

	bool bHit = SLUtil::MeleeTraceMulti(GetWorld(), OwningActor, Start, End, Radius, HitResults, true);

	if (bHit)
	{
		TSet<AActor*> AlreadyHitActors;

		for (const FHitResult& Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();

			if (SLUtil::CheckAndHandleParry(OwningActor,HitActor, Hit, true))
			{
				K2_EndAbility();
				return;
			}
            
			if (HitActor && HitActor->Implements<UDamageable>() && !AlreadyHitActors.Contains(HitActor))
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
				DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 10.f, 8, FColor::Yellow, false, 2.0f);
			}
		}
	}
}

void UGA_RunAndVerticalSlash::UpdateTrackingTarget()
{
	AActor* OwningActor = GetOwningActorFromActorInfo();
	AActor* TargetActor = SLUtil::GetActorFromBlackboard(this, TEXT("TargetActor"));

	if (OwningActor && TargetActor)
	{
		FVector TargetLoc = TargetActor->GetActorLocation();
		FVector OwnerLoc = OwningActor->GetActorLocation();
		FVector Dir = (TargetLoc - OwnerLoc).GetSafeNormal2D();

		FVector WarpTargetLoc = TargetLoc - (Dir * 110.0f);
		SLUtil::UpdateWarpTarget(OwningActor, TEXT("Target"), WarpTargetLoc);
	}
}

void UGA_RunAndVerticalSlash::StopTracking()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(TrackingTimerHandle);

		if (AAIController* AIC =SLUtil::GetAIControllerFromAbility(this))
		{
			AIC->SetFocus(nullptr);
		}
	}
}

void UGA_RunAndVerticalSlash::OnMontageCompleted()
{
	StopTracking();
	K2_EndAbility();
}

void UGA_RunAndVerticalSlash::OnMontageInterrupted()
{
	StopTracking();
	K2_EndAbility();
}
