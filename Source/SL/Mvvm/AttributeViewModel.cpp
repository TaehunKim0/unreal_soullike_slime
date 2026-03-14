// Fill out your copyright notice in the Description page of Project Settings.


#include "SL/Mvvm/AttributeViewModel.h"

#include "SL/Attributes/HealthAttributeSet.h"
#include "SL/Attributes/StaminaAttributeSet.h"

void UAttributeViewModel::InitializeViewModel(UAbilitySystemComponent* ASC)
{
	if (!ASC) return;
	
	if (auto HealthSet = ASC->GetSet<UHealthAttributeSet>())
	{
		HealthSetPtr = HealthSet;
			
		ASC->GetGameplayAttributeValueChangeDelegate(HealthSetPtr->GetHealthAttribute())
			.AddUObject(this, &UAttributeViewModel::OnHealthChanged);
       
		ASC->GetGameplayAttributeValueChangeDelegate(HealthSetPtr->GetMaxHealthAttribute())
			.AddUObject(this, &UAttributeViewModel::OnHealthChanged);

		RefreshHealth();
	}

	if (auto StaminaSet = ASC->GetSet<UStaminaAttributeSet>())
	{
		StaminaSetPtr = StaminaSet;
		ASC->GetGameplayAttributeValueChangeDelegate(StaminaSet->GetStaminaAttribute())
		   .AddUObject(this, &UAttributeViewModel::OnStaminaChanged);
        
		RefreshStamina();
	}
}

void UAttributeViewModel::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	if (HealthSetPtr.IsValid())
	{
		RefreshHealth();
	}
}

void UAttributeViewModel::OnStaminaChanged(const FOnAttributeChangeData& Data)
{
	if (StaminaSetPtr.IsValid())
	{
		RefreshStamina();
	}
}

void UAttributeViewModel::RefreshHealth()
{
	if (HealthSetPtr.IsValid())
	{
		const float CurrentHealth = HealthSetPtr->GetHealth();
		const float MaxHealth = HealthSetPtr->GetMaxHealth();
        
		const float NewPercent = (MaxHealth > 0.f) ? (CurrentHealth / MaxHealth) : 0.f;
        
		UE_MVVM_SET_PROPERTY_VALUE(HealthPercent, NewPercent);
	}
}

void UAttributeViewModel::RefreshStamina()
{
	if (StaminaSetPtr.IsValid())
	{
		const float CurrentStamina = StaminaSetPtr->GetStamina();
		const float MaxStamina = StaminaSetPtr->GetMaxStamina();

		const float NewPercent = (MaxStamina > 0.f) ? (CurrentStamina / MaxStamina) : 0.f;

		UE_MVVM_SET_PROPERTY_VALUE(StaminaPercent, NewPercent);
	}
}