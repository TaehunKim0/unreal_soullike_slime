#include "SL/AnimNotify/AnimNotifyState_AttackTrace.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "SL/Util/SLUtils.h"

void UAnimNotifyState_AttackTrace::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
}

void UAnimNotifyState_AttackTrace::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

    AActor* Owner = MeshComp->GetOwner();
    if (!Owner) return;

    // 프리뷰 액터에 ASC가 없을 때 로그 에러 방지
    UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
    if (ASC)
    {
        FGameplayEventData Payload;
        Payload.Instigator = Owner;
        Payload.EventTag = EventTag;
        Payload.EventMagnitude = bCanKnockback ? 1.0f : 0.0f;
        Payload.OptionalObject = this;
        UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, EventTag, Payload);
    }

#if WITH_EDITOR
    if (bDrawDebug && Owner->GetWorld())
    {
        // [통일] 무조건 액터 위치 + 회전된 오프셋 사용
        FTransform MeshTransform = MeshComp->GetComponentTransform();
        
        // TraceOffset을 메시의 로컬 좌표계에서 월드로 변환
        FVector CenterLocation = MeshTransform.TransformPosition(TraceOffset);
        
        // 회전 역시 메시의 회전을 따름
        FQuat MeshQuat = MeshTransform.GetRotation();
        FQuat DebugRotation = MeshQuat * TraceRotation.Quaternion();
        FCollisionShape DebugShape;

        if (TraceType == EMeleeTraceType::Sphere)
        {
            DebugShape = FCollisionShape::MakeSphere(TraceRadius);
        }
        else if (TraceType == EMeleeTraceType::Capsule)
        {
            DebugShape = FCollisionShape::MakeCapsule(TraceRadius, TraceDistance * 0.5f);
            DebugRotation = DebugRotation * FRotator(0.f, 90.f, 90.f).Quaternion();
        }
        else // Box
        {
            DebugShape = FCollisionShape::MakeBox(FVector(TraceDistance * 0.5f, TraceRadius, TraceRadius));
        }

        // 고정 위치 드로잉
        SLUtil::DrawDebugAttackShape(Owner->GetWorld(), CenterLocation, CenterLocation, DebugRotation, TraceType, DebugShape, false);
    }
#endif
}

void UAnimNotifyState_AttackTrace::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
}