#include "Items/PNQuickSlotComponent.h"

#include "Characters/PNBaseCharacter.h"
#include "Engine/Engine.h"
#include "Items/PNInventoryComponent.h"
#include "Items/PNItemDataAsset.h"
#include "Items/PNItemInstance.h"
#include "Net/UnrealNetwork.h"
#include "Stats/PNCharacterStatsComponent.h"

UPNQuickSlotComponent::UPNQuickSlotComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}

void UPNQuickSlotComponent::BeginPlay()
{
	Super::BeginPlay();

	if (HasQuickSlotAuthority())
	{
		InitializeQuickSlots();
	}
}

void UPNQuickSlotComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPNQuickSlotComponent, QuickSlots);
	DOREPLIFETIME(UPNQuickSlotComponent, SelectedQuickSlotIndex);
}

void UPNQuickSlotComponent::InitializeQuickSlots()
{
	if (!HasQuickSlotAuthority())
	{
		return;
	}

	QuickSlotCount = FMath::Clamp(QuickSlotCount, 1, 12);
	BuildDefaultQuickSlots();

	BroadcastQuickSlotsChanged();
}

FPNQuickSlotOperationResponse UPNQuickSlotComponent::AssignFromInventoryPosition(FPNInventoryGridPosition InventoryPosition, int32 SlotIndex)
{
	FPNQuickSlotOperationResponse Response;
	Response.SlotIndex = SlotIndex;
	Response.Result = EPNQuickSlotOperationResult::UnknownError;

	if (!HasQuickSlotAuthority())
	{
		Response.Result = EPNQuickSlotOperationResult::InvalidComponent;
		return Response;
	}

	if (!IsValidQuickSlotIndex(SlotIndex))
	{
		Response.Result = EPNQuickSlotOperationResult::InvalidSlotIndex;
		return Response;
	}

	if (IsQuickSlotOccupied(SlotIndex))
	{
		Response.Result = EPNQuickSlotOperationResult::SlotOccupied;
		return Response;
	}

	UPNInventoryComponent* InventoryComponent = GetOwnerInventoryComponent();
	if (!InventoryComponent)
	{
		Response.Result = EPNQuickSlotOperationResult::InvalidInventory;
		return Response;
	}

	UPNItemInstance* SourceItem = InventoryComponent->GetItemAtPosition(InventoryPosition);
	if (!SourceItem || !SourceItem->IsValidItem() || !SourceItem->GetItemData())
	{
		Response.Result = EPNQuickSlotOperationResult::InvalidItem;
		return Response;
	}

	if (!CanPlaceItemDataIntoQuickSlot(SourceItem->GetItemData()))
	{
		Response.Result = EPNQuickSlotOperationResult::ItemTypeNotAllowed;
		return Response;
	}

	const int32 QuantityToMove = FMath::Max(1, SourceItem->Quantity);

	FPNInventoryRemoveItemResult RemoveResult = InventoryComponent->RemoveItemAtPosition(InventoryPosition, QuantityToMove);
	if (!RemoveResult.bSuccess || !RemoveResult.RemovedItemInstance || !RemoveResult.RemovedItemInstance->IsValidItem())
	{
		Response.Result = ConvertInventoryRemoveFail(RemoveResult.Result);
		return Response;
	}

	FPNRepItemInstanceData SlotData = RemoveResult.RemovedItemInstance->ToRepData();
	if (!SlotData.IsValid())
	{
		Response.Result = EPNQuickSlotOperationResult::InvalidItem;
		return Response;
	}

	SetQuickSlotData(SlotIndex, SlotData);

	Response.bSuccess = true;
	Response.Result = EPNQuickSlotOperationResult::Success;
	Response.ItemData = SlotData.ItemData;
	Response.Quantity = SlotData.Quantity;

	BroadcastQuickSlotsChanged();

	return Response;
}

