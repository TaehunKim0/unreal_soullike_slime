// Fill out your copyright notice in the Description page of Project Settings.


#include "SL/Abilities/GA_Parry.h"

#include "SLAbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Kismet/GameplayStatics.h"
#include "SL/Util/SLLogChannels.h"

UGA_Parry::UGA_Parry()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ActivationPolicy = ESLAbilityActivationPolicy::InputTriggeredOnce;
}

void UGA_Parry::ActivateAbility(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (ParryMontage)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			ParryMontage,
			1.0f,
			NAME_None,
			false,
			1.0f
		);

		MontageTask->OnCompleted.AddDynamic(this, &UGA_Parry::OnMontageCompleted);
		MontageTask->OnBlendOut.AddDynamic(this, &UGA_Parry::OnMontageCompleted);
		MontageTask->OnInterrupted.AddDynamic(this, &UGA_Parry::OnMontageInterrupted);
		MontageTask->OnCancelled.AddDynamic(this, &UGA_Parry::OnMontageInterrupted);

		USLAbilitySystemComponent* MyASC = Cast<USLAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
		MyASC->ApplyItemEffect(BlockRegenStaminaEffectClass);
        
		MontageTask->ReadyForActivation();
	}
	else
	{
		UE_LOG(LogSL, Error, TEXT("ParryMontage 가 설정되지 않았습니다!"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

void UGA_Parry::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Parry::OnMontageCompleted()
{
	K2_EndAbility();
}

void UGA_Parry::OnMontageInterrupted()
{
	K2_EndAbility();
}
