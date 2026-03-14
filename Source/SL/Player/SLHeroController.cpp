// Fill out your copyright notice in the Description page of Project Settings.


#include "SL/Player/SLHeroController.h"
#include "GameFramework/Pawn.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "MVVMSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Engine/AssetManager.h"
#include "GameFramework/Character.h"
#include "SL/Abilities/SLAbilitySystemComponent.h"
#include "SL/Character/SLHero.h"
#include "SL/Data/SLInputSet.h"
#include "SL/Mvvm/AttributeViewModel.h"
#include "SL/UI/BossHpWidget.h"
#include "SL/UI/HeroHUDWidget.h"
#include "View/MVVMView.h"

ASLHeroController::ASLHeroController()
{
}

void ASLHeroController::BeginPlay()
{
	Super::BeginPlay();
}

void ASLHeroController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (!IsLocalPlayerController()) return;
	UAssetManager& AssetManager = UAssetManager::Get();
	FGameplayTag CharacterTag = FGameplayTag::RequestGameplayTag(FName("Character.Hero")); 
	FPrimaryAssetId AssetId("InputSet", CharacterTag.GetTagLeafName());

	AssetManager.LoadPrimaryAsset(AssetId);
	LoadedInputSet = Cast<USLInputSet>(AssetManager.GetPrimaryAssetObject(AssetId));

    UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
    UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);

    if (Subsystem)
    {
        if (LoadedInputSet)
        {
            for (const TObjectPtr<UInputMappingContext>& IMC : LoadedInputSet->AdditionalIMCs)
            {
                if (IMC) Subsystem->AddMappingContext(IMC, 1);
            }
        }
    }

    if (EnhancedInputComponent)
    {
        if (LoadedInputSet)
        {
        	for (const FSLInputAction& Action : LoadedInputSet->NativeInputActions)
        	{
        		if (Action.InputAction && Action.InputTag.IsValid())
        		{
        			const FGameplayTag& Tag = Action.InputTag;

        			if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Input.Native.Move"))))
        			{
        				EnhancedInputComponent->BindAction(Action.InputAction, ETriggerEvent::Triggered, this, &ASLHeroController::Move);
        			}
        			else if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Input.Native.Look"))))
        			{
        				EnhancedInputComponent->BindAction(Action.InputAction, ETriggerEvent::Triggered, this, &ASLHeroController::Look);
        			}
        		}
        	}
        	
            for (const FSLInputAction& Action : LoadedInputSet->AbilityInputActions)
            {
                if (Action.InputAction && Action.InputTag.IsValid())
                {
                    EnhancedInputComponent->BindAction(Action.InputAction, ETriggerEvent::Started, this, &ASLHeroController::InputAbilityTagPressed, Action.InputTag);
                    EnhancedInputComponent->BindAction(Action.InputAction, ETriggerEvent::Completed, this, &ASLHeroController::InputAbilityTagReleased, Action.InputTag);
                }
            }
        }
    }
}

void ASLHeroController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (!HeroHUDWidget && HeroHUDWidgetClass)
	{
		HeroHUDWidget = CreateWidget<UHeroHUDWidget>(this, HeroHUDWidgetClass);
		HeroHUDWidget->AddToViewport();
	}

	UpdateHUDViewModel(InPawn);
}

void ASLHeroController::UpdateHUDViewModel(APawn* InPawn)
{
	if (!HeroHUDWidget) return;

	if (!HeroVMInstance) 
	{
		HeroVMInstance = NewObject<UAttributeViewModel>(this);
	}

	UMVVMSubsystem* MVVMSubsystem = GEngine->GetEngineSubsystem<UMVVMSubsystem>();
	UMVVMView* MVVMView = MVVMSubsystem->GetViewFromUserWidget(HeroHUDWidget);
    
	if (MVVMView)
	{
		TScriptInterface<INotifyFieldValueChanged> VMInterface(HeroVMInstance);
		MVVMView->SetViewModel(FName("AttributeViewModel"), VMInterface);
       
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(InPawn))
		{
			if (UAbilitySystemComponent* NewASC = ASI->GetAbilitySystemComponent())
			{
				HeroVMInstance->InitializeViewModel(NewASC);
			}
		}
	}
}

