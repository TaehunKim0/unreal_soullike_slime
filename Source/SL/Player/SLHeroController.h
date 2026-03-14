// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"
#include "SL/UI/HeroHUDWidget.h"
#include "SLHeroController.generated.h"

class USLInputSet;
/**
 * 
 */
UCLASS()
class SL_API ASLHeroController : public APlayerController
{
	GENERATED_BODY()

public:
	ASLHeroController();


	UFUNCTION(BlueprintCallable, Category = "AI")
	void ShowBossHPBar(APawn* BossPawn);
	
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;

	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;

	void InputAbilityTagPressed(const FInputActionValue& InputActionValue, FGameplayTag GameplayTag);
	void InputAbilityTagReleased(const FInputActionValue& InputActionValue, FGameplayTag GameplayTag);

	void UpdateHUDViewModel(APawn* InPawn);

	UFUNCTION()
	void HandleBossWidgetCleanup(AActor* DestroyedActor);
	
private:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

protected:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UHeroHUDWidget> HeroHUDWidgetClass;
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UBossHpWidget> BossHpWidgetClass;
	
	TObjectPtr<class UAttributeViewModel> HeroVMInstance;
	TObjectPtr<class UAttributeViewModel> BossVMInstance;

private:
	UPROPERTY()
	TObjectPtr<USLInputSet> LoadedInputSet;
	UPROPERTY()
	TObjectPtr<UHeroHUDWidget> HeroHUDWidget;
	UPROPERTY()
	TObjectPtr<UBossHpWidget> BossHpWidget;
};