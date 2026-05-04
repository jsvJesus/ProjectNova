#include "Equipment/PNEquipmentComponent.h"

#include "Characters/PNBaseCharacter.h"
#include "Engine/Engine.h"
#include "Items/PNInventoryComponent.h"
#include "Items/PNItemDataAsset.h"
#include "Items/PNItemInstance.h"
#include "Items/PNItemTypes.h"
#include "Net/UnrealNetwork.h"

namespace PNEquipmentConstants
{
	static constexpr int32 HelmetInternalSlotCount = 4;
	static constexpr int32 ArmorInternalSlotCount = 4;
}

UPNEquipmentComponent::UPNEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}

void UPNEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();

	if (HasEquipmentAuthority())
	{
		InitializeEquipment();
	}
}

void UPNEquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPNEquipmentComponent, EquipmentSlots);
	DOREPLIFETIME(UPNEquipmentComponent, HelmetInternalSlots);
	DOREPLIFETIME(UPNEquipmentComponent, ArmorInternalSlots);
}

void UPNEquipmentComponent::InitializeEquipment()
{
	if (!HasEquipmentAuthority())
	{
		return;
	}

	BuildDefaultEquipmentSlots();
	BuildInternalSlots(EPNEquipmentInternalContainer::Helmet, PNEquipmentConstants::HelmetInternalSlotCount);
	BuildInternalSlots(EPNEquipmentInternalContainer::Armor, PNEquipmentConstants::ArmorInternalSlotCount);

	BroadcastEquipmentChanged();
}

FPNEquipmentOperationResponse UPNEquipmentComponent::EquipFromInventoryToSlot(FPNInventoryGridPosition InventoryPosition, EPNEquipmentSlot TargetSlot)
{
	FPNEquipmentOperationResponse Response;
	Response.Slot = TargetSlot;
	Response.Result = EPNEquipmentOperationResult::UnknownError;

	if (!HasEquipmentAuthority())
	{
		Response.Result = EPNEquipmentOperationResult::InvalidEquipment;
		return Response;
	}

	if (!IsEquipmentSlotValid(TargetSlot))
	{
		Response.Result = EPNEquipmentOperationResult::InvalidSlot;
		return Response;
	}

	if (IsEquipmentSlotOccupied(TargetSlot))
	{
		Response.Result = EPNEquipmentOperationResult::SlotOccupied;
		return Response;
	}

	UPNInventoryComponent* InventoryComponent = GetOwnerInventoryComponent();
	if (!InventoryComponent)
	{
		Response.Result = EPNEquipmentOperationResult::InvalidInventory;
		return Response;
	}

	UPNItemInstance* SourceItem = InventoryComponent->GetItemAtPosition(InventoryPosition);
	if (!SourceItem || !SourceItem->IsValidItem())
	{
		Response.Result = EPNEquipmentOperationResult::InvalidItem;
		return Response;
	}

	if (!CanEquipItemToSlot(SourceItem, TargetSlot))
	{
		Response.Result = EPNEquipmentOperationResult::ItemTypeNotAllowed;
		return Response;
	}

	FPNInventoryRemoveItemResult RemoveResult = InventoryComponent->RemoveItemAtPosition(InventoryPosition, 1);
	if (!RemoveResult.bSuccess || !RemoveResult.RemovedItemInstance || !RemoveResult.RemovedItemInstance->IsValidItem())
	{
		Response.Result = ConvertInventoryRemoveFail(RemoveResult.Result);
		return Response;
	}

	FPNRepItemInstanceData EquipData = RemoveResult.RemovedItemInstance->ToRepData();
	EquipData.Quantity = 1;

	SetEquipmentSlotItem(TargetSlot, EquipData);

	Response.bSuccess = true;
	Response.Result = EPNEquipmentOperationResult::Success;
	Response.ItemData = EquipData.ItemData;

	BroadcastEquipmentChanged();
	return Response;
}

