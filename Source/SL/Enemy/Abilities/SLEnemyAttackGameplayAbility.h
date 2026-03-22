// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SL/Abilities/SLGameplayAbility.h"
#include "SL/AnimNotify/AnimNotifyState_AttackTrace.h"
#include "SLEnemyAttackGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class SL_API USLEnemyAttackGameplayAbility : public USLGameplayAbility
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UFUNCTION()
	virtual void OnMontageCompleted();

	UFUNCTION()
	virtual void OnMontageInterrupted();

	UFUNCTION()
	virtual void OnHitEventReceived(FGameplayEventData Payload);
	
	virtual void UpdateWarpTarget();
	virtual	void ApplyHit(const UAnimNotifyState_AttackTrace* Notify);
	
protected:
	void StartTracking(float Interval = 0.01f);
	void StopTracking();
	void PlayMontageAndWaitHitEvent(class UAnimMontage* InMontage, FGameplayTag InEventTag);
	
protected:
	UPROPERTY(EditAnywhere, Category = "Tracking")
	FName WarpTargetName = TEXT("Target");

	UPROPERTY(EditAnywhere, Category = "Tracking")
	float WarpOffsetDistance = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TSubclassOf<UGameplayEffect> DamageEffectClass;
	
	UPROPERTY()
	TSet<AActor*> AlreadyHitActors;

private:
	FTimerHandle TrackingTimerHandle;
};