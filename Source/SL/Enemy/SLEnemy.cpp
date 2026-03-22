// Fill out your copyright notice in the Description page of Project Settings.


#include "SL/Enemy/SLEnemy.h"

#include "MotionWarpingComponent.h"
#include "SLEnemyAIController.h"
#include "Blueprint/UserWidget.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "SL/Abilities/GA_Focus.h"
#include "SL/Abilities/SLAbilitySystemComponent.h"
#include "SL/Attributes/HealthAttributeSet.h"
#include "SL/Data/SLAssetManager.h"
#include "MotionWarpingComponent.h"
#include "Components/SphereComponent.h"

ASLEnemy::ASLEnemy()
{
	PrimaryActorTick.bCanEverTick = true;
	SLAbilitySystemComponent = CreateDefaultSubobject<USLAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	LockOnWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComponent"));
	LockOnWidgetComp->SetupAttachment(GetCapsuleComponent());

	MotionWarpingComp = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));
}

UAbilitySystemComponent* ASLEnemy::GetAbilitySystemComponent() const
{
	return SLAbilitySystemComponent;
}

void ASLEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (SLAbilitySystemComponent)
	{
		SLAbilitySystemComponent->InitAbilityActorInfo(this, this);

		USLAssetManager& AssetManager = USLAssetManager::Get();
		FGameplayTag CharacterTag = FGameplayTag::RequestGameplayTag(FName("Character.Slime")); 
		LoadedAbilitySet = AssetManager.GetAbilitySetByTag(CharacterTag);
		
		if (LoadedAbilitySet)
		{
			SLAbilitySystemComponent->AddActorAbilities(this, *LoadedAbilitySet);
			HealthSet = SLAbilitySystemComponent->GetSet<UHealthAttributeSet>();

			SLAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(HealthSet->GetHealthAttribute())
				.AddUObject(this, &ASLEnemy::HandleHealthChanged);
		}
	}

	UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
	MsgSubsystem.RegisterListener(
		FGameplayTag::RequestGameplayTag(FName("Message.LockOn")),
		this,
		&ASLEnemy::OnTargetLockMessageReceived 
	);

	if (LockOnWidgetClass)
	{
		LockOnWidgetComp->SetVisibility(true);
		LockOnWidgetComp->SetWidgetClass(LockOnWidgetClass);
		LockOnWidgetComp->InitWidget();
		LockOnWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
		LockOnWidgetComp->SetDrawSize({16,16});
		LockOnWidgetComp->SetPivot({0.5f,0.75f});
		LockOnWidgetComp->GetWidget()->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void ASLEnemy::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}

void ASLEnemy::HandleTakeDamage(AActor* Attacker)
{
	IDamageable::HandleTakeDamage(Attacker);

	ASLEnemyAIController* AIC = Cast<ASLEnemyAIController>(GetController());
	if (!AIC) return;

	AIC->OnUnderAttack(Attacker);
}

void ASLEnemy::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
	if (Data.Attribute == HealthSet->GetHealthAttribute())
	{
		if (Data.NewValue <= 0)
			Die();
	}
}

void ASLEnemy::Die()
{
	if (SLAbilitySystemComponent)
	{
		SLAbilitySystemComponent->CancelAllAbilities();
	}

	FSLTargetLockMessage Msg;
	Msg.TargetActor = nullptr;
	Msg.bIsLockedOn = false;
	UGameplayMessageSubsystem::Get(this).BroadcastMessage(FGameplayTag::RequestGameplayTag(FName("Message.LockOn")), Msg);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// if (DeathMontage)
	// {
	// 	PlayAnimMontage(DeathMontage);
	// }
	//    
	// if (AAIController* AIC = Cast<AAIController>(GetController()))
	// {
	// 	AIC->StopMovement();
	// 	AIC->UnPossess(); 
	// }

	Destroy();
	//SetLifeSpan(0.0f);
}

void ASLEnemy::OnTargetLockMessageReceived(FGameplayTag Channel, const FSLTargetLockMessage& Payload)
{
	UUserWidget* WidgetPtr = LockOnWidgetComp->GetWidget();
	if (!WidgetPtr) return;
	
	if (Payload.TargetActor != this)
	{
		if (Payload.bIsLockedOn == true)
		{
			WidgetPtr->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	else
	{
		ESlateVisibility NewVisibility = Payload.bIsLockedOn ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
		WidgetPtr->SetVisibility(NewVisibility);
	}
}