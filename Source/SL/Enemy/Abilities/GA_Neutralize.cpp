// Fill out your copyright notice in the Description page of Project Settings.


#include "SL/Enemy/Abilities/GA_Neutralize.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/Character.h"

UGA_Neutralize::UGA_Neutralize()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Enemy.Neutralize"));
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);

	bRetriggerInstancedAbility = true;
}

void UGA_Neutralize::ActivateAbility(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                       const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (NeutralizeMontage)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		  this, NAME_None, NeutralizeMontage, 1.0f, NAME_None, false, 1.0f
		);

		MontageTask->OnCompleted.AddDynamic(this, &UGA_Neutralize::OnMontageCompleted);
		MontageTask->OnBlendOut.AddDynamic(this, &UGA_Neutralize::OnMontageCompleted);
		MontageTask->OnInterrupted.AddDynamic(this, &UGA_Neutralize::OnMontageInterrupted);
		MontageTask->OnCancelled.AddDynamic(this, &UGA_Neutralize::OnMontageInterrupted);

		MontageTask->ReadyForActivation();
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

bool UGA_Neutralize::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UGA_Neutralize::OnMontageCompleted()
{
	K2_EndAbility();
}

void UGA_Neutralize::OnMontageInterrupted()
{
	K2_EndAbility();
}
