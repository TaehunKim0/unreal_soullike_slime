#include "SL/Abilities/GA_Dodge.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SL/Util/SLLogChannels.h"

UGA_Dodge::UGA_Dodge(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    ActivationPolicy = ESLAbilityActivationPolicy::InputTriggeredOnce;
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_Dodge::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, 
                                const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    if (ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get()))
    {
        bWasUsingControllerRotation = Character->bUseControllerRotationYaw;
        Character->bUseControllerRotationYaw = false;
        Character->GetCharacterMovement()->bOrientRotationToMovement = true;

        FVector InputVector = Character->GetLastMovementInputVector();

        if (!InputVector.IsNearlyZero())
        {
            FRotator TargetRotation = InputVector.Rotation();
            
            TargetRotation.Pitch = 0.f;
            TargetRotation.Roll = 0.f;

            Character->SetActorRotation(TargetRotation);
        }

        UAbilitySystemComponent* MyASC = GetAbilitySystemComponentFromActorInfo();
        FGameplayEffectContextHandle ContextHandle = MyASC->MakeEffectContext();
        FGameplayEffectSpecHandle SpecHandle = MyASC->MakeOutgoingSpec(ReduceStaminaEffectClass, GetAbilityLevel(), ContextHandle);

        if (SpecHandle.IsValid())
        {
            MyASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
        }
    }

    if (DodgeMontage)
    {
        UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
            this,
            NAME_None,
            DodgeMontage,
            1.0f,
            NAME_None,
            false,
            1.0f
        );

        MontageTask->OnCompleted.AddDynamic(this, &UGA_Dodge::OnMontageCompleted);
        MontageTask->OnBlendOut.AddDynamic(this, &UGA_Dodge::OnMontageCompleted);
        MontageTask->OnInterrupted.AddDynamic(this, &UGA_Dodge::OnMontageInterrupted);
        MontageTask->OnCancelled.AddDynamic(this, &UGA_Dodge::OnMontageInterrupted);

        MontageTask->ReadyForActivation();
    }
    else
    {
        UE_LOG(LogSL, Error, TEXT("DodgeMontage가 설정되지 않았습니다!"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
    }
}

void UGA_Dodge::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

    if (bWasUsingControllerRotation)
    {
        if (ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get()))
        {
            Character->bUseControllerRotationYaw = true;
            Character->GetCharacterMovement()->bOrientRotationToMovement = false;
        }

        bWasUsingControllerRotation = false;
    }
}

void UGA_Dodge::OnMontageCompleted()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Dodge::OnMontageInterrupted()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}