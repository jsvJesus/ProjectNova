#include "Items/PNQuickSlotComponent.h"

#include "Characters/PNBaseCharacter.h"
#include "Engine/Engine.h"
#include "Inventory/PNInventoryComponent.h"
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

	if (UPNInventoryComponent* InventoryComponent = GetOwnerInventoryComponent())
	{
		InventoryComponent->OnInventoryChanged.AddDynamic(this, &UPNQuickSlotComponent::HandleInventoryChanged);
	}

	BroadcastQuickSlotsChanged();
	BroadcastSelectedQuickSlot();
}

void UPNQuickSlotComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UPNInventoryComponent* InventoryComponent = GetOwnerInventoryComponent())
	{
		InventoryComponent->OnInventoryChanged.RemoveDynamic(this, &UPNQuickSlotComponent::HandleInventoryChanged);
	}

	Super::EndPlay(EndPlayReason);
}

void UPNQuickSlotComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPNQuickSlotComponent, SelectedQuickSlotIndex);
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

	UPNInventoryComponent* InventoryComponent = GetOwnerInventoryComponent();
	if (!InventoryComponent)
	{
		Response.Result = EPNQuickSlotOperationResult::InvalidInventory;
		return Response;
	}

	const UPNItemInstance* SourceItem = InventoryComponent->GetItemAtPosition(InventoryPosition);
	UPNItemDataAsset* SourceItemData = SourceItem ? SourceItem->GetItemData() : nullptr;

	FPNInventoryAddItemResult MoveResult = InventoryComponent->MoveItemFromInventoryToQuickSlot(InventoryPosition, SlotIndex);

	Response.bSuccess = MoveResult.bSuccess;
	Response.SlotIndex = SlotIndex;
	Response.Quantity = MoveResult.AddedQuantity;
	Response.ItemData = SourceItemData ? SourceItemData : InventoryComponent->GetQuickSlotItemData(SlotIndex);
	Response.Result = MoveResult.bSuccess ? EPNQuickSlotOperationResult::Success : ConvertInventoryAddFail(MoveResult.Result);

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

	UPNInventoryComponent* InventoryComponent = GetOwnerInventoryComponent();
	if (!InventoryComponent)
	{
		Response.Result = EPNQuickSlotOperationResult::InvalidInventory;
		return Response;
	}

	const FPNInventoryQuickSlotEntry OldEntry = InventoryComponent->GetQuickSlotEntry(SlotIndex);
	if (OldEntry.IsEmpty())
	{
		Response.Result = EPNQuickSlotOperationResult::SlotEmpty;
		return Response;
	}

	FPNInventoryAddItemResult MoveResult = InventoryComponent->MoveQuickSlotToInventory(SlotIndex);

	Response.bSuccess = MoveResult.bSuccess;
	Response.Result = MoveResult.bSuccess ? EPNQuickSlotOperationResult::Success : ConvertInventoryAddFail(MoveResult.Result);
	Response.ItemData = OldEntry.InstanceData.ItemData;
	Response.Quantity = MoveResult.AddedQuantity;

	if (MoveResult.bSuccess && SelectedQuickSlotIndex == SlotIndex && !InventoryComponent->IsQuickSlotOccupied(SlotIndex))
	{
		SelectedQuickSlotIndex = INDEX_NONE;
		BroadcastSelectedQuickSlot();
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

	UPNInventoryComponent* InventoryComponent = GetOwnerInventoryComponent();
	if (!InventoryComponent)
	{
		Response.Result = EPNQuickSlotOperationResult::InvalidInventory;
		return Response;
	}

	if (!InventoryComponent->IsValidQuickSlotIndex(SlotIndex))
	{
		Response.Result = EPNQuickSlotOperationResult::InvalidSlotIndex;
		return Response;
	}

	SelectedQuickSlotIndex = SlotIndex;

	const FPNInventoryQuickSlotEntry SlotEntry = InventoryComponent->GetQuickSlotEntry(SlotIndex);

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

	UPNInventoryComponent* InventoryComponent = GetOwnerInventoryComponent();
	if (!InventoryComponent)
	{
		Response.Result = EPNQuickSlotOperationResult::InvalidInventory;
		return Response;
	}

	if (!InventoryComponent->IsValidQuickSlotIndex(SlotIndex))
	{
		Response.Result = EPNQuickSlotOperationResult::InvalidSlotIndex;
		return Response;
	}

	const FPNInventoryQuickSlotEntry SlotEntry = InventoryComponent->GetQuickSlotEntry(SlotIndex);

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

	UPNInventoryComponent* InventoryComponent = GetOwnerInventoryComponent();
	if (!InventoryComponent)
	{
		Response.Result = EPNQuickSlotOperationResult::InvalidInventory;
		return Response;
	}

	const FPNInventoryQuickSlotEntry SlotEntry = InventoryComponent->GetQuickSlotEntry(SlotIndex);
	if (SlotEntry.IsEmpty())
	{
		Response.Result = EPNQuickSlotOperationResult::SlotEmpty;
		return Response;
	}

	UPNItemDataAsset* ItemData = SlotEntry.InstanceData.ItemData;
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

	FPNInventoryRemoveItemResult RemoveResult = InventoryComponent->RemoveItemFromQuickSlot(SlotIndex, 1);
	if (!RemoveResult.bSuccess || !RemoveResult.RemovedItemInstance || !RemoveResult.RemovedItemInstance->IsValidItem())
	{
		Response.Result = ConvertInventoryRemoveFail(RemoveResult.Result);
		return Response;
	}

	const FPNRepItemInstanceData UsedData = RemoveResult.RemovedItemInstance->ToRepData();

	if (!ApplyConsumableEffects(UsedData))
	{
		Response.Result = EPNQuickSlotOperationResult::InvalidItem;
		return Response;
	}

	Response.ItemData = ItemData;
	Response.Quantity = FMath::Max(0, SlotEntry.InstanceData.Quantity - 1);
	Response.bSuccess = true;
	Response.Result = EPNQuickSlotOperationResult::Success;

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

	const FPNRepItemInstanceData UsedData = RemoveResult.RemovedItemInstance->ToRepData();

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
	const UPNInventoryComponent* InventoryComponent = GetOwnerInventoryComponent();
	return InventoryComponent && InventoryComponent->IsValidQuickSlotIndex(SlotIndex);
}