FPNEquipmentOperationResponse UPNEquipmentComponent::UnequipSlotToInventory(EPNEquipmentSlot Slot)
{
	FPNEquipmentOperationResponse Response;
	Response.Slot = Slot;
	Response.Result = EPNEquipmentOperationResult::UnknownError;

	if (!HasEquipmentAuthority())
	{
		Response.Result = EPNEquipmentOperationResult::InvalidEquipment;
		return Response;
	}

	if (!IsEquipmentSlotValid(Slot))
	{
		Response.Result = EPNEquipmentOperationResult::InvalidSlot;
		return Response;
	}

	if (HasAnyInternalItemsForTopSlot(Slot))
	{
		Response.Result = EPNEquipmentOperationResult::InternalItemsNotEmpty;
		return Response;
	}

	const int32 SlotIndex = FindEquipmentSlotIndex(Slot);
	if (!EquipmentSlots.IsValidIndex(SlotIndex) || EquipmentSlots[SlotIndex].IsEmpty())
	{
		Response.Result = EPNEquipmentOperationResult::SlotEmpty;
		return Response;
	}

	UPNInventoryComponent* InventoryComponent = GetOwnerInventoryComponent();
	if (!InventoryComponent)
	{
		Response.Result = EPNEquipmentOperationResult::InvalidInventory;
		return Response;
	}

	UPNItemInstance* ItemInstance = NewObject<UPNItemInstance>(this);
	if (!ItemInstance)
	{
		Response.Result = EPNEquipmentOperationResult::InvalidItem;
		return Response;
	}

	ItemInstance->InitializeFromRepData(EquipmentSlots[SlotIndex].InstanceData);
	if (!ItemInstance->IsValidItem())
	{
		Response.Result = EPNEquipmentOperationResult::InvalidItem;
		return Response;
	}

	FPNInventoryAddItemResult AddResult = InventoryComponent->AddItem(ItemInstance, true, true);
	if (!AddResult.bSuccess || AddResult.RemainingQuantity > 0)
	{
		Response.Result = ConvertInventoryAddFail(AddResult.Result);
		return Response;
	}

	Response.ItemData = ItemInstance->GetItemData();

	ClearEquipmentSlot(Slot);

	Response.bSuccess = true;
	Response.Result = EPNEquipmentOperationResult::Success;

	BroadcastEquipmentChanged();
	return Response;
}

FPNEquipmentOperationResponse UPNEquipmentComponent::InsertFromInventoryToInternalSlot(FPNInventoryGridPosition InventoryPosition, EPNEquipmentInternalContainer Container, int32 InternalSlotIndex)
{
	FPNEquipmentOperationResponse Response;
	Response.InternalContainer = Container;
	Response.InternalSlotIndex = InternalSlotIndex;
	Response.Result = EPNEquipmentOperationResult::UnknownError;

	if (!HasEquipmentAuthority())
	{
		Response.Result = EPNEquipmentOperationResult::InvalidEquipment;
		return Response;
	}

	if (Container == EPNEquipmentInternalContainer::None)
	{
		Response.Result = EPNEquipmentOperationResult::InvalidInternalContainer;
		return Response;
	}

	if (InternalSlotIndex < 0 || InternalSlotIndex >= GetInternalSlotCount(Container))
	{
		Response.Result = EPNEquipmentOperationResult::InvalidInternalSlotIndex;
		return Response;
	}

	if (InternalSlotIndex >= GetUnlockedInternalSlotCount(Container))
	{
		Response.Result = EPNEquipmentOperationResult::InternalSlotLocked;
		return Response;
	}

	if (IsInternalSlotOccupied(Container, InternalSlotIndex))
	{
		Response.Result = EPNEquipmentOperationResult::SlotOccupied;
		return Response;
	}

	if (!GetEquippedContainerData(Container))
	{
		Response.Result = EPNEquipmentOperationResult::TopSlotRequired;
		return Response;
	}

	UPNInventoryComponent* InventoryComponent = GetOwnerInventoryComponent();
	if (!InventoryComponent)
	{
		Response.Result = EPNEquipmentOperationResult::InvalidInventory;
		return Response;
	}

	UPNItemInstance* SourceItem = InventoryComponent->GetItemAtPosition(InventoryPosition);
	if (!SourceItem || !SourceItem->IsValidItem())
	{
		Response.Result = EPNEquipmentOperationResult::InvalidItem;
		return Response;
	}

	if (!CanInsertItemIntoInternalSlot(SourceItem, Container, InternalSlotIndex))
	{
		Response.Result = EPNEquipmentOperationResult::ItemTypeNotAllowed;
		return Response;
	}

	FPNInventoryRemoveItemResult RemoveResult = InventoryComponent->RemoveItemAtPosition(InventoryPosition, 1);
	if (!RemoveResult.bSuccess || !RemoveResult.RemovedItemInstance || !RemoveResult.RemovedItemInstance->IsValidItem())
	{
		Response.Result = ConvertInventoryRemoveFail(RemoveResult.Result);
		return Response;
	}

	FPNRepItemInstanceData InternalData = RemoveResult.RemovedItemInstance->ToRepData();
	InternalData.Quantity = 1;

	SetInternalSlotItem(Container, InternalSlotIndex, InternalData);

	Response.bSuccess = true;
	Response.Result = EPNEquipmentOperationResult::Success;
	Response.ItemData = InternalData.ItemData;

	BroadcastEquipmentChanged();
	return Response;
}