void ASLHeroController::HandleBossWidgetCleanup(AActor* DestroyedActor)
{
	if (BossHpWidget)
	{
		BossHpWidget->RemoveFromParent();
		BossHpWidget = nullptr;
	}
    
	BossVMInstance = nullptr;
}

void ASLHeroController::ShowBossHPBar(APawn* BossPawn)
{
	if (!IsLocalController()) return;

	if (!BossHpWidget && BossHpWidgetClass)
	{
		BossHpWidget = CreateWidget<UBossHpWidget>(this, BossHpWidgetClass);
		BossHpWidget->AddToViewport();
	}

	if (BossHpWidget)
	{
		UMVVMSubsystem* MVVMSubsystem = GEngine->GetEngineSubsystem<UMVVMSubsystem>();
		UMVVMView* BossMVVMView = MVVMSubsystem->GetViewFromUserWidget(BossHpWidget);
        
		if (BossMVVMView)
		{
			if (!BossVMInstance) BossVMInstance = NewObject<UAttributeViewModel>(this);

			TScriptInterface<INotifyFieldValueChanged> VMInterface(BossVMInstance);
			BossMVVMView->SetViewModel(FName("AttributeViewModel"), VMInterface);
              
			if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(BossPawn))
			{
				if (UAbilitySystemComponent* BossASC = ASI->GetAbilitySystemComponent())
				{
					BossVMInstance->InitializeViewModel(BossASC);
				}
			}

			BossPawn->OnDestroyed.AddDynamic(this, &ASLHeroController::HandleBossWidgetCleanup);
		}
	}
}

void ASLHeroController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	if (APawn* MyPawn = GetPawn<APawn>())
	{
		const ASLHero* Hero = Cast<ASLHero>(MyPawn);
		if (USLAbilitySystemComponent* ASC = Hero->GetSLAbilitySystemComponent())
		{
			ASC->ProcessAbilityInput(DeltaTime, bGamePaused);
		}
	}
	
	Super::PostProcessInput(DeltaTime, bGamePaused);
}

EDataValidationResult ASLHeroController::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (!HeroHUDWidgetClass)
	{
		Context.AddError(FText::FromString(TEXT("HeroHUDWidgetClass 비어있습니다. 반드시 설정해야 합니다.")));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}

void ASLHeroController::InputAbilityTagPressed(const FInputActionValue& InputActionValue, FGameplayTag GameplayTag)
{
	if (APawn* MyPawn = GetPawn<APawn>())
	{
		const ASLHero* Hero = Cast<ASLHero>(MyPawn);
		if (USLAbilitySystemComponent* ASC = Hero->GetSLAbilitySystemComponent())
		{
			ASC->AbilityInputTagPressed(GameplayTag);
		}
	}
}

void ASLHeroController::InputAbilityTagReleased(const FInputActionValue& InputActionValue, FGameplayTag GameplayTag)
{
	if (APawn* MyPawn = GetPawn<APawn>())
	{
		const ASLHero* Hero = Cast<ASLHero>(MyPawn);
		if (USLAbilitySystemComponent* ASC = Hero->GetSLAbilitySystemComponent())
		{
			ASC->AbilityInputTagPressed(GameplayTag);
		}
	}
}

void ASLHeroController::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (APawn* ControlledPawn = GetPawn())
	{
		const FRotator Rotation = GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		ControlledPawn->AddMovementInput(ForwardDirection, MovementVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ASLHeroController::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (APawn* ControlledPawn = GetPawn())
	{
		AddYawInput(LookAxisVector.X);
		AddPitchInput(LookAxisVector.Y);
	}
}