#pragma once
#include "AbilitySystemInterface.h"

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

	static bool CheckAndHandleParry(AActor* Attacker, AActor* Defender, const FHitResult& Hit)
	{
		if (!Attacker || !Defender) return false;

		if (IsTargetParrying(Defender))
		{
			UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Attacker);
			if (SourceASC)
			{
				FGameplayEventData Payload;
				Payload.Instigator = Attacker;
				Payload.Target = Defender;
				Payload.ContextHandle = SourceASC->MakeEffectContext();
				Payload.ContextHandle.AddHitResult(Hit);

				SourceASC->HandleGameplayEvent(FGameplayTag::RequestGameplayTag(TEXT("Event.ParrySuccess")), &Payload);
			}
			return true; // 패링당함
		}

		return false; // 패링 아님 (일반 히트)
	}
}