FPNEquipmentOperationResponse UPNEquipmentComponent::RemoveInternalSlotToInventory(EPNEquipmentInternalContainer Container, int32 InternalSlotIndex)
{
	FPNEquipmentOperationResponse Response;
	Response.InternalContainer = Container;
	Response.InternalSlotIndex = InternalSlotIndex;
	Response.Result = EPNEquipmentOperationResult::UnknownError;

	if (!HasEquipmentAuthority())
	{
		Response.Result = EPNEquipmentOperationResult::InvalidEquipment;
		return Response;
	}

	if (Container == EPNEquipmentInternalContainer::None)
	{
		Response.Result = EPNEquipmentOperationResult::InvalidInternalContainer;
		return Response;
	}

	if (InternalSlotIndex < 0 || InternalSlotIndex >= GetInternalSlotCount(Container))
	{
		Response.Result = EPNEquipmentOperationResult::InvalidInternalSlotIndex;
		return Response;
	}

	const int32 SlotIndex = FindInternalSlotIndex(Container, InternalSlotIndex);
	const TArray<FPNEquipmentInternalSlotEntry>* InternalSlots = GetConstInternalSlots(Container);

	if (!InternalSlots || !InternalSlots->IsValidIndex(SlotIndex) || (*InternalSlots)[SlotIndex].IsEmpty())
	{
		Response.Result = EPNEquipmentOperationResult::SlotEmpty;
		return Response;
	}

	UPNInventoryComponent* InventoryComponent = GetOwnerInventoryComponent();
	if (!InventoryComponent)
	{
		Response.Result = EPNEquipmentOperationResult::InvalidInventory;
		return Response;
	}

	UPNItemInstance* ItemInstance = NewObject<UPNItemInstance>(this);
	if (!ItemInstance)
	{
		Response.Result = EPNEquipmentOperationResult::InvalidItem;
		return Response;
	}

	ItemInstance->InitializeFromRepData((*InternalSlots)[SlotIndex].InstanceData);
	if (!ItemInstance->IsValidItem())
	{
		Response.Result = EPNEquipmentOperationResult::InvalidItem;
		return Response;
	}

	FPNInventoryAddItemResult AddResult = InventoryComponent->AddItem(ItemInstance, true, true);
	if (!AddResult.bSuccess || AddResult.RemainingQuantity > 0)
	{
		Response.Result = ConvertInventoryAddFail(AddResult.Result);
		return Response;
	}

	Response.ItemData = ItemInstance->GetItemData();

	ClearInternalSlot(Container, InternalSlotIndex);

	Response.bSuccess = true;
	Response.Result = EPNEquipmentOperationResult::Success;

	BroadcastEquipmentChanged();
	return Response;
}

void UPNEquipmentComponent::RequestEquipFromInventoryToSlot(FPNInventoryGridPosition InventoryPosition, EPNEquipmentSlot TargetSlot)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	if (OwnerActor->HasAuthority())
	{
		EquipFromInventoryToSlot(InventoryPosition, TargetSlot);
		return;
	}

	Server_EquipFromInventoryToSlot(InventoryPosition, TargetSlot);
}

void UPNEquipmentComponent::RequestUnequipSlotToInventory(EPNEquipmentSlot Slot)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	if (OwnerActor->HasAuthority())
	{
		UnequipSlotToInventory(Slot);
		return;
	}

	Server_UnequipSlotToInventory(Slot);
}

void UPNEquipmentComponent::RequestInsertFromInventoryToInternalSlot(FPNInventoryGridPosition InventoryPosition, EPNEquipmentInternalContainer Container, int32 InternalSlotIndex)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	if (OwnerActor->HasAuthority())
	{
		InsertFromInventoryToInternalSlot(InventoryPosition, Container, InternalSlotIndex);
		return;
	}

	Server_InsertFromInventoryToInternalSlot(InventoryPosition, Container, InternalSlotIndex);
}

void UPNEquipmentComponent::RequestRemoveInternalSlotToInventory(EPNEquipmentInternalContainer Container, int32 InternalSlotIndex)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	if (OwnerActor->HasAuthority())
	{
		RemoveInternalSlotToInventory(Container, InternalSlotIndex);
		return;
	}

	Server_RemoveInternalSlotToInventory(Container, InternalSlotIndex);
}

void UPNEquipmentComponent::Server_EquipFromInventoryToSlot_Implementation(FPNInventoryGridPosition InventoryPosition, EPNEquipmentSlot TargetSlot)
{
	EquipFromInventoryToSlot(InventoryPosition, TargetSlot);
}

void UPNEquipmentComponent::Server_UnequipSlotToInventory_Implementation(EPNEquipmentSlot Slot)
{
	UnequipSlotToInventory(Slot);
}

void UPNEquipmentComponent::Server_InsertFromInventoryToInternalSlot_Implementation(FPNInventoryGridPosition InventoryPosition, EPNEquipmentInternalContainer Container, int32 InternalSlotIndex)
{
	InsertFromInventoryToInternalSlot(InventoryPosition, Container, InternalSlotIndex);
}

