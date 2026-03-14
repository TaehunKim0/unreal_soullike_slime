// Fill out your copyright notice in the Description page of Project Settings.


#include "SL/Enemy/SLEnemyAIController.h"
#include "SL/Player/SLHeroController.h"

ASLEnemyAIController::ASLEnemyAIController()
{
	
}

void ASLEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (BehaviorTree)
	{
		RunBehaviorTree(BehaviorTree);
	}
}

void ASLEnemyAIController::OnPlayerDetected(APawn* PlayerPawn)
{
	if (!PlayerPawn) return;
	if (ASLHeroController* PC = Cast<ASLHeroController>(PlayerPawn->GetController()))
	{
		PC->ShowBossHPBar(GetPawn());
	}
}