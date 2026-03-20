// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "Components/SlateWrapperTypes.h"
#include "SL/Character/QuickSlotComponent.h"
#include "SL/Item/SLItem.h"
#include "AttributeViewModel.generated.h"

struct FSLUseItemMessage;
class UTexture2D;
struct FSLEquipItemMessage;
/**
 * 
 */
UCLASS()
class SL_API UAttributeViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UAttributeViewModel();
	
	void InitializeViewModel(class UAbilitySystemComponent* ASC);

protected:
	// Attribute
	void OnHealthChanged(const struct FOnAttributeChangeData& Data);
	void OnStaminaChanged(const struct FOnAttributeChangeData& Data);
	void RefreshHealth();
	void RefreshStamina();
	
	//Item
	void OnItemOverlapped(FGameplayTag Channel, const FSLItemOverlapMessage& Payload);
	void OnItemEquipped(FGameplayTag Channel, const FSLEquipItemMessage& Payload);
	void OnWeaponEquipped(FGameplayTag Channel, const FSLEquipWeaponMessage& Payload);
	void OnItemUsed(FGameplayTag Channel, const FSLUseItemMessage& Payload);

protected:
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "UI")
	float HealthPercent;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "UI")
	float StaminaPercent;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "UI")
	ESlateVisibility PickupVisibility = ESlateVisibility::Collapsed;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "UI")
	FText PickupItemName;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "UI")
	TObjectPtr<UTexture2D> EquipItemImage;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "UI")
	FText EquipItemCountText;
	
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "UI")
	TObjectPtr<UTexture2D> WeaponImage;

private:
	TWeakObjectPtr<const class UHealthAttributeSet> HealthSetPtr;
	TWeakObjectPtr<const class UStaminaAttributeSet> StaminaSetPtr;
};