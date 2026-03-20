// Fill out your copyright notice in the Description page of Project Settings.


#include "SL/Mvvm/AttributeViewModel.h"

#include "Components/SlateWrapperTypes.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "SL/Attributes/HealthAttributeSet.h"
#include "SL/Attributes/StaminaAttributeSet.h"
#include "SL/Character/QuickSlotComponent.h"
#include "SL/Data/ItemData.h"

UAttributeViewModel::UAttributeViewModel()
{
	static ConstructorHelpers::FObjectFinder<UTexture2D> DefaultTexture(TEXT("/Script/Engine.Texture2D'/Game/SL/Hero/UI/T_ItemBackground.T_ItemBackground'"));
	if (DefaultTexture.Succeeded())
	{
		EquipItemImage = DefaultTexture.Object;
		WeaponImage = DefaultTexture.Object;
	}
}

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

	UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
	MsgSubsystem.RegisterListener(
		FGameplayTag::RequestGameplayTag(FName("Message.ItemOverlap")),
		this,
		&UAttributeViewModel::OnItemOverlapped
	);

	MsgSubsystem.RegisterListener(
		FGameplayTag::RequestGameplayTag(FName("Message.EquipItem")),
		this,
		&UAttributeViewModel::OnItemEquipped
	);

	MsgSubsystem.RegisterListener(
	FGameplayTag::RequestGameplayTag(FName("Message.EquipWeapon")),
	this,
		&UAttributeViewModel::OnWeaponEquipped
	);

	MsgSubsystem.RegisterListener(
	FGameplayTag::RequestGameplayTag(FName("Message.UseItem")),
	this,
		&UAttributeViewModel::OnItemUsed
	);
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

void UAttributeViewModel::OnItemOverlapped(FGameplayTag Channel, const FSLItemOverlapMessage& Payload)
{
	if (Payload.bIsOverlapped == true)
	{
		auto NewVisibility = ESlateVisibility::Visible; 
		UE_MVVM_SET_PROPERTY_VALUE(PickupVisibility, NewVisibility);

		auto NewItemName = Payload.ItemName; 
		UE_MVVM_SET_PROPERTY_VALUE(PickupItemName, NewItemName);
	}
	else
	{
		auto NewVisibility = ESlateVisibility::Collapsed; 
		UE_MVVM_SET_PROPERTY_VALUE(PickupVisibility, NewVisibility);
	}
}

void UAttributeViewModel::OnItemEquipped(FGameplayTag Channel, const FSLEquipItemMessage& Payload)
{
	if (Payload.bIsEquip == true)
	{
		auto NewImage = Payload.ItemData->Icon;
		UE_MVVM_SET_PROPERTY_VALUE(EquipItemImage, NewImage);

		auto Count = Payload.ItemData->Count;
		FString String = FString::Printf(TEXT("%d"), Count);
		auto NewText = FText::FromString(String);
		UE_MVVM_SET_PROPERTY_VALUE(EquipItemCountText, NewText);
	}
}

void UAttributeViewModel::OnWeaponEquipped(FGameplayTag Channel, const FSLEquipWeaponMessage& Payload)
{
	if (Payload.bIsEquip == true)
	{
		auto NewImage = Payload.ItemData->Icon;
		UE_MVVM_SET_PROPERTY_VALUE(WeaponImage, NewImage);
	}
}

void UAttributeViewModel::OnItemUsed(FGameplayTag Channel, const FSLUseItemMessage& Payload)
{
	FString String = FString::Printf(TEXT("%d"), Payload.RemainItemCount);
	auto NewText = FText::FromString(String);
	UE_MVVM_SET_PROPERTY_VALUE(EquipItemCountText, NewText);
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