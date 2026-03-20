// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "ItemData.generated.h"

UENUM(Blueprintable)
enum class EItemType : uint8 { Consumable, Weapon, None };


/**
 * 
 */
UCLASS()
class SL_API UItemData : public UPrimaryDataAsset
{
	GENERATED_BODY()

protected:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("ItemData", ItemTag.GetTagLeafName());
	}

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	
public:
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag ItemTag;

	UPROPERTY(EditDefaultsOnly)
	EItemType ItemType;

	UPROPERTY(EditDefaultsOnly)
	FText Name;

	UPROPERTY(EditDefaultsOnly)
	int32 Count;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UStaticMesh> Mesh;

	UPROPERTY(EditDefaultsOnly)
	TArray<TSubclassOf<UGameplayEffect>> ItemEffectClassArray;
};