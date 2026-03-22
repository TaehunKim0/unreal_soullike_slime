// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SLEnemyAttackGameplayAbility.h"
#include "SL/Abilities/SLGameplayAbility.h"
#include "GA_RunAndVerticalSlash.generated.h"

struct FAIRequestID;
struct FPathFollowingResult;
/**
 * 
 */
UCLASS()
class SL_API UGA_RunAndVerticalSlash : public USLEnemyAttackGameplayAbility
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
protected:
	void StartAttackMontage(FAIRequestID RequestID, const FPathFollowingResult& Result);

protected:
	UPROPERTY(EditAnywhere, Category="Animation")
	TObjectPtr<class UAnimMontage> AttackMontage;

	UPROPERTY(EditAnywhere, Category="Speed")
	float RunSpeed = 1000.f;

private:
	float OriginWalkSpeed = 0.f;
};