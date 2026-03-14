#include "SL/Abilities/GA_Focus.h"
#include "SL/Enemy/SLEnemy.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "SL/Util/SLLogChannels.h"

UGA_Focus::UGA_Focus()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_Focus::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    ACharacter* Hero = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
    OwnerPC = ActorInfo->PlayerController.Get();

    if (!Hero || !OwnerPC)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    int32 ViewportSizeX, ViewportSizeY;
    OwnerPC->GetViewportSize(ViewportSizeX, ViewportSizeY);
    FVector2D ScreenCenter = FVector2D(ViewportSizeX * 0.5f, ViewportSizeY * 0.5f);
    
    FVector TraceStart, TraceDirection;
    if (UGameplayStatics::DeprojectScreenToWorld(OwnerPC, ScreenCenter, TraceStart, TraceDirection))
    {
        FVector TraceEnd = TraceStart + (TraceDirection * 5000.0f);

        float SweepRadius = 50.0f;
        FCollisionShape SphereShape = FCollisionShape::MakeSphere(SweepRadius);
        
        FHitResult HitResult;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(Hero);
        
        if (GetWorld()->SweepSingleByChannel(HitResult, TraceStart, TraceEnd, FQuat::Identity, ECC_Pawn, SphereShape, Params))
        {
            if (ASLEnemy* Enemy = Cast<ASLEnemy>(HitResult.GetActor()))
            {
                TargetActor = Enemy;

                // 락온 시작 메시지 발송
                FSLTargetLockMessage Msg;
                Msg.TargetActor = TargetActor;
                Msg.bIsLockedOn = true;

                UE_LOG(LogSL, Log, TEXT("Focus: Lock-On Target Acquired. Sending Message."));
                UGameplayMessageSubsystem::Get(this).BroadcastMessage(
                    FGameplayTag::RequestGameplayTag(FName("Message.LockOn")), Msg);
            }
        }
    }

    if (!TargetActor)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    Hero->GetCharacterMovement()->bOrientRotationToMovement = false;
    Hero->bUseControllerRotationYaw = true;

    UpdateLockOn();
}

void UGA_Focus::UpdateLockOn()
{
    if (!IsActive() || !TargetActor || !OwnerPC) 
    {
        return; 
    }

    AActor* Avatar = GetAvatarActorFromActorInfo();
    if (Avatar)
    {
        FVector TargetLocation = TargetActor->GetActorLocation();
        FRotator TargetRot = UKismetMathLibrary::FindLookAtRotation(Avatar->GetActorLocation(), TargetLocation);
        FRotator CurrentRot = OwnerPC->GetControlRotation();

        FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, GetWorld()->GetDeltaSeconds(), 12.0f);
        OwnerPC->SetControlRotation(FRotator(CurrentRot.Pitch, NewRot.Yaw, CurrentRot.Roll));
    }

    if (IsActive())
    {
        UAbilityTask_WaitDelay* WaitTask = UAbilityTask_WaitDelay::WaitDelay(this, 0.01f);
        if (WaitTask)
        {
            WaitTask->OnFinish.AddDynamic(this, &UGA_Focus::UpdateLockOn);
            WaitTask->ReadyForActivation();
        }
    }
}

void UGA_Focus::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    if (TargetActor)
    {
        FSLTargetLockMessage Msg;
        Msg.TargetActor = TargetActor;
        Msg.bIsLockedOn = false;
                
        UGameplayMessageSubsystem::Get(this).BroadcastMessage(
            FGameplayTag::RequestGameplayTag(FName("Message.LockOn")), Msg);
    }

    if (ACharacter* Hero = Cast<ACharacter>(ActorInfo->AvatarActor.Get()))
    {
        Hero->GetCharacterMovement()->bOrientRotationToMovement = true;
        Hero->bUseControllerRotationYaw = false;
    }

    TargetActor = nullptr;

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Focus::InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    Super::InputPressed(Handle, ActorInfo, ActivationInfo);

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}