FPNQuickSlotOperationResponse UPNQuickSlotComponent::ClearQuickSlotToInventory(int32 SlotIndex)
{
	FPNQuickSlotOperationResponse Response;
	Response.SlotIndex = SlotIndex;
	Response.Result = EPNQuickSlotOperationResult::UnknownError;

	if (!HasQuickSlotAuthority())
	{
		Response.Result = EPNQuickSlotOperationResult::InvalidComponent;
		return Response;
	}

	if (!IsValidQuickSlotIndex(SlotIndex))
	{
		Response.Result = EPNQuickSlotOperationResult::InvalidSlotIndex;
		return Response;
	}

	const int32 SlotArrayIndex = FindQuickSlotArrayIndex(SlotIndex);
	if (!QuickSlots.IsValidIndex(SlotArrayIndex) || QuickSlots[SlotArrayIndex].IsEmpty())
	{
		Response.Result = EPNQuickSlotOperationResult::SlotEmpty;
		return Response;
	}

	UPNInventoryComponent* InventoryComponent = GetOwnerInventoryComponent();
	if (!InventoryComponent)
	{
		Response.Result = EPNQuickSlotOperationResult::InvalidInventory;
		return Response;
	}

	UPNItemInstance* ItemInstance = NewObject<UPNItemInstance>(this);
	if (!ItemInstance)
	{
		Response.Result = EPNQuickSlotOperationResult::InvalidItem;
		return Response;
	}

	ItemInstance->InitializeFromRepData(QuickSlots[SlotArrayIndex].InstanceData);

	if (!ItemInstance->IsValidItem())
	{
		Response.Result = EPNQuickSlotOperationResult::InvalidItem;
		return Response;
	}

	const int32 OriginalQuantity = ItemInstance->Quantity;

	FPNInventoryAddItemResult AddResult = InventoryComponent->AddItem(ItemInstance, true, true);
	if (AddResult.AddedQuantity <= 0)
	{
		Response.Result = ConvertInventoryAddFail(AddResult.Result);
		return Response;
	}

	Response.ItemData = ItemInstance->GetItemData();
	Response.Quantity = AddResult.AddedQuantity;

	if (AddResult.RemainingQuantity <= 0)
	{
		ClearQuickSlotData(SlotIndex);

		if (SelectedQuickSlotIndex == SlotIndex)
		{
			SelectedQuickSlotIndex = INDEX_NONE;
			BroadcastSelectedQuickSlot();
		}

		Response.bSuccess = true;
		Response.Result = EPNQuickSlotOperationResult::Success;
	}
	else
	{
		QuickSlots[SlotArrayIndex].InstanceData.Quantity = FMath::Clamp(AddResult.RemainingQuantity, 1, OriginalQuantity);
		Response.bSuccess = false;
		Response.Result = EPNQuickSlotOperationResult::InventoryAddFailed;
	}

	BroadcastQuickSlotsChanged();

	return Response;
}

FPNQuickSlotOperationResponse UPNQuickSlotComponent::SelectQuickSlot(int32 SlotIndex)
{
	FPNQuickSlotOperationResponse Response;
	Response.SlotIndex = SlotIndex;
	Response.Result = EPNQuickSlotOperationResult::UnknownError;

	if (!HasQuickSlotAuthority())
	{
		Response.Result = EPNQuickSlotOperationResult::InvalidComponent;
		return Response;
	}

	if (!IsValidQuickSlotIndex(SlotIndex))
	{
		Response.Result = EPNQuickSlotOperationResult::InvalidSlotIndex;
		return Response;
	}

	SelectedQuickSlotIndex = SlotIndex;

	const FPNQuickSlotEntry SlotEntry = GetQuickSlotEntry(SlotIndex);
	Response.ItemData = SlotEntry.InstanceData.ItemData;
	Response.Quantity = SlotEntry.InstanceData.Quantity;
	Response.bSuccess = true;
	Response.Result = EPNQuickSlotOperationResult::Success;

	BroadcastSelectedQuickSlot();

	return Response;
}