void UPNEquipmentComponent::Server_RemoveInternalSlotToInventory_Implementation(EPNEquipmentInternalContainer Container, int32 InternalSlotIndex)
{
	RemoveInternalSlotToInventory(Container, InternalSlotIndex);
}

bool UPNEquipmentComponent::IsEquipmentSlotValid(EPNEquipmentSlot Slot) const
{
	return Slot != EPNEquipmentSlot::None;
}

bool UPNEquipmentComponent::IsEquipmentSlotOccupied(EPNEquipmentSlot Slot) const
{
	const int32 SlotIndex = FindEquipmentSlotIndex(Slot);
	if (!EquipmentSlots.IsValidIndex(SlotIndex))
	{
		return false;
	}

	return EquipmentSlots[SlotIndex].IsOccupied();
}

bool UPNEquipmentComponent::IsInternalSlotOccupied(EPNEquipmentInternalContainer Container, int32 InternalSlotIndex) const
{
	const int32 SlotIndex = FindInternalSlotIndex(Container, InternalSlotIndex);
	const TArray<FPNEquipmentInternalSlotEntry>* InternalSlots = GetConstInternalSlots(Container);

	if (!InternalSlots || !InternalSlots->IsValidIndex(SlotIndex))
	{
		return false;
	}

	return (*InternalSlots)[SlotIndex].IsOccupied();
}

int32 UPNEquipmentComponent::GetInternalSlotCount(EPNEquipmentInternalContainer Container) const
{
	const TArray<FPNEquipmentInternalSlotEntry>* InternalSlots = GetConstInternalSlots(Container);

	return InternalSlots ? InternalSlots->Num() : 0;
}

int32 UPNEquipmentComponent::GetUnlockedInternalSlotCount(EPNEquipmentInternalContainer Container) const
{
	UPNItemDataAsset* ContainerData = GetEquippedContainerData(Container);
	if (!ContainerData)
	{
		return 0;
	}

	const int32 DefaultSlotCount = Container == EPNEquipmentInternalContainer::Helmet
		? PNEquipmentConstants::HelmetInternalSlotCount
		: PNEquipmentConstants::ArmorInternalSlotCount;

	const FPNArmorSlotData& SlotData = ContainerData->ArmorStats.Slots;

	if (SlotData.MaxSlots <= 0)
	{
		return DefaultSlotCount;
	}

	const int32 MaxSlots = FMath::Clamp(SlotData.MaxSlots, 0, DefaultSlotCount);

	if (SlotData.DefaultUnlockedSlots <= 0)
	{
		return MaxSlots;
	}

	return FMath::Clamp(SlotData.DefaultUnlockedSlots, 0, MaxSlots);
}

FPNEquipmentSlotEntry UPNEquipmentComponent::GetEquipmentSlotEntry(EPNEquipmentSlot Slot) const
{
	const int32 SlotIndex = FindEquipmentSlotIndex(Slot);
	if (!EquipmentSlots.IsValidIndex(SlotIndex))
	{
		FPNEquipmentSlotEntry EmptyEntry;
		EmptyEntry.Slot = Slot;
		return EmptyEntry;
	}

	return EquipmentSlots[SlotIndex];
}

FPNEquipmentInternalSlotEntry UPNEquipmentComponent::GetInternalSlotEntry(EPNEquipmentInternalContainer Container, int32 InternalSlotIndex) const
{
	const int32 SlotIndex = FindInternalSlotIndex(Container, InternalSlotIndex);
	const TArray<FPNEquipmentInternalSlotEntry>* InternalSlots = GetConstInternalSlots(Container);

	if (!InternalSlots || !InternalSlots->IsValidIndex(SlotIndex))
	{
		FPNEquipmentInternalSlotEntry EmptyEntry;
		EmptyEntry.Container = Container;
		EmptyEntry.SlotIndex = InternalSlotIndex;
		return EmptyEntry;
	}

	return (*InternalSlots)[SlotIndex];
}

UPNItemDataAsset* UPNEquipmentComponent::GetEquippedItemData(EPNEquipmentSlot Slot) const
{
	const int32 SlotIndex = FindEquipmentSlotIndex(Slot);
	if (!EquipmentSlots.IsValidIndex(SlotIndex))
	{
		return nullptr;
	}

	return EquipmentSlots[SlotIndex].InstanceData.ItemData;
}

UPNItemDataAsset* UPNEquipmentComponent::GetInternalSlotItemData(EPNEquipmentInternalContainer Container, int32 InternalSlotIndex) const
{
	const int32 SlotIndex = FindInternalSlotIndex(Container, InternalSlotIndex);
	const TArray<FPNEquipmentInternalSlotEntry>* InternalSlots = GetConstInternalSlots(Container);

	if (!InternalSlots || !InternalSlots->IsValidIndex(SlotIndex))
	{
		return nullptr;
	}

	return (*InternalSlots)[SlotIndex].InstanceData.ItemData;
}

