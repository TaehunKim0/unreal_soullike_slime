// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SL/Abilities/SLGameplayAbility.h"
#include "GA_Retreat.generated.h"

struct FAIRequestID;
struct FPathFollowingResult;
/**
 * 
 */
UCLASS()
class SL_API UGA_Retreat : public USLGameplayAbility
{
	GENERATED_BODY()

protected:
	virtual void ActivateAbility(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	void OnMoveFinished(FAIRequestID RequestID, const FPathFollowingResult& Result);

protected:
	UPROPERTY(EditAnywhere, Category = "Retreat")
	float TargetDistance = 1000.0f;

	UPROPERTY(EditAnywhere, Category = "Retreat")
	float RetreatSpeedRate = 0.6f;
};