FPNQuickSlotOperationResponse UPNQuickSlotComponent::ActivateQuickSlot(int32 SlotIndex, bool bDoubleClick)
{
	FPNQuickSlotOperationResponse Response;
	Response.SlotIndex = SlotIndex;
	Response.Result = EPNQuickSlotOperationResult::UnknownError;

	if (!IsValidQuickSlotIndex(SlotIndex))
	{
		Response.Result = EPNQuickSlotOperationResult::InvalidSlotIndex;
		return Response;
	}

	const FPNQuickSlotEntry SlotEntry = GetQuickSlotEntry(SlotIndex);

	if (!SlotEntry.IsOccupied())
	{
		return SelectQuickSlot(SlotIndex);
	}

	UPNItemDataAsset* ItemData = SlotEntry.InstanceData.ItemData;
	if (!ItemData)
	{
		Response.Result = EPNQuickSlotOperationResult::InvalidItem;
		return Response;
	}

	if (IsConsumableItemData(ItemData))
	{
		if (bDoubleClick)
		{
			return UseQuickSlot(SlotIndex);
		}

		return SelectQuickSlot(SlotIndex);
	}

	return SelectQuickSlot(SlotIndex);
}

FPNQuickSlotOperationResponse UPNQuickSlotComponent::UseQuickSlot(int32 SlotIndex)
{
	FPNQuickSlotOperationResponse Response;
	Response.SlotIndex = SlotIndex;
	Response.Result = EPNQuickSlotOperationResult::UnknownError;

	if (!HasQuickSlotAuthority())
	{
		Response.Result = EPNQuickSlotOperationResult::InvalidComponent;
		return Response;
	}

	if (!IsValidQuickSlotIndex(SlotIndex))
	{
		Response.Result = EPNQuickSlotOperationResult::InvalidSlotIndex;
		return Response;
	}

	const int32 SlotArrayIndex = FindQuickSlotArrayIndex(SlotIndex);
	if (!QuickSlots.IsValidIndex(SlotArrayIndex) || QuickSlots[SlotArrayIndex].IsEmpty())
	{
		Response.Result = EPNQuickSlotOperationResult::SlotEmpty;
		return Response;
	}

	UPNItemDataAsset* ItemData = QuickSlots[SlotArrayIndex].InstanceData.ItemData;
	if (!ItemData)
	{
		Response.Result = EPNQuickSlotOperationResult::InvalidItem;
		return Response;
	}

	if (!IsConsumableItemData(ItemData))
	{
		Response.Result = EPNQuickSlotOperationResult::ItemNotConsumable;
		return Response;
	}

	if (!GetOwnerCharacterStatsComponent())
	{
		Response.Result = EPNQuickSlotOperationResult::InvalidCharacterStats;
		return Response;
	}

	if (!ApplyConsumableEffects(QuickSlots[SlotArrayIndex].InstanceData))
	{
		Response.Result = EPNQuickSlotOperationResult::InvalidItem;
		return Response;
	}

	QuickSlots[SlotArrayIndex].InstanceData.Quantity--;

	Response.ItemData = ItemData;
	Response.Quantity = FMath::Max(0, QuickSlots[SlotArrayIndex].InstanceData.Quantity);
	Response.bSuccess = true;
	Response.Result = EPNQuickSlotOperationResult::Success;

	if (QuickSlots[SlotArrayIndex].InstanceData.Quantity <= 0)
	{
		ClearQuickSlotData(SlotIndex);
	}

	BroadcastQuickSlotsChanged();

	if (SelectedQuickSlotIndex == SlotIndex)
	{
		BroadcastSelectedQuickSlot();
	}

	return Response;
}

