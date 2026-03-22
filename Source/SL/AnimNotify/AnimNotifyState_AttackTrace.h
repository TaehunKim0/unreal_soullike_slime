// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_AttackTrace.generated.h"

UENUM(BlueprintType)
enum class EMeleeTraceType : uint8
{
	Sphere      UMETA(DisplayName = "Sphere"),
	Capsule     UMETA(DisplayName = "Capsule"),
	Box         UMETA(DisplayName = "Box")
};
/**
 * 
 */
UCLASS()
class SL_API UAnimNotifyState_AttackTrace : public UAnimNotifyState
{
	GENERATED_BODY()

protected:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
	                 const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime,
	                const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	               const FAnimNotifyEventReference& EventReference) override;

public:
	UPROPERTY(EditAnywhere, Category = "Attack")
	FGameplayTag EventTag;

	UPROPERTY(EditAnywhere, Category = "Attack")
	bool bCanKnockback = true;

	UPROPERTY(EditAnywhere, Category = "Attack")
	bool bDrawDebug = true;

	UPROPERTY(EditAnywhere, Category = "Attack")
	bool bResetHitActorsOnBegin = true;

	UPROPERTY(EditAnywhere, Category = "Attack")
	EMeleeTraceType TraceType = EMeleeTraceType::Sphere;

	UPROPERTY(EditAnywhere, Category = "Range")
	float TraceRadius = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Range")
	float TraceDistance = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Range")
	FVector TraceOffset = FVector(0.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, Category = "Attack")
	FRotator TraceRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, Category = "Range")
	bool bUseSocket = false;

	UPROPERTY(EditAnywhere, Category = "Range", meta = (EditCondition = "bUseSocket"))
	FName SocketName = TEXT("WeaponTip");
};
