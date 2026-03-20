// Fill out your copyright notice in the Description page of Project Settings.


#include "SL/Enemy/Abilities/GA_ParrySuccess.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/Character.h"

UGA_ParrySuccess::UGA_ParrySuccess()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = FGameplayTag::RequestGameplayTag(TEXT("Event.ParrySuccess"));
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);

	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	bRetriggerInstancedAbility = true;
}

void UGA_ParrySuccess::ActivateAbility(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                       const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (TriggerEventData && TriggerEventData->Target)
	{
		AActor* Defender = const_cast<AActor*>(TriggerEventData->Target.Get());
		ACharacter* MyChar = Cast<ACharacter>(GetAvatarActorFromActorInfo());

		if (MyChar && Defender)
		{
			// 방어자 -> 공격자 방향 벡터 계산
			FVector KnockbackDir = (MyChar->GetActorLocation() - Defender->GetActorLocation()).GetSafeNormal();
			KnockbackDir.Z = 0.0f; // 위로 뜨지 않게 보정
            
			// 캐릭터를 살짝 띄우며 뒤로 밀어냄
			MyChar->LaunchCharacter(KnockbackDir * 200.f + FVector(0, 0, 200.0f), true, true);
		}
	}

	if (ParrySuccessMontage)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		  this, NAME_None, ParrySuccessMontage, 1.0f, NAME_None, false, 1.0f
		);

		MontageTask->OnCompleted.AddDynamic(this, &UGA_ParrySuccess::OnMontageCompleted);
		MontageTask->OnBlendOut.AddDynamic(this, &UGA_ParrySuccess::OnMontageCompleted);
		MontageTask->OnInterrupted.AddDynamic(this, &UGA_ParrySuccess::OnMontageInterrupted);
		MontageTask->OnCancelled.AddDynamic(this, &UGA_ParrySuccess::OnMontageInterrupted);

		MontageTask->ReadyForActivation();
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

bool UGA_ParrySuccess::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UGA_ParrySuccess::OnMontageCompleted()
{
	K2_EndAbility();
}

void UGA_ParrySuccess::OnMontageInterrupted()
{
	K2_EndAbility();
}
