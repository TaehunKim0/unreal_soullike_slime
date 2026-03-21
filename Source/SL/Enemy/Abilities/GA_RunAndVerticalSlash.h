// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SL/Abilities/SLGameplayAbility.h"
#include "GA_RunAndVerticalSlash.generated.h"

struct FAIRequestID;
struct FPathFollowingResult;
/**
 * 
 */
UCLASS()
class SL_API UGA_RunAndVerticalSlash : public USLGameplayAbility
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
protected:
	UFUNCTION()
	void OnHitEventReceived(FGameplayEventData Payload);
	
	void PerformMeleeTrace();

	void StartAttackMontage(FAIRequestID RequestID, const FPathFollowingResult& Result);
	
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageInterrupted();

protected:
	void UpdateTrackingTarget();
	void StopTracking();

protected:
	FTimerHandle TrackingTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditAnywhere, Category="Animation")
	TObjectPtr<class UAnimMontage> AttackMontage;
	
};