bool UPNQuickSlotComponent::IsQuickSlotOccupied(int32 SlotIndex) const
{
	const UPNInventoryComponent* InventoryComponent = GetOwnerInventoryComponent();
	return InventoryComponent && InventoryComponent->IsQuickSlotOccupied(SlotIndex);
}

bool UPNQuickSlotComponent::IsQuickSlotSelected(int32 SlotIndex) const
{
	return SelectedQuickSlotIndex == SlotIndex;
}

bool UPNQuickSlotComponent::IsConsumableItemData(UPNItemDataAsset* ItemData) const
{
	return ItemData && ItemData->ItemType == EPNItemType::IT_Consumables;
}

FPNInventoryQuickSlotEntry UPNQuickSlotComponent::GetQuickSlotEntry(int32 SlotIndex) const
{
	const UPNInventoryComponent* InventoryComponent = GetOwnerInventoryComponent();

	if (!InventoryComponent)
	{
		FPNInventoryQuickSlotEntry EmptyEntry;
		EmptyEntry.SlotIndex = SlotIndex;
		return EmptyEntry;
	}

	return InventoryComponent->GetQuickSlotEntry(SlotIndex);
}

FPNInventoryQuickSlotEntry UPNQuickSlotComponent::GetSelectedQuickSlotEntry() const
{
	return GetQuickSlotEntry(SelectedQuickSlotIndex);
}

UPNItemDataAsset* UPNQuickSlotComponent::GetQuickSlotItemData(int32 SlotIndex) const
{
	const UPNInventoryComponent* InventoryComponent = GetOwnerInventoryComponent();
	return InventoryComponent ? InventoryComponent->GetQuickSlotItemData(SlotIndex) : nullptr;
}

UPNItemDataAsset* UPNQuickSlotComponent::GetSelectedQuickSlotItemData() const
{
	return GetQuickSlotItemData(SelectedQuickSlotIndex);
}

int32 UPNQuickSlotComponent::GetSelectedQuickSlotIndex() const
{
	return SelectedQuickSlotIndex;
}

const TArray<FPNInventoryQuickSlotEntry>& UPNQuickSlotComponent::GetQuickSlots() const
{
	static const TArray<FPNInventoryQuickSlotEntry> EmptyQuickSlots;

	const UPNInventoryComponent* InventoryComponent = GetOwnerInventoryComponent();
	return InventoryComponent ? InventoryComponent->GetQuickSlots() : EmptyQuickSlots;
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

	for (const FPNInventoryQuickSlotEntry& SlotEntry : GetQuickSlots())
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

void UPNQuickSlotComponent::OnRep_SelectedQuickSlotIndex()
{
	BroadcastSelectedQuickSlot();

	if (bDebugQuickSlots)
	{
		PrintQuickSlotsDebug();
	}
}

void UPNQuickSlotComponent::HandleInventoryChanged()
{
	BroadcastQuickSlotsChanged();
	BroadcastSelectedQuickSlot();
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

	case EPNInventoryOperationResult::SlotEmpty:
		return EPNQuickSlotOperationResult::SlotEmpty;

	case EPNInventoryOperationResult::SlotOccupied:
		return EPNQuickSlotOperationResult::SlotOccupied;

	case EPNInventoryOperationResult::ItemTypeNotAllowed:
		return EPNQuickSlotOperationResult::ItemTypeNotAllowed;

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