FPNQuickSlotOperationResponse UPNQuickSlotComponent::UseInventoryConsumable(FPNInventoryGridPosition InventoryPosition)
{
	FPNQuickSlotOperationResponse Response;
	Response.Result = EPNQuickSlotOperationResult::UnknownError;

	if (!HasQuickSlotAuthority())
	{
		Response.Result = EPNQuickSlotOperationResult::InvalidComponent;
		return Response;
	}

	UPNInventoryComponent* InventoryComponent = GetOwnerInventoryComponent();
	if (!InventoryComponent)
	{
		Response.Result = EPNQuickSlotOperationResult::InvalidInventory;
		return Response;
	}

	UPNItemInstance* SourceItem = InventoryComponent->GetItemAtPosition(InventoryPosition);
	if (!SourceItem || !SourceItem->IsValidItem() || !SourceItem->GetItemData())
	{
		Response.Result = EPNQuickSlotOperationResult::InvalidItem;
		return Response;
	}

	if (!IsConsumableItemData(SourceItem->GetItemData()))
	{
		Response.Result = EPNQuickSlotOperationResult::ItemNotConsumable;
		return Response;
	}

	if (!GetOwnerCharacterStatsComponent())
	{
		Response.Result = EPNQuickSlotOperationResult::InvalidCharacterStats;
		return Response;
	}

	FPNInventoryRemoveItemResult RemoveResult = InventoryComponent->RemoveItemAtPosition(InventoryPosition, 1);
	if (!RemoveResult.bSuccess || !RemoveResult.RemovedItemInstance || !RemoveResult.RemovedItemInstance->IsValidItem())
	{
		Response.Result = ConvertInventoryRemoveFail(RemoveResult.Result);
		return Response;
	}

	FPNRepItemInstanceData UsedData = RemoveResult.RemovedItemInstance->ToRepData();

	if (!ApplyConsumableEffects(UsedData))
	{
		Response.Result = EPNQuickSlotOperationResult::InvalidItem;
		return Response;
	}

	Response.bSuccess = true;
	Response.Result = EPNQuickSlotOperationResult::Success;
	Response.ItemData = UsedData.ItemData;
	Response.Quantity = 1;

	BroadcastQuickSlotsChanged();

	return Response;
}

void UPNQuickSlotComponent::RequestAssignFromInventoryPosition(FPNInventoryGridPosition InventoryPosition, int32 SlotIndex)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	if (OwnerActor->HasAuthority())
	{
		AssignFromInventoryPosition(InventoryPosition, SlotIndex);
		return;
	}

	Server_AssignFromInventoryPosition(InventoryPosition, SlotIndex);
}

void UPNQuickSlotComponent::RequestClearQuickSlotToInventory(int32 SlotIndex)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	if (OwnerActor->HasAuthority())
	{
		ClearQuickSlotToInventory(SlotIndex);
		return;
	}

	Server_ClearQuickSlotToInventory(SlotIndex);
}

void UPNQuickSlotComponent::RequestSelectQuickSlot(int32 SlotIndex)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	if (OwnerActor->HasAuthority())
	{
		SelectQuickSlot(SlotIndex);
		return;
	}

	Server_SelectQuickSlot(SlotIndex);
}

void UPNQuickSlotComponent::RequestActivateQuickSlot(int32 SlotIndex, bool bDoubleClick)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	if (OwnerActor->HasAuthority())
	{
		ActivateQuickSlot(SlotIndex, bDoubleClick);
		return;
	}

	Server_ActivateQuickSlot(SlotIndex, bDoubleClick);
}

void UPNQuickSlotComponent::RequestUseQuickSlot(int32 SlotIndex)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	if (OwnerActor->HasAuthority())
	{
		UseQuickSlot(SlotIndex);
		return;
	}

	Server_UseQuickSlot(SlotIndex);
}

void UPNQuickSlotComponent::RequestUseInventoryConsumable(FPNInventoryGridPosition InventoryPosition)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	if (OwnerActor->HasAuthority())
	{
		UseInventoryConsumable(InventoryPosition);
		return;
	}

	Server_UseInventoryConsumable(InventoryPosition);
}