const TArray<FPNEquipmentSlotEntry>& UPNEquipmentComponent::GetEquipmentSlots() const
{
	return EquipmentSlots;
}

const TArray<FPNEquipmentInternalSlotEntry>& UPNEquipmentComponent::GetHelmetInternalSlots() const
{
	return HelmetInternalSlots;
}

const TArray<FPNEquipmentInternalSlotEntry>& UPNEquipmentComponent::GetArmorInternalSlots() const
{
	return ArmorInternalSlots;
}

bool UPNEquipmentComponent::CanEquipItemToSlot(UPNItemInstance* ItemInstance, EPNEquipmentSlot Slot) const
{
	if (!ItemInstance || !ItemInstance->IsValidItem())
	{
		return false;
	}

	UPNItemDataAsset* ItemData = ItemInstance->GetItemData();
	if (!ItemData)
	{
		return false;
	}

	switch (Slot)
	{
	case EPNEquipmentSlot::Helmet:
		return ItemData->ItemType == EPNItemType::IT_HArmor;

	case EPNEquipmentSlot::Gloves:
		return ItemData->ItemType == EPNItemType::IT_Gloves
			|| ItemData->ItemCategory == EPNItemCategory::Gloves;

	case EPNEquipmentSlot::Armor:
		return ItemData->ItemType == EPNItemType::IT_Armor;

	case EPNEquipmentSlot::Backpack:
		return ItemData->ItemType == EPNItemType::IT_Backpack;

	case EPNEquipmentSlot::PrimaryWeapon1:
	case EPNEquipmentSlot::PrimaryWeapon2:
		return ItemData->ItemType == EPNItemType::IT_Weapon
			&& IsPrimaryWeaponCategory(ItemData->ItemCategory);

	case EPNEquipmentSlot::Sidearm:
		return ItemData->ItemType == EPNItemType::IT_Weapon
			&& IsSidearmCategory(ItemData->ItemCategory);

	case EPNEquipmentSlot::Knife:
		return ItemData->ItemType == EPNItemType::IT_Weapon
			&& ItemData->ItemCategory == EPNItemCategory::Melee;

	default:
		return false;
	}
}

bool UPNEquipmentComponent::CanInsertItemIntoInternalSlot(UPNItemInstance* ItemInstance, EPNEquipmentInternalContainer Container, int32 InternalSlotIndex) const
{
	if (!ItemInstance || !ItemInstance->IsValidItem())
	{
		return false;
	}

	if (InternalSlotIndex < 0 || InternalSlotIndex >= GetUnlockedInternalSlotCount(Container))
	{
		return false;
	}

	UPNItemDataAsset* ItemData = ItemInstance->GetItemData();
	if (!ItemData || ItemData->ItemType != EPNItemType::IT_Armor_ATTM)
	{
		return false;
	}

	UPNItemDataAsset* ContainerData = GetEquippedContainerData(Container);
	if (!ContainerData)
	{
		return false;
	}

	const FPNArmorSlotData& SlotData = ContainerData->ArmorStats.Slots;

	const bool bHasSlotRules =
		SlotData.bUsesPlate ||
		SlotData.bUsesArtifacts ||
		SlotData.bUsesMask ||
		SlotData.bUsesHDevice;

	if (Container == EPNEquipmentInternalContainer::Helmet)
	{
		if (ItemData->ItemCategory == EPNItemCategory::Mask)
		{
			return !bHasSlotRules || SlotData.bUsesMask;
		}

		if (ItemData->ItemCategory == EPNItemCategory::HDevice)
		{
			return !bHasSlotRules || SlotData.bUsesHDevice;
		}

		if (ItemData->ItemCategory == EPNItemCategory::Artifacts)
		{
			return !bHasSlotRules || SlotData.bUsesArtifacts;
		}

		return false;
	}

	if (Container == EPNEquipmentInternalContainer::Armor)
	{
		if (ItemData->ItemCategory == EPNItemCategory::Plate)
		{
			return !bHasSlotRules || SlotData.bUsesPlate;
		}

		if (ItemData->ItemCategory == EPNItemCategory::Artifacts)
		{
			return !bHasSlotRules || SlotData.bUsesArtifacts;
		}

		return false;
	}

	return false;
}

