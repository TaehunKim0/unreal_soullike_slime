// Fill out your copyright notice in the Description page of Project Settings.


#include "SL/AnimNotify/AnimNotify_AddLooseGamePlayTag.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

void UAnimNotify_AddLooseGamePlayTag::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                             const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (MeshComp && MeshComp->GetOwner())
	{
		UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MeshComp->GetOwner());
		if (ASC && GamePlayTag.IsValid())
		{
			if (bRemove)
			{
				ASC->RemoveLooseGameplayTag(GamePlayTag);
			}
			else
			{
				ASC->AddLooseGameplayTag(GamePlayTag);
			}
		}
	}
}