void UPNQuickSlotComponent::Server_AssignFromInventoryPosition_Implementation(FPNInventoryGridPosition InventoryPosition, int32 SlotIndex)
{
	AssignFromInventoryPosition(InventoryPosition, SlotIndex);
}

void UPNQuickSlotComponent::Server_ClearQuickSlotToInventory_Implementation(int32 SlotIndex)
{
	ClearQuickSlotToInventory(SlotIndex);
}

void UPNQuickSlotComponent::Server_SelectQuickSlot_Implementation(int32 SlotIndex)
{
	SelectQuickSlot(SlotIndex);
}

void UPNQuickSlotComponent::Server_ActivateQuickSlot_Implementation(int32 SlotIndex, bool bDoubleClick)
{
	ActivateQuickSlot(SlotIndex, bDoubleClick);
}

void UPNQuickSlotComponent::Server_UseQuickSlot_Implementation(int32 SlotIndex)
{
	UseQuickSlot(SlotIndex);
}

void UPNQuickSlotComponent::Server_UseInventoryConsumable_Implementation(FPNInventoryGridPosition InventoryPosition)
{
	UseInventoryConsumable(InventoryPosition);
}

void UPNQuickSlotComponent::Client_PlayFirstPersonUseAnimation_Implementation(EPNAnimType UseAnimType, UPNItemDataAsset* ItemData)
{
	OnFirstPersonUseAnimationRequested.Broadcast(UseAnimType, ItemData);

	if (bDebugQuickSlots && GEngine && ItemData)
	{
		const FString ItemName = ItemData->GetItemName().IsEmpty()
			? ItemData->GetItemId().ToString()
			: ItemData->GetItemName().ToString();

		GEngine->AddOnScreenDebugMessage(
			GetOwner() ? static_cast<int32>(GetOwner()->GetUniqueID()) + 31000 : INDEX_NONE,
			3.0f,
			FColor::Yellow,
			FString::Printf(TEXT("[CLIENT] 1P Use Anim: %s | %s"), *UEnum::GetValueAsString(UseAnimType), *ItemName)
		);
	}
}

bool UPNQuickSlotComponent::IsValidQuickSlotIndex(int32 SlotIndex) const
{
	return SlotIndex >= 0 && SlotIndex < FMath::Clamp(QuickSlotCount, 1, 12);
}

bool UPNQuickSlotComponent::IsQuickSlotOccupied(int32 SlotIndex) const
{
	const int32 SlotArrayIndex = FindQuickSlotArrayIndex(SlotIndex);

	if (!QuickSlots.IsValidIndex(SlotArrayIndex))
	{
		return false;
	}

	return QuickSlots[SlotArrayIndex].IsOccupied();
}

bool UPNQuickSlotComponent::IsQuickSlotSelected(int32 SlotIndex) const
{
	return SelectedQuickSlotIndex == SlotIndex;
}

bool UPNQuickSlotComponent::CanPlaceItemDataIntoQuickSlot(UPNItemDataAsset* ItemData) const
{
	if (!ItemData)
	{
		return false;
	}

	return ItemData->ItemType == EPNItemType::IT_Consumables
		|| ItemData->ItemType == EPNItemType::IT_Items
		|| ItemData->ItemType == EPNItemType::IT_Builds;
}

bool UPNQuickSlotComponent::IsConsumableItemData(UPNItemDataAsset* ItemData) const
{
	return ItemData && ItemData->ItemType == EPNItemType::IT_Consumables;
}

FPNQuickSlotEntry UPNQuickSlotComponent::GetQuickSlotEntry(int32 SlotIndex) const
{
	const int32 SlotArrayIndex = FindQuickSlotArrayIndex(SlotIndex);

	if (!QuickSlots.IsValidIndex(SlotArrayIndex))
	{
		FPNQuickSlotEntry EmptyEntry;
		EmptyEntry.SlotIndex = SlotIndex;
		return EmptyEntry;
	}

	return QuickSlots[SlotArrayIndex];
}