FString UPNEquipmentComponent::GetEquipmentDebugString() const
{
	const AActor* OwnerActor = GetOwner();

	const FString NetSide = OwnerActor && OwnerActor->HasAuthority()
		? TEXT("SERVER")
		: TEXT("CLIENT");

	const FString OwnerName = OwnerActor
		? OwnerActor->GetName()
		: TEXT("NoOwner");

	FString Result = FString::Printf(
		TEXT("[%s] EquipmentComponent: %s\nTop Slots: %d | HelmetSlots: %d | ArmorSlots: %d"),
		*NetSide,
		*OwnerName,
		EquipmentSlots.Num(),
		HelmetInternalSlots.Num(),
		ArmorInternalSlots.Num()
	);

	for (int32 Index = 0; Index < EquipmentSlots.Num(); ++Index)
	{
		const FPNEquipmentSlotEntry& SlotEntry = EquipmentSlots[Index];

		const UPNItemDataAsset* ItemData = SlotEntry.InstanceData.ItemData;
		const FString ItemName = ItemData
			? (ItemData->GetItemName().IsEmpty() ? ItemData->GetItemId().ToString() : ItemData->GetItemName().ToString())
			: TEXT("Empty");

		Result += FString::Printf(
			TEXT("\nTOP [%d] Slot:%s Item:%s x%d Dur:%.1f Bat:%.1f Exp:%.1f Ammo:%d"),
			Index,
			*UEnum::GetValueAsString(SlotEntry.Slot),
			*ItemName,
			SlotEntry.InstanceData.Quantity,
			SlotEntry.InstanceData.CurrentDurability,
			SlotEntry.InstanceData.CurrentBatteryCharge,
			SlotEntry.InstanceData.RemainingShelfLifeSeconds,
			SlotEntry.InstanceData.AmmoInMagazine
		);
	}

	for (int32 Index = 0; Index < HelmetInternalSlots.Num(); ++Index)
	{
		const FPNEquipmentInternalSlotEntry& SlotEntry = HelmetInternalSlots[Index];

		const UPNItemDataAsset* ItemData = SlotEntry.InstanceData.ItemData;
		const FString ItemName = ItemData
			? (ItemData->GetItemName().IsEmpty() ? ItemData->GetItemId().ToString() : ItemData->GetItemName().ToString())
			: TEXT("Empty");

		Result += FString::Printf(
			TEXT("\nHELMET [%d] Item:%s x%d"),
			SlotEntry.SlotIndex,
			*ItemName,
			SlotEntry.InstanceData.Quantity
		);
	}

	for (int32 Index = 0; Index < ArmorInternalSlots.Num(); ++Index)
	{
		const FPNEquipmentInternalSlotEntry& SlotEntry = ArmorInternalSlots[Index];

		const UPNItemDataAsset* ItemData = SlotEntry.InstanceData.ItemData;
		const FString ItemName = ItemData
			? (ItemData->GetItemName().IsEmpty() ? ItemData->GetItemId().ToString() : ItemData->GetItemName().ToString())
			: TEXT("Empty");

		Result += FString::Printf(
			TEXT("\nARMOR [%d] Item:%s x%d"),
			SlotEntry.SlotIndex,
			*ItemName,
			SlotEntry.InstanceData.Quantity
		);
	}

	return Result;
}

void UPNEquipmentComponent::PrintEquipmentDebug() const
{
	const FString DebugText = GetEquipmentDebugString();

	UE_LOG(LogTemp, Warning, TEXT("%s"), *DebugText);

	if (GEngine)
	{
		const int32 DebugKey = GetOwner()
			? static_cast<int32>(GetOwner()->GetUniqueID()) + 10000
			: INDEX_NONE;

		GEngine->AddOnScreenDebugMessage(
			DebugKey,
			6.0f,
			FColor::Cyan,
			DebugText
		);
	}
}

void UPNEquipmentComponent::OnRep_EquipmentSlots()
{
	OnEquipmentChanged.Broadcast();

	if (bDebugEquipmentReplication)
	{
		PrintEquipmentDebug();
	}
}

void UPNEquipmentComponent::OnRep_HelmetInternalSlots()
{
	OnEquipmentChanged.Broadcast();

	if (bDebugEquipmentReplication)
	{
		PrintEquipmentDebug();
	}
}

void UPNEquipmentComponent::OnRep_ArmorInternalSlots()
{
	OnEquipmentChanged.Broadcast();

	if (bDebugEquipmentReplication)
	{
		PrintEquipmentDebug();
	}
}

bool UPNEquipmentComponent::HasEquipmentAuthority() const
{
	const AActor* OwnerActor = GetOwner();
	return !OwnerActor || OwnerActor->HasAuthority();
}

UPNInventoryComponent* UPNEquipmentComponent::GetOwnerInventoryComponent() const
{
	const APNBaseCharacter* OwnerCharacter = Cast<APNBaseCharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		return nullptr;
	}

	return OwnerCharacter->GetInventoryComponent();
}

