// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "QuickSlotComponent.generated.h"

class UItemData;

USTRUCT(BlueprintType)
struct FSLPickUpItemMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> PickupActor = nullptr; 

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UItemData> ItemData = nullptr;
};

USTRUCT(BlueprintType)
struct FSLEquipItemMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> EquipActor = nullptr; 

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UItemData> ItemData = nullptr;

	UPROPERTY(BlueprintReadOnly)
	bool bIsEquip = false;
};

USTRUCT(BlueprintType)
struct FSLEquipWeaponMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> EquipActor = nullptr; 

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UItemData> ItemData = nullptr;

	UPROPERTY(BlueprintReadOnly)
	bool bIsEquip = false;
};

USTRUCT(BlueprintType)
struct FSLUseItemMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> UseActor = nullptr; 

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag ItemTag;

	UPROPERTY(BlueprintReadOnly)
	int32 RemainItemCount;
};

class ASLItem;
struct FSLItemOverlapMessage;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SL_API UQuickSlotComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UQuickSlotComponent();

	void AddItem(ASLItem* Item);
	bool CanUseItem() const;
	void UseCurrentItem();
	
	AActor* GetCurrentOverlapItem() const { return CurrentOverlapItem; }

protected:
	virtual void BeginPlay() override;
	void OnItemOverlapReceived(FGameplayTag Channel, const FSLItemOverlapMessage& Payload);
	
private:
	UPROPERTY()
	TObjectPtr<AActor> CurrentOverlapItem;

	UPROPERTY()
	TObjectPtr<UItemData> CurrentItemData;

	UPROPERTY()
	TObjectPtr<UItemData> CurrentWeaponData;

	int32 CurrentItemRemainCount;
};