FPNQuickSlotEntry UPNQuickSlotComponent::GetSelectedQuickSlotEntry() const
{
	return GetQuickSlotEntry(SelectedQuickSlotIndex);
}

UPNItemDataAsset* UPNQuickSlotComponent::GetQuickSlotItemData(int32 SlotIndex) const
{
	const FPNQuickSlotEntry SlotEntry = GetQuickSlotEntry(SlotIndex);
	return SlotEntry.InstanceData.ItemData;
}

UPNItemDataAsset* UPNQuickSlotComponent::GetSelectedQuickSlotItemData() const
{
	return GetQuickSlotItemData(SelectedQuickSlotIndex);
}

int32 UPNQuickSlotComponent::GetSelectedQuickSlotIndex() const
{
	return SelectedQuickSlotIndex;
}

const TArray<FPNQuickSlotEntry>& UPNQuickSlotComponent::GetQuickSlots() const
{
	return QuickSlots;
}

FString UPNQuickSlotComponent::GetQuickSlotsDebugString() const
{
	const AActor* OwnerActor = GetOwner();

	const FString NetSide = OwnerActor && OwnerActor->HasAuthority()
		? TEXT("SERVER")
		: TEXT("CLIENT");

	const FString OwnerName = OwnerActor
		? OwnerActor->GetName()
		: TEXT("NoOwner");

	FString Result = FString::Printf(
		TEXT("[%s] QuickSlots: %s | Selected: %d"),
		*NetSide,
		*OwnerName,
		SelectedQuickSlotIndex
	);

	for (const FPNQuickSlotEntry& SlotEntry : QuickSlots)
	{
		const UPNItemDataAsset* ItemData = SlotEntry.InstanceData.ItemData;

		const FString ItemName = ItemData
			? (ItemData->GetItemName().IsEmpty() ? ItemData->GetItemId().ToString() : ItemData->GetItemName().ToString())
			: TEXT("Empty");

		Result += FString::Printf(
			TEXT("\n[%d] %s x%d"),
			SlotEntry.SlotIndex,
			*ItemName,
			SlotEntry.InstanceData.Quantity
		);
	}

	return Result;
}

void UPNQuickSlotComponent::PrintQuickSlotsDebug() const
{
	const FString DebugText = GetQuickSlotsDebugString();

	UE_LOG(LogTemp, Warning, TEXT("%s"), *DebugText);

	if (GEngine)
	{
		const int32 DebugKey = GetOwner()
			? static_cast<int32>(GetOwner()->GetUniqueID()) + 30000
			: INDEX_NONE;

		GEngine->AddOnScreenDebugMessage(
			DebugKey,
			5.0f,
			FColor::Yellow,
			DebugText
		);
	}
}

void UPNQuickSlotComponent::OnRep_QuickSlots()
{
	OnQuickSlotsChanged.Broadcast();

	if (bDebugQuickSlots)
	{
		PrintQuickSlotsDebug();
	}

	BroadcastSelectedQuickSlot();
}

void UPNQuickSlotComponent::OnRep_SelectedQuickSlotIndex()
{
	BroadcastSelectedQuickSlot();

	if (bDebugQuickSlots)
	{
		PrintQuickSlotsDebug();
	}
}

bool UPNQuickSlotComponent::HasQuickSlotAuthority() const
{
	const AActor* OwnerActor = GetOwner();
	return !OwnerActor || OwnerActor->HasAuthority();
}

UPNInventoryComponent* UPNQuickSlotComponent::GetOwnerInventoryComponent() const
{
	const APNBaseCharacter* OwnerCharacter = Cast<APNBaseCharacter>(GetOwner());
	return OwnerCharacter ? OwnerCharacter->GetInventoryComponent() : nullptr;
}

