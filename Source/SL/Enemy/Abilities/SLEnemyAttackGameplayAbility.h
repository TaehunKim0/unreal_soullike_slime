// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SL/Abilities/SLGameplayAbility.h"
#include "SLEnemyAttackGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class SL_API USLEnemyAttackGameplayAbility : public USLGameplayAbility
{
	GENERATED_BODY()

public:
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UFUNCTION()
	virtual void OnMontageCompleted();

	UFUNCTION()
	virtual void OnMontageInterrupted();

	UFUNCTION()
	virtual void OnHitEventReceived(FGameplayEventData Payload);
	
protected:
	virtual void UpdateWarpTarget();
	virtual	void ApplyHit();
	
	void StartTracking(float Interval = 0.01f);
	void StopTracking();
	void PlayMontageAndWaitHitEvent(class UAnimMontage* InMontage, FGameplayTag InEventTag);
	
protected:
	FTimerHandle TrackingTimerHandle;
    
	UPROPERTY(EditAnywhere, Category = "Tracking")
	FName WarpTargetName = TEXT("Target");

	UPROPERTY(EditAnywhere, Category = "Tracking")
	float WarpOffsetDistance = 250.0f;
};