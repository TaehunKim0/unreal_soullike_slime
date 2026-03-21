// Fill out your copyright notice in the Description page of Project Settings.


#include "SL/Enemy/AI/SLBTTask_BlueprintBase.h"

#include "AbilitySystemInterface.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "SL/Abilities/SLAbilitySystemComponent.h"


AActor* USLBTTask_BlueprintBase::GetSelfActor(UBehaviorTreeComponent& OwnerComp)
{
	if (auto MyBlackboard = OwnerComp.GetBlackboardComponent())
	{
		UObject* BlackBoardObject = MyBlackboard->GetValueAsObject(FName("SelfActor"));
		return Cast<AActor>(BlackBoardObject);
	}

	return nullptr;
}

USLAbilitySystemComponent* USLBTTask_BlueprintBase::GetSLASC(UBehaviorTreeComponent& OwnerComp)
{
	auto SelfActor = GetSelfActor(OwnerComp);
	if (!SelfActor) return nullptr;

	auto ASC = Cast<IAbilitySystemInterface>(SelfActor)->GetAbilitySystemComponent();
	if (!ASC) return nullptr;

	auto SLASC = Cast<USLAbilitySystemComponent>(ASC);
	if (!SLASC) return nullptr;

	return SLASC;
}
