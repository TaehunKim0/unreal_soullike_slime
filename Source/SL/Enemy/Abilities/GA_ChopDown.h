// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SLEnemyAttackGameplayAbility.h"
#include "SL/Abilities/SLGameplayAbility.h"
#include "GA_ChopDown.generated.h"

/**
 * 
 */
UCLASS()
class SL_API UGA_ChopDown : public USLEnemyAttackGameplayAbility
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
protected:
	UPROPERTY(EditAnywhere, Category="Animation")
	TObjectPtr<class UAnimMontage> AttackMontage;
};
