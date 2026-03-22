#include "SL/Enemy/Abilities/GA_Retreat.h"

#include "AITypes.h"
#include "NavigationSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "SL/Util/SLLogChannels.h"
#include "SL/Util/SLUtils.h"


void UGA_Retreat::ActivateAbility(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                  const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AActor* OwningActor = GetOwningActorFromActorInfo();
	AActor* TargetActor = SLUtil::GetActorFromBlackboard(this, TEXT("TargetActor"));

	if (!OwningActor || !TargetActor)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (AAIController* AIC = SLUtil::GetAIControllerFromAbility(this))
	{
		// 1. 플레이어를 계속 바라보게 설정
		AIC->SetFocus(TargetActor);

		// 2. 이동 속도 낮추기 (뒷걸음질 느낌)
		if (ACharacter* Char = Cast<ACharacter>(OwningActor))
		{
			Char->GetCharacterMovement()->MaxWalkSpeed *= RetreatSpeedRate;
		}

		// 3. 도망갈 목적지 계산 (플레이어 반대 방향)
		FVector DirFromTarget = (OwningActor->GetActorLocation() - TargetActor->GetActorLocation()).GetSafeNormal2D();
		FVector RetreatLocation = TargetActor->GetActorLocation() + (DirFromTarget * TargetDistance);

		// 4. 네비메시 유효성 체크 (벽 뒤로 넘어가려 하는 것 방지)
		UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
		FNavLocation ProjectedLocation;
		if (NavSys && NavSys->ProjectPointToNavigation(RetreatLocation, ProjectedLocation))
		{
			RetreatLocation = ProjectedLocation.Location;
		}

		// 5. 이동 실행
		FAIMoveRequest MoveReq;
		MoveReq.SetGoalLocation(RetreatLocation);
		MoveReq.SetAcceptanceRadius(20.f);

		FPathFollowingRequestResult MoveResult = AIC->MoveTo(MoveReq);

		UE_LOG(LogSL, Warning, TEXT("Move Request ID: %d, Result Code: %s"), 
			MoveResult.MoveId.GetID(), 
			*UEnum::GetValueAsString(MoveResult.Code));

		if (MoveResult.Code == EPathFollowingRequestResult::AlreadyAtGoal)
		{
			UE_LOG(LogSL, Error, TEXT("TargetLocation이 너무 가까워서 이동을 건너뜁니다!"));
		}
        
		if (UPathFollowingComponent* PFollowComp = AIC->GetPathFollowingComponent())
		{
			PFollowComp->OnRequestFinished.AddUObject(this, &UGA_Retreat::OnMoveFinished);
		}

		DrawDebugSphere(GetWorld(), RetreatLocation, 20.f, 12, FColor::Red, false, 2.0f);
	}
}

void UGA_Retreat::OnMoveFinished(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	if (AAIController* AIC = SLUtil::GetAIControllerFromAbility(this))
	{
		AIC->SetFocus(nullptr);
		if (ACharacter* Char = Cast<ACharacter>(AIC->GetPawn()))
		{
			Char->GetCharacterMovement()->MaxWalkSpeed /= RetreatSpeedRate;
		}
	}
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}