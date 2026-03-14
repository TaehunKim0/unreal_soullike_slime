// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HeroHUDWidget.generated.h"

/**
 * 
 */
UCLASS()
class SL_API UHeroHUDWidget : public UUserWidget
{
	GENERATED_BODY()


protected:
	virtual void NativeConstruct() override;

protected:
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	TObjectPtr<class UProgressBar> ProgressBar_Hp;

	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	TObjectPtr<class UProgressBar> ProgressBar_Stamina;
};