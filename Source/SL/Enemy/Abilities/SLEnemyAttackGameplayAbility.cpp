// Fill out your copyright notice in the Description page of Project Settings.


#include "SL/Enemy/Abilities/SLEnemyAttackGameplayAbility.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "SL/Util/SLUtils.h"

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

void USLEnemyAttackGameplayAbility::ApplyHit()
{
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
	ApplyHit();
}
