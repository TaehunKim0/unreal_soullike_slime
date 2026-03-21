// Fill out your copyright notice in the Description page of Project Settings.


#include "SL/Enemy/AI/BTT_ActivateAbility.h"
#include "Abilities/GameplayAbility.h"
#include "SL/Abilities/SLAbilitySystemComponent.h"

EBTNodeResult::Type UBTT_ActivateAbility::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto SLASC = GetSLASC(OwnerComp);
	if (!SLASC) return EBTNodeResult::Failed;

	if (SLASC->ActivateAbility(AbilityTag, ActiveHandle))
	{
		CachedOwnerComp = &OwnerComp;
		SLASC->OnAbilityEnded.AddUObject(this, &UBTT_ActivateAbility::HandleAbilityEnded);
		return EBTNodeResult::InProgress;
	}
	
	return EBTNodeResult::Failed;
}

void UBTT_ActivateAbility::HandleAbilityEnded(const FAbilityEndedData& EndData)
{
	if (EndData.AbilitySpecHandle == ActiveHandle)
	{
		if (EndData.AbilityThatEnded)
		{
			auto SLASC = GetSLASC(*CachedOwnerComp);
			if (!SLASC) return;
			
			SLASC->OnAbilityEnded.RemoveAll(this);

			EBTNodeResult::Type NodeResult = EndData.bWasCancelled ? 
											 EBTNodeResult::Failed : 
											 EBTNodeResult::Succeeded;

			FinishLatentTask(*CachedOwnerComp, NodeResult);
		}
	}
}
