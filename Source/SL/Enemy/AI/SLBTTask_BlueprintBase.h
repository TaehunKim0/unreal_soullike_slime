// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlueprintBase.h"
#include "SLBTTask_BlueprintBase.generated.h"

class USLAbilitySystemComponent;
/**
 * 
 */
UCLASS()
class SL_API USLBTTask_BlueprintBase : public UBTTask_BlueprintBase
{
	GENERATED_BODY()

public:
	AActor* GetSelfActor(UBehaviorTreeComponent& OwnerComp);
	USLAbilitySystemComponent* GetSLASC(UBehaviorTreeComponent& OwnerComp);
};