UPNCharacterStatsComponent* UPNQuickSlotComponent::GetOwnerCharacterStatsComponent() const
{
	const APNBaseCharacter* OwnerCharacter = Cast<APNBaseCharacter>(GetOwner());
	return OwnerCharacter ? OwnerCharacter->GetCharacterStatsComponent() : nullptr;
}

void UPNQuickSlotComponent::BuildDefaultQuickSlots()
{
	QuickSlotCount = FMath::Clamp(QuickSlotCount, 1, 12);

	for (int32 Index = 0; Index < QuickSlotCount; ++Index)
	{
		FindOrCreateQuickSlotArrayIndex(Index);
	}
}

int32 UPNQuickSlotComponent::FindQuickSlotArrayIndex(int32 SlotIndex) const
{
	for (int32 Index = 0; Index < QuickSlots.Num(); ++Index)
	{
		if (QuickSlots[Index].SlotIndex == SlotIndex)
		{
			return Index;
		}
	}

	return INDEX_NONE;
}

int32 UPNQuickSlotComponent::FindOrCreateQuickSlotArrayIndex(int32 SlotIndex)
{
	if (!IsValidQuickSlotIndex(SlotIndex))
	{
		return INDEX_NONE;
	}

	const int32 ExistingIndex = FindQuickSlotArrayIndex(SlotIndex);
	if (ExistingIndex != INDEX_NONE)
	{
		return ExistingIndex;
	}

	FPNQuickSlotEntry NewEntry;
	NewEntry.SlotIndex = SlotIndex;

	return QuickSlots.Add(NewEntry);
}

void UPNQuickSlotComponent::SetQuickSlotData(int32 SlotIndex, const FPNRepItemInstanceData& InstanceData)
{
	const int32 SlotArrayIndex = FindOrCreateQuickSlotArrayIndex(SlotIndex);

	if (!QuickSlots.IsValidIndex(SlotArrayIndex))
	{
		return;
	}

	QuickSlots[SlotArrayIndex].InstanceData = InstanceData;
}

void UPNQuickSlotComponent::ClearQuickSlotData(int32 SlotIndex)
{
	const int32 SlotArrayIndex = FindQuickSlotArrayIndex(SlotIndex);

	if (!QuickSlots.IsValidIndex(SlotArrayIndex))
	{
		return;
	}

	QuickSlots[SlotArrayIndex].InstanceData = FPNRepItemInstanceData();
}

