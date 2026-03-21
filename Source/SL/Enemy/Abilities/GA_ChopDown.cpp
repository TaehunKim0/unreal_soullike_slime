// Fill out your copyright notice in the Description page of Project Settings.


#include "SL/Enemy/Abilities/GA_ChopDown.h"
#include "DrawDebugHelpers.h" 
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Net/RepLayout.h"
#include "SL/Abilities/SLAbilitySystemComponent.h"
#include "SL/Interface/IDamageable.h"
#include "SL/Util/SLLogChannels.h"
#include "SL/Util/SLUtils.h"

void UGA_ChopDown::ActivateAbility(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                   const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (AttackMontage)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		   this, NAME_None, AttackMontage, 1.0f, NAME_None, false, 1.0f
		);
		MontageTask->OnCompleted.AddDynamic(this, &UGA_ChopDown::OnMontageCompleted);
		MontageTask->OnBlendOut.AddDynamic(this, &UGA_ChopDown::OnMontageCompleted);
		MontageTask->OnInterrupted.AddDynamic(this, &UGA_ChopDown::OnMontageInterrupted);
		MontageTask->OnCancelled.AddDynamic(this, &UGA_ChopDown::OnMontageInterrupted);
		MontageTask->ReadyForActivation();

		UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this, 
			FGameplayTag::RequestGameplayTag(TEXT("Event.Enemy.Montage.ChopDown")),
			nullptr, 
			false,
			false
		);

		WaitEventTask->EventReceived.AddDynamic(this, &UGA_ChopDown::OnHitEventReceived);
		WaitEventTask->ReadyForActivation();
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

void UGA_ChopDown::OnHitEventReceived(FGameplayEventData Payload)
{
	PerformMeleeTrace();
}


void UGA_ChopDown::PerformMeleeTrace()
{
	AActor* OwningActor = GetOwningActorFromActorInfo();
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	
    if (!OwningActor) return;

    FVector Forward = OwningActor->GetActorForwardVector();
    FVector Start = OwningActor->GetActorLocation() + (Forward * 50.0f);
    FVector End = Start + (Forward * 300.0f);
    float Radius = 80.0f;

    TArray<FHitResult> HitResults;
	
    bool bHit = SLUtil::MeleeTraceMulti(GetWorld(), OwningActor, Start, End, Radius, HitResults, true);

    if (bHit)
    {
        TSet<AActor*> AlreadyHitActors;

        for (const FHitResult& Hit : HitResults)
        {
            AActor* HitActor = Hit.GetActor();

			if (SLUtil::CheckAndHandleParry(OwningActor,HitActor, Hit, true))
			{
				K2_CancelAbility();
				return;
			}
            
            if (HitActor && HitActor->Implements<UDamageable>() && !AlreadyHitActors.Contains(HitActor))
            {
                AlreadyHitActors.Add(HitActor);

                if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor))
                {
                    FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
                    ContextHandle.AddHitResult(Hit);
                    
                    FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), ContextHandle);
                    if (SpecHandle.IsValid())
                    {
                        SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
                    }
                }
                DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 10.f, 8, FColor::Yellow, false, 2.0f);
            }
        }
    }
}

bool UGA_ChopDown::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UGA_ChopDown::OnMontageCompleted()
{
	K2_EndAbility();
}

void UGA_ChopDown::OnMontageInterrupted()
{
	K2_EndAbility();
}

