// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "SL/Enemy/AI/SLBTTask_BlueprintBase.h"
#include "BTT_ActivateAbility.generated.h"

/**
 * 
 */
UCLASS()
class SL_API UBTT_ActivateAbility : public USLBTTask_BlueprintBase
{
	GENERATED_BODY()

public:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	void HandleAbilityEnded(const FAbilityEndedData& EndData);

protected:
	UPROPERTY(EditAnywhere)
	FGameplayTag AbilityTag;

private:
	UPROPERTY()
	UBehaviorTreeComponent* CachedOwnerComp;
    
	FGameplayAbilitySpecHandle ActiveHandle;
};