bool UPNQuickSlotComponent::ApplyConsumableEffects(const FPNRepItemInstanceData& InstanceData)
{
	UPNItemDataAsset* ItemData = InstanceData.ItemData;
	if (!IsConsumableItemData(ItemData))
	{
		return false;
	}

	UPNCharacterStatsComponent* StatsComponent = GetOwnerCharacterStatsComponent();
	if (!StatsComponent)
	{
		return false;
	}

	const FPNConsumableStats& ConsumableStats = ItemData->ConsumableStats;

	const float HealthDelta = RollStatRange(ConsumableStats.HealthMin, ConsumableStats.HealthMax);
	const float FoodDelta = RollStatRange(ConsumableStats.FoodMin, ConsumableStats.FoodMax);
	const float WaterDelta = RollStatRange(ConsumableStats.WaterMin, ConsumableStats.WaterMax);
	const float StaminaDelta = RollStatRange(ConsumableStats.StaminaMin, ConsumableStats.StaminaMax);

	if (HealthDelta > 0.0f)
	{
		StatsComponent->Heal(HealthDelta);
	}
	else if (HealthDelta < 0.0f)
	{
		StatsComponent->ApplyDamage(-HealthDelta, nullptr, GetOwner());
	}

	if (!FMath::IsNearlyZero(FoodDelta))
	{
		StatsComponent->AddHunger(FoodDelta);
	}

	if (!FMath::IsNearlyZero(WaterDelta))
	{
		StatsComponent->AddThirst(WaterDelta);
	}

	if (StaminaDelta > 0.0f)
	{
		StatsComponent->RestoreStamina(StaminaDelta);
	}
	else if (StaminaDelta < 0.0f)
	{
		StatsComponent->ConsumeStamina(-StaminaDelta);
	}

	if (!FMath::IsNearlyZero(ConsumableStats.Toxicity))
	{
		StatsComponent->AddToxicity(ConsumableStats.Toxicity);
	}

	if (ConsumableStats.CureToxicity > 0.0f)
	{
		StatsComponent->AddToxicity(-ConsumableStats.CureToxicity);
	}

	if (ConsumableStats.CurePsi > 0.0f)
	{
		StatsComponent->AddPsy(-ConsumableStats.CurePsi);
	}

	if (ConsumableStats.CureRadiation > 0.0f)
	{
		StatsComponent->AddRadiation(-ConsumableStats.CureRadiation);
	}

	if (ConsumableStats.CureBleeding > 0.0f)
	{
		StatsComponent->AddBleeding(-ConsumableStats.CureBleeding);
	}

	if (ItemData->Expiration.bUsesExpiration
		&& InstanceData.RemainingShelfLifeSeconds <= 0.0f
		&& ItemData->Expiration.ToxicityWhenExpired > 0.0f)
	{
		StatsComponent->AddToxicity(ItemData->Expiration.ToxicityWhenExpired);
	}

	Client_PlayFirstPersonUseAnimation(ConsumableStats.UseAnimType, ItemData);

	if (bDebugQuickSlots)
	{
		UE_LOG(LogTemp, Warning, TEXT("[QuickSlot] Used consumable: %s"), *ItemData->GetItemId().ToString());
	}

	return true;
}

float UPNQuickSlotComponent::RollStatRange(float MinValue, float MaxValue) const
{
	if (FMath::IsNearlyEqual(MinValue, MaxValue))
	{
		return MinValue;
	}

	const float ActualMin = FMath::Min(MinValue, MaxValue);
	const float ActualMax = FMath::Max(MinValue, MaxValue);

	return FMath::FRandRange(ActualMin, ActualMax);
}

EPNQuickSlotOperationResult UPNQuickSlotComponent::ConvertInventoryAddFail(EPNInventoryOperationResult InventoryResult) const
{
	switch (InventoryResult)
	{
	case EPNInventoryOperationResult::InvalidInventory:
		return EPNQuickSlotOperationResult::InvalidInventory;

	case EPNInventoryOperationResult::InvalidItem:
		return EPNQuickSlotOperationResult::InvalidItem;

	case EPNInventoryOperationResult::NoSpace:
	case EPNInventoryOperationResult::OverWeight:
	default:
		return EPNQuickSlotOperationResult::InventoryAddFailed;
	}
}

EPNQuickSlotOperationResult UPNQuickSlotComponent::ConvertInventoryRemoveFail(EPNInventoryOperationResult InventoryResult) const
{
	switch (InventoryResult)
	{
	case EPNInventoryOperationResult::InvalidInventory:
		return EPNQuickSlotOperationResult::InvalidInventory;

	case EPNInventoryOperationResult::InvalidItem:
		return EPNQuickSlotOperationResult::InvalidItem;

	case EPNInventoryOperationResult::SlotEmpty:
		return EPNQuickSlotOperationResult::SlotEmpty;

	default:
		return EPNQuickSlotOperationResult::InventoryRemoveFailed;
	}
}

void UPNQuickSlotComponent::BroadcastQuickSlotsChanged()
{
	OnQuickSlotsChanged.Broadcast();

	if (bDebugQuickSlots)
	{
		PrintQuickSlotsDebug();
	}
}

void UPNQuickSlotComponent::BroadcastSelectedQuickSlot()
{
	UPNItemDataAsset* SelectedItemData = GetSelectedQuickSlotItemData();

	OnQuickSlotSelected.Broadcast(SelectedQuickSlotIndex, SelectedItemData);
}