// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_AddLooseGamePlayTag.generated.h"

/**
 * 
 */
UCLASS()
class SL_API UAnimNotify_AddLooseGamePlayTag : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	UPROPERTY(EditAnywhere, Category = "GAS")
	FGameplayTag GamePlayTag;

	UPROPERTY(EditAnywhere, Category = "GAS")
	bool bRemove;
};