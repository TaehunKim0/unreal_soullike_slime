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
		MoveReq.SetAcceptanceRadius(70.f);
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

void UGA_RunAndVerticalSlash::StartAttackMontage( FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	if (AAIController* AIC = SLUtil::GetAIControllerFromAbility(this))
	{
		if (UPathFollowingComponent* PFollowComp = AIC->GetPathFollowingComponent())
		{
			PFollowComp->OnRequestFinished.RemoveAll(this);
		}
	}

	StartTracking();
	PlayMontageAndWaitHitEvent(AttackMontage, FGameplayTag::RequestGameplayTag(TEXT("Event.Enemy.Montage.RunAndVerticalSlash")));
}