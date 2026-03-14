// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SL/Abilities/SLGameplayAbility.h"
#include "GA_Focus.generated.h"

USTRUCT(BlueprintType)
struct FSLTargetLockMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AActor> TargetActor = nullptr; 

	UPROPERTY(BlueprintReadWrite)
	bool bIsLockedOn = false;                 
};

/**
 * 
 */
UCLASS()
class SL_API UGA_Focus : public USLGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Focus();
	
protected:
	virtual void ActivateAbility(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	virtual void EndAbility(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	
private:
	UFUNCTION()
	void UpdateLockOn();

	UPROPERTY()
	TObjectPtr<AActor> TargetActor;

	UPROPERTY()
	TObjectPtr<APlayerController> OwnerPC;
};