void UPNEquipmentComponent::BuildDefaultEquipmentSlots()
{
	const TArray<EPNEquipmentSlot> DefaultSlots =
	{
		EPNEquipmentSlot::Helmet,
		EPNEquipmentSlot::Gloves,
		EPNEquipmentSlot::Armor,
		EPNEquipmentSlot::Backpack,
		EPNEquipmentSlot::PrimaryWeapon1,
		EPNEquipmentSlot::PrimaryWeapon2,
		EPNEquipmentSlot::Sidearm,
		EPNEquipmentSlot::Knife
	};

	for (EPNEquipmentSlot Slot : DefaultSlots)
	{
		FindOrCreateEquipmentSlotIndex(Slot);
	}
}

void UPNEquipmentComponent::BuildInternalSlots(EPNEquipmentInternalContainer Container, int32 SlotCount)
{
	for (int32 Index = 0; Index < SlotCount; ++Index)
	{
		FindOrCreateInternalSlotIndex(Container, Index);
	}
}

int32 UPNEquipmentComponent::FindEquipmentSlotIndex(EPNEquipmentSlot Slot) const
{
	for (int32 Index = 0; Index < EquipmentSlots.Num(); ++Index)
	{
		if (EquipmentSlots[Index].Slot == Slot)
		{
			return Index;
		}
	}

	return INDEX_NONE;
}

int32 UPNEquipmentComponent::FindOrCreateEquipmentSlotIndex(EPNEquipmentSlot Slot)
{
	if (!IsEquipmentSlotValid(Slot))
	{
		return INDEX_NONE;
	}

	const int32 ExistingIndex = FindEquipmentSlotIndex(Slot);
	if (ExistingIndex != INDEX_NONE)
	{
		return ExistingIndex;
	}

	FPNEquipmentSlotEntry NewEntry;
	NewEntry.Slot = Slot;

	return EquipmentSlots.Add(NewEntry);
}

TArray<FPNEquipmentInternalSlotEntry>* UPNEquipmentComponent::GetMutableInternalSlots(EPNEquipmentInternalContainer Container)
{
	if (Container == EPNEquipmentInternalContainer::Helmet)
	{
		return &HelmetInternalSlots;
	}

	if (Container == EPNEquipmentInternalContainer::Armor)
	{
		return &ArmorInternalSlots;
	}

	return nullptr;
}

const TArray<FPNEquipmentInternalSlotEntry>* UPNEquipmentComponent::GetConstInternalSlots(EPNEquipmentInternalContainer Container) const
{
	if (Container == EPNEquipmentInternalContainer::Helmet)
	{
		return &HelmetInternalSlots;
	}

	if (Container == EPNEquipmentInternalContainer::Armor)
	{
		return &ArmorInternalSlots;
	}

	return nullptr;
}

int32 UPNEquipmentComponent::FindInternalSlotIndex(EPNEquipmentInternalContainer Container, int32 InternalSlotIndex) const
{
	const TArray<FPNEquipmentInternalSlotEntry>* InternalSlots = GetConstInternalSlots(Container);
	if (!InternalSlots)
	{
		return INDEX_NONE;
	}

	for (int32 Index = 0; Index < InternalSlots->Num(); ++Index)
	{
		if ((*InternalSlots)[Index].Container == Container && (*InternalSlots)[Index].SlotIndex == InternalSlotIndex)
		{
			return Index;
		}
	}

	return INDEX_NONE;
}

int32 UPNEquipmentComponent::FindOrCreateInternalSlotIndex(EPNEquipmentInternalContainer Container, int32 InternalSlotIndex)
{
	TArray<FPNEquipmentInternalSlotEntry>* InternalSlots = GetMutableInternalSlots(Container);
	if (!InternalSlots)
	{
		return INDEX_NONE;
	}

	const int32 ExistingIndex = FindInternalSlotIndex(Container, InternalSlotIndex);
	if (ExistingIndex != INDEX_NONE)
	{
		return ExistingIndex;
	}

	FPNEquipmentInternalSlotEntry NewEntry;
	NewEntry.Container = Container;
	NewEntry.SlotIndex = InternalSlotIndex;

	return InternalSlots->Add(NewEntry);
}

void UPNEquipmentComponent::SetEquipmentSlotItem(EPNEquipmentSlot Slot, const FPNRepItemInstanceData& InstanceData)
{
	const int32 SlotIndex = FindOrCreateEquipmentSlotIndex(Slot);
	if (!EquipmentSlots.IsValidIndex(SlotIndex))
	{
		return;
	}

	EquipmentSlots[SlotIndex].InstanceData = InstanceData;
}

void UPNEquipmentComponent::ClearEquipmentSlot(EPNEquipmentSlot Slot)
{
	const int32 SlotIndex = FindEquipmentSlotIndex(Slot);
	if (!EquipmentSlots.IsValidIndex(SlotIndex))
	{
		return;
	}

	EquipmentSlots[SlotIndex].InstanceData = FPNRepItemInstanceData();
}

