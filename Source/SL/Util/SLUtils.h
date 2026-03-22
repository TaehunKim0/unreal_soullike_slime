#pragma once
#include "AbilitySystemInterface.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "SL/Abilities/SLAbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "MotionWarping/Public/MotionWarpingComponent.h"
#include "SL/AnimNotify/AnimNotifyState_AttackTrace.h"

namespace SLUtil
{
	static bool MeleeTraceMulti(UWorld* World, AActor* IgnoredActor, const FVector& Start, const FVector& End, float Radius, TArray<FHitResult>& OutHits, bool bDrawDebug = false)
	{
		if (!World) return false;

		FCollisionQueryParams Params;
		Params.AddIgnoredActor(IgnoredActor);

		// 1. Sweep 실행
		bool bHit = World->SweepMultiByChannel(
			OutHits, Start, End, FQuat::Identity, 
			ECC_Pawn, FCollisionShape::MakeSphere(Radius), Params
		);

		// 2. 디버그 드로잉
		if (bDrawDebug)
		{
			FColor DebugColor = bHit ? FColor::Red : FColor::Green;
			FVector Center = (Start + End) * 0.5f;
			float HalfHeight = (FVector::Dist(Start, End) * 0.5f) + Radius;
			FQuat CapsuleRot = FRotationMatrix::MakeFromZ(End - Start).ToQuat();

			DrawDebugCapsule(World, Center, HalfHeight, Radius, CapsuleRot, DebugColor, false, 2.0f);
		}

		return bHit;
	}

	static bool IsTargetParrying(AActor* Target)
	{
		if (IAbilitySystemInterface* ASCHolder = Cast<IAbilitySystemInterface>(Target))
		{
			return ASCHolder->GetAbilitySystemComponent()->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("State.Parrying"));
		}
		return false;
	}

	static bool CheckAndHandleParry(AActor* Attacker, AActor* Defender, const FHitResult& Hit, bool bEnemyKnockback)
	{
		if (!Attacker || !Defender) return false;

		if (IsTargetParrying(Defender))
		{
			UAbilitySystemComponent* AttackerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Attacker);
			UAbilitySystemComponent* DefenderASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Defender);
      
			FGameplayEventData Payload;
			Payload.Instigator = Attacker;
			Payload.Target = Defender;
			Payload.ContextHandle = AttackerASC ? AttackerASC->MakeEffectContext() : FGameplayEffectContextHandle();
			Payload.ContextHandle.AddHitResult(Hit);

			if (DefenderASC)
			{
				DefenderASC->HandleGameplayEvent(FGameplayTag::RequestGameplayTag(TEXT("Event.Hero.ParrySuccess")), &Payload);
			}
      
			if (AttackerASC && bEnemyKnockback)
			{
				AttackerASC->HandleGameplayEvent(FGameplayTag::RequestGameplayTag(TEXT("Event.Enemy.Neutralize")), &Payload);
			}

			return true; // 패ling 성공 처리
		}

		return false;
	}

	static UBlackboardComponent* GetBlackboardFromAbility(UGameplayAbility* Ability)
	{
		if (!Ability) return nullptr;

		APawn* AvatarPawn = Cast<APawn>(Ability->GetAvatarActorFromActorInfo());
		if (!AvatarPawn) return nullptr;

		AAIController* AIC = Cast<AAIController>(AvatarPawn->GetController());
		if (!AIC) return nullptr;

		return AIC->GetBlackboardComponent();
	}

	static AActor* GetActorFromBlackboard(UGameplayAbility* Ability, FName KeyName)
	{
		UBlackboardComponent* BB = GetBlackboardFromAbility(Ability);
		return BB ? Cast<AActor>(BB->GetValueAsObject(KeyName)) : nullptr;
	}

	static UMotionWarpingComponent* GetMotionWarpingComponent(AActor* InActor)
	{
		return InActor ? InActor->FindComponentByClass<UMotionWarpingComponent>() : nullptr;
	}

	static void UpdateWarpTarget(AActor* InActor, FName WarpTargetName, FVector TargetLocation)
	{
		if (auto* MotionWarpComp = GetMotionWarpingComponent(InActor))
		{
			MotionWarpComp->AddOrUpdateWarpTargetFromLocation(WarpTargetName, TargetLocation);
		}
	}

	static AAIController* GetAIControllerFromAbility(UGameplayAbility* Ability)
	{
		if (!Ability) return nullptr;

		AActor* AvatarActor = Ability->GetAvatarActorFromActorInfo();
		if (!AvatarActor) return nullptr;

		APawn* Pawn = Cast<APawn>(AvatarActor);
		if (!Pawn) return nullptr;

		return Cast<AAIController>(Pawn->GetController());
	}

	static FCollisionQueryParams GetIgnoreParams(AActor* IgnoreActor)
	{
		FCollisionQueryParams Params;
		if (IgnoreActor)
		{
			Params.AddIgnoredActor(IgnoreActor);
		}
		Params.bTraceComplex = false;
		Params.bReturnPhysicalMaterial = false;
    
		return Params;
	}

	static void DrawDebugAttackShape(UWorld* World, const FVector& Start, const FVector& End, const FQuat& Rotation, EMeleeTraceType TraceType, const FCollisionShape& Shape, bool bHit)
	{
		if (!World) return;

		FColor DebugColor = bHit ? FColor::Green : FColor::Red;
		float Duration = 0.05f; // NotifyTick 마다 그려지므로 짧게 유지

		switch (TraceType)
		{
		case EMeleeTraceType::Sphere:
			DrawDebugSphere(World, Start, Shape.GetSphereRadius(), 12, DebugColor, false, Duration);
			if (Start != End)
			{
				DrawDebugLine(World, Start, End, DebugColor, false, Duration);
				DrawDebugSphere(World, End, Shape.GetSphereRadius(), 12, DebugColor, false, Duration);
			}
			break;

		case EMeleeTraceType::Capsule:
			DrawDebugCapsule(World, Start, Shape.GetCapsuleHalfHeight(), Shape.GetCapsuleRadius(), Rotation, DebugColor, false, Duration);
			break;

		case EMeleeTraceType::Box:
			DrawDebugBox(World, Start, Shape.GetExtent(), Rotation, DebugColor, false, Duration);
			break;
		}
	}
	
}
