// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "SL/Interface/IDamageable.h"
#include "SLHero.generated.h"

struct FSLEquipWeaponMessage;
struct FGameplayTag;
class UHealthAttributeSet;

UCLASS()
class SL_API ASLHero : public ACharacter, public IAbilitySystemInterface, public IDamageable
{
	GENERATED_BODY()

public:
	ASLHero();
	
	virtual void HandleTakeDamage(AActor* Attacker) override;

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void OnWeaponEquipped(FGameplayTag Channel, const FSLEquipWeaponMessage& Payload);

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	TObjectPtr<class USLAbilitySystemComponent> GetSLAbilitySystemComponent() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UQuickSlotComponent* QuickSlot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UStaticMeshComponent* WeaponMeshComp;

protected:
	UPROPERTY()
	TObjectPtr<class USLAbilitySystemComponent> SLAbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<const UHealthAttributeSet> HealthSet;
	
	UPROPERTY()
	TObjectPtr<class USLAbilitySet> AbilitySet;
};