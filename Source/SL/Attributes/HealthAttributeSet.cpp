// Fill out your copyright notice in the Description page of Project Settings.


#include "SL/Attributes/HealthAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"
#include "SL/Interface/IDamageable.h"
#include "SL/Util/SLLogChannels.h"

UHealthAttributeSet::UHealthAttributeSet()
	: Health(100.f)
	, MaxHealth(100.f)
{
	HealthBeforeAttributeChange = 0.0f;
	MaxHealthBeforeAttributeChange = 0.0f;
}

void UHealthAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UHealthAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHealthAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
}

bool UHealthAttributeSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
	if (!Super::PreGameplayEffectExecute(Data))
	{
		return false;
	}

	const FGameplayTagContainer* TargetTags = Data.EffectSpec.CapturedTargetTags.GetAggregatedTags();
	if (TargetTags && TargetTags->HasTag(FGameplayTag::RequestGameplayTag("State.Invincible")))
	{
		UE_LOG(LogSL, Warning, TEXT("무적 상태이므로 데미지를 입지 않음"));
		return false;
	}

	if (Health.GetCurrentValue() <= 0.0f)
	{
		if (Data.EvaluatedData.Attribute == GetDamageAttribute() || 
			Data.EvaluatedData.Attribute == GetHealingAttribute())
		{
			return false;
		}
	}

	return true;
}

void UHealthAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
	}
}

void UHealthAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data); 

	const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();
	AActor* Instigator = EffectContext.GetInstigatorAbilitySystemComponent()->GetAvatarActor();
	AActor* Causer = EffectContext.GetEffectCauser();
	AActor* TargetActor = GetOwningActor();

	const float OldHealth = GetHealth();
	const float OldMaxHealth = GetMaxHealth();

	// 1. Damage 처리
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		const float LocalDamage = GetDamage();
		SetDamage(0.0f); 

		if (LocalDamage > 0.0f)
		{
			const float NewHealth = FMath::Clamp(OldHealth - LocalDamage, 0.0f, OldMaxHealth);
			SetHealth(NewHealth);
            
			UE_LOG(LogSL, Warning, TEXT("[DAMAGE] Target: %s | Damage: %.1f | HP: %.1f -> %.1f"), 
				*TargetActor->GetName(), LocalDamage, OldHealth, NewHealth);

			if (APawn* Pawn = Cast<APawn>(TargetActor))
			{
				if (IDamageable* Damageable = Cast<IDamageable>(Pawn))
				{
					Damageable->HandleTakeDamage(Instigator);
				}
			}
		}
	}

	// 2. Healing 처리
	else if (Data.EvaluatedData.Attribute == GetHealingAttribute())
	{
		const float LocalHealing = GetHealing();
		SetHealing(0.0f);

		if (LocalHealing > 0.0f)
		{
			const float NewHealth = FMath::Clamp(OldHealth + LocalHealing, 0.0f, OldMaxHealth);
			SetHealth(NewHealth);
		}
	}
}

void UHealthAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if (Attribute == GetMaxHealthAttribute())
	{
		// MaxHealth가 줄어들어서 현재 Health가 초과하는 경우
		if (GetHealth() > NewValue)
		{
			UAbilitySystemComponent* BSASC = GetOwningAbilitySystemComponent();
			check(BSASC);

			// // Health를 새로운 MaxHealth로 강제 설정
			// BSASC->ApplyModToAttribute(GetHealthAttribute(), 
			// 							 EGameplayModOp::Override,
			// 							 NewValue);
			//
			// // SetHealth 하면 무한 재귀
		}
	}
}

