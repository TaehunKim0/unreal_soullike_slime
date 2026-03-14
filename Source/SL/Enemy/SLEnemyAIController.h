// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "SLEnemyAIController.generated.h"

/**
 * 
 */
UCLASS()
class SL_API ASLEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	ASLEnemyAIController();

protected:
	virtual void OnPossess(APawn* InPawn) override;

protected:
	UFUNCTION(BlueprintCallable, Category = "AI")
	void OnPlayerDetected(APawn* PlayerPawn);

private:
	UPROPERTY(EditAnywhere, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;
};