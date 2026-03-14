#include "SL/Abilities/SLAbilitySystemComponent.h"
#include "SL/Abilities/SLGameplayAbility.h" // 프로젝트의 기본 GA 클래스 헤더
#include "SL/Data/SLAbilitySet.h"
#include "SL/Util/SLLogChannels.h"

USLAbilitySystemComponent::USLAbilitySystemComponent()
{
}

void USLAbilitySystemComponent::AddActorAbilities(AActor* InActor, const USLAbilitySet& InAbilitySet)
{
    check(InActor);
    
    if (!InActor->HasAuthority())
    {
        return;
    }

    for (const FSLAbilitySet_AttributeSet& AttributeToGrant : InAbilitySet.GrantAttributeSets)
    {
        if (!IsValid(AttributeToGrant.AttributeSet)) continue;

        const UAttributeSet* ExistingSet = GetAttributeSubobject(AttributeToGrant.AttributeSet);
        if (ExistingSet) 
        {
            continue; 
        }

        // 이미 동일한 타입의 AttributeSet이 있는지 확인 후 추가
        UAttributeSet* NewAS = NewObject<UAttributeSet>(InActor, AttributeToGrant.AttributeSet);
        if (NewAS)
        {
            AddAttributeSetSubobject(NewAS);
            UE_LOG(LogSL, Log, TEXT("%s 부여 성공: 어트리뷰트 %s"),*GetOwner()->GetName(), *AttributeToGrant.AttributeSet->GetName());
        }
    }

    for (const FSLAbilitySet_GameplayAbility& AbilityToGrant : InAbilitySet.GrantAbilitiesWithInputTag)
    {
        if (!IsValid(AbilityToGrant.Ability)) continue;

        FGameplayAbilitySpec Spec(AbilityToGrant.Ability, AbilityToGrant.AbilityLevel);
        
        // 아까 구현한 AbilityInputTagPressed에서 검색 가능하게 함
        if (AbilityToGrant.InputTag.IsValid())
        {
            Spec.GetDynamicSpecSourceTags().AddTag(AbilityToGrant.InputTag);
        }

        FGameplayAbilitySpecHandle AbilityHandle = GiveAbility(Spec);
        AbilitySpecs.Add(AbilityHandle);

        if (AbilityHandle.IsValid())
        {
            UE_LOG(LogSL, Log, TEXT("부여 성공: 어빌리티 %s (태그: %s)"), 
                *AbilityToGrant.Ability->GetName(), *AbilityToGrant.InputTag.ToString());
        }
    }

    for (const FSLAbilitySet_GameplayEffect& EffectToGrant : InAbilitySet.GrantGameplayEffects)
    {
        if (!IsValid(EffectToGrant.GameplayEffect)) continue;

        FGameplayEffectContextHandle ContextHandle = MakeEffectContext();
        FGameplayEffectSpecHandle SpecHandle = MakeOutgoingSpec(EffectToGrant.GameplayEffect, EffectToGrant.EffectLevel, ContextHandle);
        FActiveGameplayEffectHandle GEHandle = ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

        if (GEHandle.IsValid())
        {
            UE_LOG(LogSL, Log, TEXT("부여 성공: 이펙트 %s"), *EffectToGrant.GameplayEffect->GetName());
        }
    }
}

void USLAbilitySystemComponent::RemoveActorAbilities()
{
    CancelAllAbilities();

    for (const FGameplayAbilitySpecHandle& Handle : AbilitySpecs)
    {
        if (Handle.IsValid())
        {
            ClearAbility(Handle);
        }
    }

    AbilitySpecs.Empty();
    
    InputPressedSpecHandles.Empty();
    InputHeldSpecHandles.Empty();
    InputReleasedSpecHandles.Empty();
}

void USLAbilitySystemComponent::ProcessAbilityInput(float DeltaTime, bool bGamePaused)
{
    static TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate;
    AbilitiesToActivate.Reset();

    // 1. 눌려있는(Held) 입력 처리
    for (const FGameplayAbilitySpecHandle& SpecHandle : InputHeldSpecHandles)
    {
        if (const FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
        {
            if (AbilitySpec->Ability && !AbilitySpec->IsActive())
            {
                const USLGameplayAbility* SLAbilityCDO = CastChecked<USLGameplayAbility>(AbilitySpec->Ability);
                
                // 입력이 활성화되어 있는 동안 계속 실행되는 정책인 경우
                if (SLAbilityCDO->GetActivationPolicy() == ESLAbilityActivationPolicy::WhileInputActive)
                {
                    AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
                }
            }
        }
    }

    // 2. 이번 프레임에 막 눌린(Pressed) 입력 처리
    for (const FGameplayAbilitySpecHandle& SpecHandle : InputPressedSpecHandles)
    {
        if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
        {
            if (AbilitySpec->Ability)
            {
                AbilitySpec->InputPressed = true;
                if (AbilitySpec->IsActive())
                {
                    // 이미 실행 중이라면 입력을 전달
                    AbilitySpecInputPressed(*AbilitySpec);
                }
                else
                {
                    const USLGameplayAbility* SLAbilityCDO = CastChecked<USLGameplayAbility>(AbilitySpec->Ability);
                    
                    // 한 번의 입력으로 트리거되는 정책인 경우
                    if (SLAbilityCDO->GetActivationPolicy() == ESLAbilityActivationPolicy::InputTriggeredOnce)
                    {
                        AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
                    }
                }
            }
        }
    }

    // 3. 실제 어빌리티 활성화 시도
    for (const FGameplayAbilitySpecHandle& AbilitySpecHandle : AbilitiesToActivate)
    {
        TryActivateAbility(AbilitySpecHandle);

        UE_LOG(LogSL, Warning, TEXT("Try Activate Ability : %s"), *AbilitySpecHandle.ToString())
    }

    // 4. 이번 프레임에 떼어진(Released) 입력 처리
    for (const FGameplayAbilitySpecHandle& SpecHandle : InputReleasedSpecHandles)
    {
        if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
        {
            if (AbilitySpec->Ability)
            {
                AbilitySpec->InputPressed = false;

                if (AbilitySpec->IsActive())
                {
                    // 어빌리티에 입력이 떼어졌음을 전달
                    AbilitySpecInputReleased(*AbilitySpec);
                }
            }
        }
    }

    // 캐시된 핸들 초기화 (Held는 유지)
    InputPressedSpecHandles.Reset();
    InputReleasedSpecHandles.Reset();
}

void USLAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
    if (InputTag.IsValid())
    {
        for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
        {
            // 어빌리티의 DynamicSpecSourceTags에 해당 입력 태그가 있는지 확인
            if (AbilitySpec.Ability && (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag)))
            {
                InputPressedSpecHandles.AddUnique(AbilitySpec.Handle);
                InputHeldSpecHandles.AddUnique(AbilitySpec.Handle);
            }
        }
    }
}

void USLAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
    if (InputTag.IsValid())
    {
        for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
        {
            if (AbilitySpec.Ability && (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag)))
            {
                InputReleasedSpecHandles.AddUnique(AbilitySpec.Handle);
                InputHeldSpecHandles.Remove(AbilitySpec.Handle);
            }
        }
    }
}