void UPNEquipmentComponent::SetInternalSlotItem(EPNEquipmentInternalContainer Container, int32 InternalSlotIndex, const FPNRepItemInstanceData& InstanceData)
{
	const int32 SlotIndex = FindOrCreateInternalSlotIndex(Container, InternalSlotIndex);

	TArray<FPNEquipmentInternalSlotEntry>* InternalSlots = GetMutableInternalSlots(Container);
	if (!InternalSlots || !InternalSlots->IsValidIndex(SlotIndex))
	{
		return;
	}

	(*InternalSlots)[SlotIndex].InstanceData = InstanceData;
}

void UPNEquipmentComponent::ClearInternalSlot(EPNEquipmentInternalContainer Container, int32 InternalSlotIndex)
{
	const int32 SlotIndex = FindInternalSlotIndex(Container, InternalSlotIndex);

	TArray<FPNEquipmentInternalSlotEntry>* InternalSlots = GetMutableInternalSlots(Container);
	if (!InternalSlots || !InternalSlots->IsValidIndex(SlotIndex))
	{
		return;
	}

	(*InternalSlots)[SlotIndex].InstanceData = FPNRepItemInstanceData();
}

bool UPNEquipmentComponent::HasAnyInternalItemsForTopSlot(EPNEquipmentSlot Slot) const
{
	if (Slot == EPNEquipmentSlot::Helmet)
	{
		for (const FPNEquipmentInternalSlotEntry& InternalSlot : HelmetInternalSlots)
		{
			if (InternalSlot.IsOccupied())
			{
				return true;
			}
		}
	}

	if (Slot == EPNEquipmentSlot::Armor)
	{
		for (const FPNEquipmentInternalSlotEntry& InternalSlot : ArmorInternalSlots)
		{
			if (InternalSlot.IsOccupied())
			{
				return true;
			}
		}
	}

	return false;
}

UPNItemDataAsset* UPNEquipmentComponent::GetEquippedContainerData(EPNEquipmentInternalContainer Container) const
{
	if (Container == EPNEquipmentInternalContainer::Helmet)
	{
		return GetEquippedItemData(EPNEquipmentSlot::Helmet);
	}

	if (Container == EPNEquipmentInternalContainer::Armor)
	{
		return GetEquippedItemData(EPNEquipmentSlot::Armor);
	}

	return nullptr;
}

bool UPNEquipmentComponent::IsPrimaryWeaponCategory(EPNItemCategory Category) const
{
	return Category == EPNItemCategory::ASR_Type1
		|| Category == EPNItemCategory::ASR_Type2
		|| Category == EPNItemCategory::SNP
		|| Category == EPNItemCategory::SHTG
		|| Category == EPNItemCategory::SMG
		|| Category == EPNItemCategory::MG
		|| Category == EPNItemCategory::RPG;
}

bool UPNEquipmentComponent::IsSidearmCategory(EPNItemCategory Category) const
{
	return Category == EPNItemCategory::HG_Single
		|| Category == EPNItemCategory::HG_Knife
		|| Category == EPNItemCategory::HG_Shield
		|| Category == EPNItemCategory::HG_Items;
}

EPNEquipmentOperationResult UPNEquipmentComponent::ConvertInventoryAddFail(EPNInventoryOperationResult InventoryResult) const
{
	switch (InventoryResult)
	{
	case EPNInventoryOperationResult::NoSpace:
		return EPNEquipmentOperationResult::NoSpace;

	case EPNInventoryOperationResult::OverWeight:
		return EPNEquipmentOperationResult::OverWeight;

	case EPNInventoryOperationResult::InvalidInventory:
		return EPNEquipmentOperationResult::InvalidInventory;

	default:
		return EPNEquipmentOperationResult::InventoryAddFailed;
	}
}

EPNEquipmentOperationResult UPNEquipmentComponent::ConvertInventoryRemoveFail(EPNInventoryOperationResult InventoryResult) const
{
	switch (InventoryResult)
	{
	case EPNInventoryOperationResult::SlotEmpty:
		return EPNEquipmentOperationResult::SlotEmpty;

	case EPNInventoryOperationResult::InvalidInventory:
		return EPNEquipmentOperationResult::InvalidInventory;

	case EPNInventoryOperationResult::InvalidItem:
		return EPNEquipmentOperationResult::InvalidItem;

	default:
		return EPNEquipmentOperationResult::InventoryRemoveFailed;
	}
}

void UPNEquipmentComponent::BroadcastEquipmentChanged()
{
	OnEquipmentChanged.Broadcast();

	if (bDebugEquipmentReplication)
	{
		PrintEquipmentDebug();
	}
}