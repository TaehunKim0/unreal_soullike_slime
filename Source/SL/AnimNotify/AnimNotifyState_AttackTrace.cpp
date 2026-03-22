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

    UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
    if (ASC)
    {
        FGameplayEventData Payload;
        Payload.Instigator = Owner;
        Payload.EventTag = EventTag;
        Payload.EventMagnitude = bCanNeutralize ? 1.0f : 0.0f;
        Payload.OptionalObject = this;
        UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, EventTag, Payload);
    }

#if WITH_EDITOR
    if (bDrawDebug && Owner->GetWorld())
    {
        FTransform MeshTransform = MeshComp->GetComponentTransform();
        FVector CenterLocation = MeshTransform.TransformPosition(TraceOffset);
        
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
        else 
        {
            DebugShape = FCollisionShape::MakeBox(FVector(TraceDistance * 0.5f, TraceRadius, TraceRadius));
        }

        SLUtil::DrawDebugAttackShape(Owner->GetWorld(), CenterLocation, CenterLocation, DebugRotation, TraceType, DebugShape, false);
    }
#endif
}

void UAnimNotifyState_AttackTrace::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
}