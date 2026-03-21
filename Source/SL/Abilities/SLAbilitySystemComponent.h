#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "NativeGameplayTags.h"
#include "SLAbilitySystemComponent.generated.h"

class USLAbilitySet;

UCLASS()
class SL_API USLAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	USLAbilitySystemComponent();

public:
	void AddActorAbilities(AActor* InActor, const USLAbilitySet& InAbilitySet);
	void RemoveActorAbilities();

	bool ActivateAbility(FGameplayTag GamePlayTag);
	bool ActivateAbility(FGameplayTag GamePlayTag, FGameplayAbilitySpecHandle& AbilitySpecHandle);

	UFUNCTION(BlueprintCallable)
	void ApplyItemEffect(const TSubclassOf<UGameplayEffect>& EffectClass, float Level = 1.0f, AActor* EffectCauser = nullptr);
	
	UFUNCTION(BlueprintCallable)
	void ApplyItemEffects(const TArray<TSubclassOf<UGameplayEffect>>& EffectClasses, float Level = 1.0f, AActor* EffectCauser = nullptr);
	
public:
	void ProcessAbilityInput(float DeltaTime, bool bGamePaused);

	void AbilityInputTagPressed(const FGameplayTag& InputTag);
	void AbilityInputTagReleased(const FGameplayTag& InputTag);

protected:
	// 이번 프레임에 눌린 어빌리티 핸들들
	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;

	// 현재 계속 눌려있는(Hold) 어빌리티 핸들들
	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;

	// 이번 프레임에 떼어진 어빌리티 핸들들
	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;

private:
	TArray<struct FGameplayAbilitySpecHandle> AbilitySpecs;
};