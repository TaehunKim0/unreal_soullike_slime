// Fill out your copyright notice in the Description page of Project Settings.


#include "SL/Abilities/GA_Knockback.h"

#include "SLAbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SL/Util/SLLogChannels.h"

UGA_Knockback::UGA_Knockback()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Hero.Knockback"));
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);

	bRetriggerInstancedAbility = true;
}

void UGA_Knockback::ActivateAbility(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (Character && KnockbackMontage)
	{
		// TODO : 슬라임의 공격 반대 방향으로 날아가게
		FVector KnockbackDirection = -Character->GetActorForwardVector();
		FVector LaunchVelocity = (KnockbackDirection * KnockbackStrength) + FVector(0.f, 0.f, UpwardStrength);

		Character->LaunchCharacter(LaunchVelocity, true, true);

		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		  this, NAME_None, KnockbackMontage, 1.0f, NAME_None, false, 1.0f
		);

		MontageTask->OnCompleted.AddDynamic(this, &UGA_Knockback::OnMontageCompleted);
		MontageTask->OnBlendOut.AddDynamic(this, &UGA_Knockback::OnMontageCompleted);
		MontageTask->OnInterrupted.AddDynamic(this, &UGA_Knockback::OnMontageInterrupted);
		MontageTask->OnCancelled.AddDynamic(this, &UGA_Knockback::OnMontageInterrupted);

		MontageTask->ReadyForActivation();
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

void UGA_Knockback::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Knockback::OnMontageCompleted()
{
	K2_EndAbility();
}

void UGA_Knockback::OnMontageInterrupted()
{
	K2_EndAbility();
}
