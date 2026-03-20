// Fill out your copyright notice in the Description page of Project Settings.


#include "SL/Character/SLHero.h"

#include "AbilitySystemComponent.h"
#include "QuickSlotComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/SpringArmComponent.h"
#include "SL/Abilities/SLAbilitySystemComponent.h"
#include "SL/Attributes/HealthAttributeSet.h"
#include "SL/Data/ItemData.h"
#include "SL/Data/SLAssetManager.h"
#include "SL/Mvvm/AttributeViewModel.h"
#include "SL/Player/SLPlayerState.h"
#include "SL/Util/SLLogChannels.h"


// Sets default values
ASLHero::ASLHero()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCharacterMovement()->bOrientRotationToMovement = true; 
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// 카메라 붐 생성 (캐릭터 뒤에서 카메라를 당겨옴)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 450.0f;        // 1. 거리를 300에서 450~500 정도로 확장 (더 멀리)
	CameraBoom->SocketOffset = FVector(0.f, 40.f, 60.f); // 2. 카메라를 캐릭터의 오른쪽 어깨 위로 살짝 이동 (어깨너머 시점)
	CameraBoom->TargetOffset = FVector(0.f, 0.f, 45.f);  // 3. 카메라가 바라보는 지점을 캐릭터 허리 위쪽으로 올림
    
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bInheritPitch = true;
	CameraBoom->bInheritYaw = true;
	CameraBoom->bInheritRoll = false; // 롤 회전은 방지

	// 4. 카메라 래그 (Camera Lag) 추가 - 소울라이크 특유의 부드러운 카메라 움직임
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 10.0f;      // 위치 지연 속도
	CameraBoom->bEnableCameraRotationLag = true;
	CameraBoom->CameraRotationLagSpeed = 15.0f; // 회전 지연 속도
	CameraBoom->CameraLagMaxDistance = 50.0f;   // 최대 지연 거리

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; // 카메라는 붐에 대해 상대적으로 회전하지 않음
	FollowCamera->FieldOfView = 90.0f;

	QuickSlot = CreateDefaultSubobject<UQuickSlotComponent>(TEXT("QuickSlot"));
	WeaponMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	WeaponMeshComp->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform,TEXT("weaponSocket"));
}

void ASLHero::BeginPlay()
{
	Super::BeginPlay();
}

void ASLHero::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	ASLPlayerState* PS = GetPlayerState<ASLPlayerState>();
	if (PS)
	{
		SLAbilitySystemComponent = Cast<USLAbilitySystemComponent>(PS->GetAbilitySystemComponent());
		
		SLAbilitySystemComponent->RemoveActorAbilities();
		PS->GetAbilitySystemComponent()->InitAbilityActorInfo(PS, this);

		USLAssetManager& AssetManager = USLAssetManager::Get();
		FGameplayTag CharacterTag = FGameplayTag::RequestGameplayTag(FName("Character.Hero")); 
		AbilitySet = AssetManager.GetAbilitySetByTag(CharacterTag);
		
		if (AbilitySet && SLAbilitySystemComponent)
		{
			SLAbilitySystemComponent->AddActorAbilities(this, *AbilitySet);
			HealthSet = SLAbilitySystemComponent->GetSet<UHealthAttributeSet>();
		}

		UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
		MessageSubsystem.RegisterListener(
			FGameplayTag::RequestGameplayTag(FName("Message.EquipWeapon")),
			this,
				&ASLHero::OnWeaponEquipped
			);
	}
}

void ASLHero::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (SLAbilitySystemComponent)
	{
		SLAbilitySystemComponent->RemoveActorAbilities();
	}
}

void ASLHero::OnWeaponEquipped(FGameplayTag Channel, const FSLEquipWeaponMessage& Payload)
{
	if (Payload.ItemData->Mesh)
	{
		WeaponMeshComp->SetStaticMesh(Payload.ItemData->Mesh);
		SLAbilitySystemComponent->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag("State.HasWeapon"));
	}
}

void ASLHero::HandleTakeDamage(AActor* Attacker)
{
	IDamageable::HandleTakeDamage(Attacker);

	UE_LOG(LogSL, Warning, TEXT("Hero TakeDamage from : %s"), *Attacker->GetName());

	SLAbilitySystemComponent->ActivateAbility(FGameplayTag::RequestGameplayTag(FName("Ability.Hit")));
}


void ASLHero::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ASLHero::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

UAbilitySystemComponent* ASLHero::GetAbilitySystemComponent() const
{
	return GetSLAbilitySystemComponent();
}

TObjectPtr<USLAbilitySystemComponent> ASLHero::GetSLAbilitySystemComponent() const
{
	return SLAbilitySystemComponent;
}

