// Fill out your copyright notice in the Description page of Project Settings.


#include "SL/Abilities/GA_ParrySuccess.h"

#include "Kismet/GameplayStatics.h"
#include "SL/Util/SLLogChannels.h"

UGA_ParrySuccess::UGA_ParrySuccess()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Hero.ParrySuccess"));
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);

	bRetriggerInstancedAbility = true;
}

void UGA_ParrySuccess::ActivateAbility(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                       const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (TriggerEventData && TriggerEventData->EventTag == FGameplayTag::RequestGameplayTag(TEXT("Event.Hero.ParrySuccess")))
	{
		UGameplayStatics::PlaySound2D(GetWorld(), ParrySuccessSound);
		K2_EndAbility();
		return;
	}
}

void UGA_ParrySuccess::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
