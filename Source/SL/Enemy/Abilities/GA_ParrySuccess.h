// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SL/Abilities/SLGameplayAbility.h"
#include "GA_ParrySuccess.generated.h"

/**
 * 
 */
UCLASS()
class SL_API UGA_ParrySuccess : public USLGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_ParrySuccess();
	
	virtual void ActivateAbility(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;

protected:
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageInterrupted();

protected:
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UAnimMontage> ParrySuccessMontage;
};
