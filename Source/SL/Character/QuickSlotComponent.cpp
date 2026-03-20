// Fill out your copyright notice in the Description page of Project Settings.


#include "SL/Character/QuickSlotComponent.h"

#include "AbilitySystemInterface.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "SL/Abilities/SLAbilitySystemComponent.h"
#include "SL/Data/ItemData.h"
#include "SL/Item/SLItem.h"
#include "SL/Util/SLLogChannels.h"

UQuickSlotComponent::UQuickSlotComponent()
	: CurrentItemRemainCount(0)
{
	PrimaryComponentTick.bCanEverTick = true;
}

bool UQuickSlotComponent::CanUseItem() const
{
	if (!CurrentItemData || CurrentItemRemainCount == 0)
		return false;

	return true;
}

void UQuickSlotComponent::UseCurrentItem()
{
	if (!CurrentItemData)
	{
		UE_LOG(LogSL, Error, TEXT("현재 선택된 아이템이 없습니다."));
		return;
	}

	if (CurrentItemRemainCount <= 0)
	{
		UE_LOG(LogSL, Error, TEXT("현재 선택된 아이템을 다 사용했습니다."));
		CurrentItemData = nullptr;
		return;
	}
	
	if (auto ASI = Cast<IAbilitySystemInterface>(GetOwner()))
	{
		if (USLAbilitySystemComponent* MyASC = Cast<USLAbilitySystemComponent>(ASI->GetAbilitySystemComponent()))
		{
			if (CurrentItemData->ItemEffectClassArray.Num() > 0)
			{
				MyASC->ApplyItemEffects(CurrentItemData->ItemEffectClassArray, 1.0f, GetOwner());
				UE_LOG(LogSL, Warning, TEXT("아이템 사용 : %s"), *CurrentItemData->Name.ToString());

				FSLUseItemMessage Msg;
				Msg.ItemTag = CurrentItemData->ItemTag;
				Msg.UseActor = GetOwner();
				Msg.RemainItemCount = --CurrentItemRemainCount;
				UGameplayMessageSubsystem::Get(GetWorld()).BroadcastMessage(FGameplayTag::RequestGameplayTag("Message.UseItem"), Msg);
			}
		}
	}
	else
	{
		UE_LOG(LogSL, Error, TEXT("캐릭터에게 ASI 가 없습니다."));
	}
}

void UQuickSlotComponent::AddItem(ASLItem* Item)
{
	if (!Item) return;

	UItemData* NewData = Item->GetItemData();
	if (!NewData) return;
	
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());

	switch (NewData->ItemType)
	{
	case EItemType::Consumable:
		{
			CurrentItemData = NewData;
			FSLEquipItemMessage ChangeMessage;
			ChangeMessage.ItemData = CurrentItemData;
			ChangeMessage.EquipActor = GetOwner();
			ChangeMessage.bIsEquip = true;
			MessageSubsystem.BroadcastMessage(FGameplayTag::RequestGameplayTag("Message.EquipItem"), ChangeMessage);
	
			FSLPickUpItemMessage Message;
			Message.ItemData = CurrentItemData;
			Message.PickupActor = GetOwner();
			MessageSubsystem.BroadcastMessage(FGameplayTag::RequestGameplayTag("Message.PickupItem"), Message);
			CurrentItemRemainCount = CurrentItemData->Count;
			UE_LOG(LogSL, Warning, TEXT("아이템 획득 : %s"), *CurrentItemData->Name.ToString());
		
			break;
		}

	case EItemType::Weapon:
		{
			CurrentWeaponData = NewData;
			FSLEquipWeaponMessage WeaponMessage;
			WeaponMessage.ItemData = CurrentWeaponData;
			WeaponMessage.bIsEquip = true;
			WeaponMessage.EquipActor = GetOwner();
			MessageSubsystem.BroadcastMessage(FGameplayTag::RequestGameplayTag("Message.EquipWeapon"), WeaponMessage);
			UE_LOG(LogSL, Warning, TEXT("장비 획득 : %s"), *CurrentWeaponData->Name.ToString());
			break;
		}
		
	default: ;
	}
	
	Item->Destroy();
}

void UQuickSlotComponent::BeginPlay()
{
	Super::BeginPlay();

	UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
	MsgSubsystem.RegisterListener(
		FGameplayTag::RequestGameplayTag(FName("Message.ItemOverlap")),
		this,
		&UQuickSlotComponent::OnItemOverlapReceived
	);
}

void UQuickSlotComponent::OnItemOverlapReceived(FGameplayTag Channel, const FSLItemOverlapMessage& Payload)
{
	if (Payload.bIsOverlapped == true && Payload.ItemActor->IsValidLowLevel() && Payload.OverlappedActor == GetOwner())
	{
		CurrentOverlapItem = Payload.ItemActor;
	}

	else if (Payload.bIsOverlapped == false && Payload.OverlappedActor == GetOwner())
	{
		CurrentOverlapItem = nullptr;
	}
}