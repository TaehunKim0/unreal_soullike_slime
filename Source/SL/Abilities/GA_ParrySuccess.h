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

protected:
	virtual void ActivateAbility(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData*
							 TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UPROPERTY(EditAnywhere)
	TObjectPtr<class USoundBase> ParrySuccessSound;
};
