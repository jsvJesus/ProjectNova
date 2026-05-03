#include "Items/PNInventoryComponent.h"
#include "Items/PNItemDataAsset.h"
#include "Items/PNItemInstance.h"

UPNInventoryComponent::UPNInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	Settings.InventoryType = EPNInventoryType::Inventory;
	Settings.GridSize.Columns = 5;
	Settings.GridSize.Rows = 5;
	Settings.bUseWeightLimit = true;
	Settings.MaxWeight = 30.0f;
	Settings.bAllowItemRotation = true;
	Settings.bAllowStacking = true;
	Settings.bCanReceiveItems = true;
	Settings.bCanRemoveItems = true;
	Settings.bCanDropItems = true;
	Settings.bCanTradeItems = false;
}

void UPNInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	RebuildSlots();
}

void UPNInventoryComponent::InitializeInventory(const FPNInventorySettings& InSettings)
{
	Settings = InSettings;

	Settings.GridSize.Columns = FMath::Max(1, Settings.GridSize.Columns);
	Settings.GridSize.Rows = FMath::Max(1, Settings.GridSize.Rows);
	Settings.MaxWeight = FMath::Max(0.0f, Settings.MaxWeight);

	Items.Reset();
	RebuildSlots();
	BroadcastInventoryChanged();
}

void UPNInventoryComponent::RebuildSlots()
{
	Slots.Reset();

	const int32 Columns = GetColumns();
	const int32 Rows = GetRows();
	const int32 TotalSlots = Columns * Rows;

	Slots.SetNum(TotalSlots);

	for (int32 Index = 0; Index < TotalSlots; ++Index)
	{
		FPNInventorySlot& Slot = Slots[Index];

		Slot.SlotIndex = Index;
		Slot.Position = IndexToPosition(Index);
		Slot.State = EPNInventorySlotState::Empty;
		Slot.ItemInstance = nullptr;
		Slot.bRootSlot = false;
	}

	for (const FPNInventoryItemEntry& Entry : Items)
	{
		MarkItemArea(Entry);
	}
}

UPNItemInstance* UPNInventoryComponent::CreateItemInstance(UPNItemDataAsset* ItemData, int32 Quantity)
{
	if (!ItemData)
	{
		return nullptr;
	}

	UPNItemInstance* NewInstance = NewObject<UPNItemInstance>(this);
	if (!NewInstance)
	{
		return nullptr;
	}

	NewInstance->Initialize(ItemData, Quantity);
	return NewInstance;
}

FPNInventoryAddItemResult UPNInventoryComponent::AddItem(UPNItemInstance* ItemInstance, bool bAllowStack, bool bAutoRotate)
{
	FPNInventoryAddItemResult Result;
	Result.Result = EPNInventoryOperationResult::UnknownError;

	if (!Settings.bCanReceiveItems)
	{
		Result.Result = EPNInventoryOperationResult::InventoryTypeNotAllowed;
		return Result;
	}

	if (!ItemInstance || !ItemInstance->IsValidItem())
	{
		Result.Result = EPNInventoryOperationResult::InvalidItem;
		return Result;
	}

	int32 RemainingQuantity = FMath::Max(1, ItemInstance->Quantity);
	Result.RemainingQuantity = RemainingQuantity;

	if (!CanFitWeight(ItemInstance, RemainingQuantity))
	{
		Result.Result = EPNInventoryOperationResult::OverWeight;
		return Result;
	}

	if (bAllowStack && Settings.bAllowStacking)
	{
		TryStackItem(ItemInstance, RemainingQuantity, Result);

		if (RemainingQuantity <= 0)
		{
			Result.bSuccess = true;
			Result.Result = EPNInventoryOperationResult::Success;
			Result.RemainingQuantity = 0;
			BroadcastInventoryChanged();
			return Result;
		}
	}

	while (RemainingQuantity > 0)
	{
		FPNInventoryGridPosition FreePosition;
		bool bRotated = false;

		if (!FindFreePositionForItem(ItemInstance, bAutoRotate, FreePosition, bRotated))
		{
			Result.bSuccess = Result.AddedQuantity > 0;
			Result.Result = Result.bSuccess ? EPNInventoryOperationResult::NoSpace : EPNInventoryOperationResult::NoSpace;
			Result.RemainingQuantity = RemainingQuantity;

			if (Result.bSuccess)
			{
				BroadcastInventoryChanged();
			}

			return Result;
		}

		const int32 MaxStack = ItemInstance->GetMaxStack();
		const int32 QuantityToPlace = FMath::Clamp(RemainingQuantity, 1, MaxStack);

		UPNItemInstance* NewStack = DuplicateItemInstance(ItemInstance, QuantityToPlace);
		if (!NewStack)
		{
			Result.Result = EPNInventoryOperationResult::InvalidItem;
			Result.RemainingQuantity = RemainingQuantity;
			return Result;
		}

		AddItemEntry(NewStack, FreePosition, bRotated);

		Result.Position = FreePosition;
		Result.TargetItemInstance = NewStack;
		Result.AddedQuantity += QuantityToPlace;

		RemainingQuantity -= QuantityToPlace;
	}

	Result.bSuccess = true;
	Result.Result = EPNInventoryOperationResult::Success;
	Result.RemainingQuantity = 0;

	BroadcastInventoryChanged();
	return Result;
}

FPNInventoryAddItemResult UPNInventoryComponent::AddItemAtPosition(UPNItemInstance* ItemInstance, FPNInventoryGridPosition Position, bool bRotated, bool bAllowStack)
{
	FPNInventoryAddItemResult Result;
	Result.Position = Position;
	Result.Result = EPNInventoryOperationResult::UnknownError;

	if (!Settings.bCanReceiveItems)
	{
		Result.Result = EPNInventoryOperationResult::InventoryTypeNotAllowed;
		return Result;
	}

	if (!ItemInstance || !ItemInstance->IsValidItem())
	{
		Result.Result = EPNInventoryOperationResult::InvalidItem;
		return Result;
	}

	const int32 RequestedQuantity = FMath::Max(1, ItemInstance->Quantity);
	Result.RemainingQuantity = RequestedQuantity;

	if (!CanFitWeight(ItemInstance, RequestedQuantity))
	{
		Result.Result = EPNInventoryOperationResult::OverWeight;
		return Result;
	}

	if (!IsValidGridPosition(Position))
	{
		Result.Result = EPNInventoryOperationResult::OutOfBounds;
		return Result;
	}

	UPNItemInstance* ExistingItem = GetItemAtPosition(Position);

	if (ExistingItem)
	{
		if (!bAllowStack || !Settings.bAllowStacking)
		{
			Result.Result = EPNInventoryOperationResult::SlotOccupied;
			return Result;
		}

		if (!ExistingItem->CanStackWith(ItemInstance))
		{
			Result.Result = EPNInventoryOperationResult::StackMismatch;
			return Result;
		}

		const int32 FreeStackSpace = ExistingItem->GetMaxStack() - ExistingItem->Quantity;
		if (FreeStackSpace <= 0)
		{
			Result.Result = EPNInventoryOperationResult::StackFull;
			return Result;
		}

		const int32 QuantityToAdd = FMath::Min(FreeStackSpace, RequestedQuantity);

		if (!ExistingItem->AddQuantity(QuantityToAdd))
		{
			Result.Result = EPNInventoryOperationResult::StackFull;
			return Result;
		}

		Result.bSuccess = true;
		Result.Result = EPNInventoryOperationResult::Success;
		Result.AddedQuantity = QuantityToAdd;
		Result.RemainingQuantity = RequestedQuantity - QuantityToAdd;
		Result.TargetItemInstance = ExistingItem;

		BroadcastInventoryChanged();
		return Result;
	}

	FPNInventoryItemSize ItemSize = BuildItemSize(ItemInstance, bRotated);

	if (!IsAreaInside(Position, ItemSize))
	{
		Result.Result = EPNInventoryOperationResult::OutOfBounds;
		return Result;
	}

	if (!IsAreaFree(Position, ItemSize))
	{
		Result.Result = EPNInventoryOperationResult::SlotOccupied;
		return Result;
	}

	const int32 QuantityToPlace = FMath::Clamp(RequestedQuantity, 1, ItemInstance->GetMaxStack());

	UPNItemInstance* NewStack = DuplicateItemInstance(ItemInstance, QuantityToPlace);
	if (!NewStack)
	{
		Result.Result = EPNInventoryOperationResult::InvalidItem;
		return Result;
	}

	AddItemEntry(NewStack, Position, bRotated);

	Result.bSuccess = true;
	Result.Result = EPNInventoryOperationResult::Success;
	Result.AddedQuantity = QuantityToPlace;
	Result.RemainingQuantity = RequestedQuantity - QuantityToPlace;
	Result.TargetItemInstance = NewStack;

	BroadcastInventoryChanged();
	return Result;
}

FPNInventoryMoveItemResult UPNInventoryComponent::MoveItem(UPNItemInstance* ItemInstance, FPNInventoryGridPosition NewPosition, bool bRotated)
{
	FPNInventoryMoveItemResult Result;
	Result.NewPosition = NewPosition;
	Result.Result = EPNInventoryOperationResult::UnknownError;

	if (!ItemInstance || !ItemInstance->IsValidItem())
	{
		Result.Result = EPNInventoryOperationResult::InvalidItem;
		return Result;
	}

	const int32 EntryIndex = FindEntryIndexByItem(ItemInstance);
	if (EntryIndex == INDEX_NONE)
	{
		Result.Result = EPNInventoryOperationResult::InvalidItem;
		return Result;
	}

	FPNInventoryItemEntry& Entry = Items[EntryIndex];

	Result.OldPosition = Entry.Position;

	if (Entry.Position == NewPosition && Entry.bRotated == bRotated)
	{
		Result.Result = EPNInventoryOperationResult::SameSlot;
		return Result;
	}

	FPNInventoryItemSize NewSize = BuildItemSize(ItemInstance, bRotated);

	if (!IsAreaInside(NewPosition, NewSize))
	{
		Result.Result = EPNInventoryOperationResult::OutOfBounds;
		return Result;
	}

	if (!IsAreaFreeInternal(NewPosition, NewSize, ItemInstance))
	{
		Result.Result = EPNInventoryOperationResult::SlotOccupied;
		return Result;
	}

	Entry.Position = NewPosition;
	Entry.bRotated = bRotated;
	Entry.Size = NewSize;

	RebuildSlots();

	Result.bSuccess = true;
	Result.Result = EPNInventoryOperationResult::Success;

	BroadcastInventoryChanged();
	return Result;
}

FPNInventoryMoveItemResult UPNInventoryComponent::MoveItemFromPosition(FPNInventoryGridPosition OldPosition, FPNInventoryGridPosition NewPosition, bool bRotated)
{
	UPNItemInstance* ItemInstance = GetItemAtPosition(OldPosition);

	if (!ItemInstance)
	{
		FPNInventoryMoveItemResult Result;
		Result.OldPosition = OldPosition;
		Result.NewPosition = NewPosition;
		Result.Result = EPNInventoryOperationResult::SlotEmpty;
		return Result;
	}

	return MoveItem(ItemInstance, NewPosition, bRotated);
}

FPNInventoryRemoveItemResult UPNInventoryComponent::RemoveItemInstance(UPNItemInstance* ItemInstance, int32 QuantityToRemove)
{
	FPNInventoryRemoveItemResult Result;
	Result.Result = EPNInventoryOperationResult::UnknownError;

	if (!Settings.bCanRemoveItems)
	{
		Result.Result = EPNInventoryOperationResult::InventoryTypeNotAllowed;
		return Result;
	}

	if (!ItemInstance || !ItemInstance->IsValidItem())
	{
		Result.Result = EPNInventoryOperationResult::InvalidItem;
		return Result;
	}

	if (QuantityToRemove <= 0)
	{
		Result.Result = EPNInventoryOperationResult::InvalidQuantity;
		return Result;
	}

	const int32 EntryIndex = FindEntryIndexByItem(ItemInstance);
	if (EntryIndex == INDEX_NONE)
	{
		Result.Result = EPNInventoryOperationResult::InvalidItem;
		return Result;
	}

	const int32 RemovedQuantity = FMath::Min(ItemInstance->Quantity, QuantityToRemove);

	if (RemovedQuantity <= 0)
	{
		Result.Result = EPNInventoryOperationResult::NotEnoughQuantity;
		return Result;
	}

	Result.RemovedQuantity = RemovedQuantity;
	Result.RemovedItemInstance = DuplicateItemInstance(ItemInstance, RemovedQuantity);

	if (RemovedQuantity >= ItemInstance->Quantity)
	{
		RemoveItemEntryByIndex(EntryIndex);
	}
	else
	{
		ItemInstance->RemoveQuantity(RemovedQuantity);
	}

	RebuildSlots();

	Result.bSuccess = true;
	Result.Result = EPNInventoryOperationResult::Success;

	BroadcastInventoryChanged();
	return Result;
}

FPNInventoryRemoveItemResult UPNInventoryComponent::RemoveItemAtPosition(FPNInventoryGridPosition Position, int32 QuantityToRemove)
{
	UPNItemInstance* ItemInstance = GetItemAtPosition(Position);

	if (!ItemInstance)
	{
		FPNInventoryRemoveItemResult Result;
		Result.Result = EPNInventoryOperationResult::SlotEmpty;
		return Result;
	}

	return RemoveItemInstance(ItemInstance, QuantityToRemove);
}

const TArray<FPNInventorySlot>& UPNInventoryComponent::GetSlots() const
{
	return Slots;
}

const TArray<FPNInventoryItemEntry>& UPNInventoryComponent::GetItems() const
{
	return Items;
}

int32 UPNInventoryComponent::GetColumns() const
{
	return FMath::Max(1, Settings.GridSize.Columns);
}

int32 UPNInventoryComponent::GetRows() const
{
	return FMath::Max(1, Settings.GridSize.Rows);
}

int32 UPNInventoryComponent::GetSlotCount() const
{
	return GetColumns() * GetRows();
}

float UPNInventoryComponent::GetCurrentWeight() const
{
	float TotalWeight = 0.0f;

	for (const FPNInventoryItemEntry& Entry : Items)
	{
		if (Entry.ItemInstance)
		{
			TotalWeight += Entry.ItemInstance->GetTotalWeight();
		}
	}

	return TotalWeight;
}

float UPNInventoryComponent::GetMaxWeight() const
{
	return FMath::Max(0.0f, Settings.MaxWeight);
}

bool UPNInventoryComponent::UsesWeightLimit() const
{
	return Settings.bUseWeightLimit;
}

bool UPNInventoryComponent::CanReceiveItems() const
{
	return Settings.bCanReceiveItems;
}

bool UPNInventoryComponent::CanRemoveItems() const
{
	return Settings.bCanRemoveItems;
}

bool UPNInventoryComponent::IsValidGridPosition(FPNInventoryGridPosition Position) const
{
	return Position.X >= 0
		&& Position.Y >= 0
		&& Position.X < GetColumns()
		&& Position.Y < GetRows();
}

bool UPNInventoryComponent::IsAreaInside(FPNInventoryGridPosition Position, FPNInventoryItemSize Size) const
{
	const int32 Width = FMath::Max(1, Size.GetFinalWidth());
	const int32 Height = FMath::Max(1, Size.GetFinalHeight());

	if (Position.X < 0 || Position.Y < 0)
	{
		return false;
	}

	if (Position.X + Width > GetColumns())
	{
		return false;
	}

	if (Position.Y + Height > GetRows())
	{
		return false;
	}

	return true;
}

bool UPNInventoryComponent::IsAreaFree(FPNInventoryGridPosition Position, FPNInventoryItemSize Size) const
{
	return IsAreaFreeInternal(Position, Size, nullptr);
}

int32 UPNInventoryComponent::PositionToIndex(FPNInventoryGridPosition Position) const
{
	if (!IsValidGridPosition(Position))
	{
		return INDEX_NONE;
	}

	return Position.Y * GetColumns() + Position.X;
}

FPNInventoryGridPosition UPNInventoryComponent::IndexToPosition(int32 SlotIndex) const
{
	FPNInventoryGridPosition Position;

	if (SlotIndex < 0 || SlotIndex >= GetSlotCount())
	{
		Position.X = INDEX_NONE;
		Position.Y = INDEX_NONE;
		return Position;
	}

	Position.X = SlotIndex % GetColumns();
	Position.Y = SlotIndex / GetColumns();

	return Position;
}

UPNItemInstance* UPNInventoryComponent::GetItemAtPosition(FPNInventoryGridPosition Position) const
{
	const int32 SlotIndex = PositionToIndex(Position);

	if (!Slots.IsValidIndex(SlotIndex))
	{
		return nullptr;
	}

	return Slots[SlotIndex].ItemInstance;
}

bool UPNInventoryComponent::FindFreePositionForItem(UPNItemInstance* ItemInstance, bool bAutoRotate, FPNInventoryGridPosition& OutPosition, bool& bOutRotated) const
{
	OutPosition = FPNInventoryGridPosition();
	bOutRotated = false;

	if (!ItemInstance || !ItemInstance->IsValidItem())
	{
		return false;
	}

	FPNInventoryItemSize NormalSize = BuildItemSize(ItemInstance, false);

	for (int32 Y = 0; Y < GetRows(); ++Y)
	{
		for (int32 X = 0; X < GetColumns(); ++X)
		{
			FPNInventoryGridPosition TestPosition;
			TestPosition.X = X;
			TestPosition.Y = Y;

			if (IsAreaFree(TestPosition, NormalSize))
			{
				OutPosition = TestPosition;
				bOutRotated = false;
				return true;
			}
		}
	}

	if (!bAutoRotate || !Settings.bAllowItemRotation)
	{
		return false;
	}

	FPNInventoryItemSize RotatedSize = BuildItemSize(ItemInstance, true);

	if (RotatedSize.GetFinalWidth() == NormalSize.GetFinalWidth() && RotatedSize.GetFinalHeight() == NormalSize.GetFinalHeight())
	{
		return false;
	}

	for (int32 Y = 0; Y < GetRows(); ++Y)
	{
		for (int32 X = 0; X < GetColumns(); ++X)
		{
			FPNInventoryGridPosition TestPosition;
			TestPosition.X = X;
			TestPosition.Y = Y;

			if (IsAreaFree(TestPosition, RotatedSize))
			{
				OutPosition = TestPosition;
				bOutRotated = true;
				return true;
			}
		}
	}

	return false;
}

bool UPNInventoryComponent::CanFitWeight(UPNItemInstance* ItemInstance, int32 QuantityToAdd) const
{
	if (!Settings.bUseWeightLimit)
	{
		return true;
	}

	if (!ItemInstance || !ItemInstance->GetItemData())
	{
		return false;
	}

	const float CurrentWeight = GetCurrentWeight();
	const float AddWeight = ItemInstance->GetItemData()->GetTotalWeightForCount(FMath::Max(1, QuantityToAdd));

	return CurrentWeight + AddWeight <= GetMaxWeight();
}

bool UPNInventoryComponent::TryStackItem(UPNItemInstance* SourceItem, int32& InOutRemainingQuantity, FPNInventoryAddItemResult& InOutResult)
{
	if (!SourceItem || !SourceItem->IsValidItem())
	{
		return false;
	}

	if (!Settings.bAllowStacking)
	{
		return false;
	}

	if (InOutRemainingQuantity <= 0)
	{
		return false;
	}

	bool bStackedAnything = false;

	for (FPNInventoryItemEntry& Entry : Items)
	{
		if (InOutRemainingQuantity <= 0)
		{
			break;
		}

		UPNItemInstance* TargetItem = Entry.ItemInstance;
		if (!TargetItem || !TargetItem->CanStackWith(SourceItem))
		{
			continue;
		}

		const int32 FreeStackSpace = TargetItem->GetMaxStack() - TargetItem->Quantity;
		if (FreeStackSpace <= 0)
		{
			continue;
		}

		const int32 QuantityToAdd = FMath::Min(FreeStackSpace, InOutRemainingQuantity);

		if (TargetItem->AddQuantity(QuantityToAdd))
		{
			InOutRemainingQuantity -= QuantityToAdd;
			InOutResult.AddedQuantity += QuantityToAdd;
			InOutResult.TargetItemInstance = TargetItem;
			InOutResult.Position = Entry.Position;
			bStackedAnything = true;
		}
	}

	return bStackedAnything;
}

bool UPNInventoryComponent::IsAreaFreeInternal(FPNInventoryGridPosition Position, FPNInventoryItemSize Size, const UPNItemInstance* IgnoredItem) const
{
	if (!IsAreaInside(Position, Size))
	{
		return false;
	}

	const int32 Width = FMath::Max(1, Size.GetFinalWidth());
	const int32 Height = FMath::Max(1, Size.GetFinalHeight());

	for (int32 LocalY = 0; LocalY < Height; ++LocalY)
	{
		for (int32 LocalX = 0; LocalX < Width; ++LocalX)
		{
			FPNInventoryGridPosition TestPosition;
			TestPosition.X = Position.X + LocalX;
			TestPosition.Y = Position.Y + LocalY;

			const int32 SlotIndex = PositionToIndex(TestPosition);
			if (!Slots.IsValidIndex(SlotIndex))
			{
				return false;
			}

			const FPNInventorySlot& Slot = Slots[SlotIndex];

			if (Slot.State != EPNInventorySlotState::Empty)
			{
				if (!IgnoredItem || Slot.ItemInstance != IgnoredItem)
				{
					return false;
				}
			}
		}
	}

	return true;
}

int32 UPNInventoryComponent::FindEntryIndexByItem(const UPNItemInstance* ItemInstance) const
{
	if (!ItemInstance)
	{
		return INDEX_NONE;
	}

	for (int32 Index = 0; Index < Items.Num(); ++Index)
	{
		if (Items[Index].ItemInstance == ItemInstance)
		{
			return Index;
		}
	}

	return INDEX_NONE;
}

int32 UPNInventoryComponent::FindEntryIndexByPosition(FPNInventoryGridPosition Position) const
{
	UPNItemInstance* ItemAtPosition = GetItemAtPosition(Position);

	if (!ItemAtPosition)
	{
		return INDEX_NONE;
	}

	return FindEntryIndexByItem(ItemAtPosition);
}

FPNInventoryItemSize UPNInventoryComponent::BuildItemSize(const UPNItemInstance* ItemInstance, bool bRotated) const
{
	FPNInventoryItemSize Size;
	Size.Width = 1;
	Size.Height = 1;
	Size.bRotated = bRotated;

	if (!ItemInstance || !ItemInstance->GetItemData())
	{
		return Size;
	}

	Size.Width = FMath::Max(1, ItemInstance->GetItemData()->GridSize.Width);
	Size.Height = FMath::Max(1, ItemInstance->GetItemData()->GridSize.Height);
	Size.bRotated = bRotated && Settings.bAllowItemRotation;

	return Size;
}

UPNItemInstance* UPNInventoryComponent::DuplicateItemInstance(UPNItemInstance* SourceItem, int32 Quantity)
{
	if (!SourceItem || !SourceItem->GetItemData())
	{
		return nullptr;
	}

	UPNItemInstance* NewInstance = NewObject<UPNItemInstance>(this);
	if (!NewInstance)
	{
		return nullptr;
	}

	NewInstance->Initialize(SourceItem->GetItemData(), Quantity);

	NewInstance->CurrentDurability = SourceItem->CurrentDurability;
	NewInstance->CurrentBatteryCharge = SourceItem->CurrentBatteryCharge;
	NewInstance->RemainingShelfLifeSeconds = SourceItem->RemainingShelfLifeSeconds;
	NewInstance->AmmoInMagazine = SourceItem->AmmoInMagazine;
	NewInstance->bInitialized = true;
	NewInstance->SetQuantityClamped(Quantity);

	return NewInstance;
}

void UPNInventoryComponent::AddItemEntry(UPNItemInstance* ItemInstance, FPNInventoryGridPosition Position, bool bRotated)
{
	if (!ItemInstance)
	{
		return;
	}

	FPNInventoryItemEntry NewEntry;
	NewEntry.ItemInstance = ItemInstance;
	NewEntry.Position = Position;
	NewEntry.bRotated = bRotated;
	NewEntry.Size = BuildItemSize(ItemInstance, bRotated);

	Items.Add(NewEntry);

	RebuildSlots();
}

void UPNInventoryComponent::RemoveItemEntryByIndex(int32 EntryIndex)
{
	if (!Items.IsValidIndex(EntryIndex))
	{
		return;
	}

	Items.RemoveAt(EntryIndex);
}

void UPNInventoryComponent::MarkItemArea(const FPNInventoryItemEntry& Entry)
{
	if (!Entry.ItemInstance)
	{
		return;
	}

	const int32 Width = FMath::Max(1, Entry.Size.GetFinalWidth());
	const int32 Height = FMath::Max(1, Entry.Size.GetFinalHeight());

	for (int32 LocalY = 0; LocalY < Height; ++LocalY)
	{
		for (int32 LocalX = 0; LocalX < Width; ++LocalX)
		{
			FPNInventoryGridPosition Position;
			Position.X = Entry.Position.X + LocalX;
			Position.Y = Entry.Position.Y + LocalY;

			const int32 SlotIndex = PositionToIndex(Position);
			if (!Slots.IsValidIndex(SlotIndex))
			{
				continue;
			}

			FPNInventorySlot& Slot = Slots[SlotIndex];
			Slot.ItemInstance = Entry.ItemInstance;
			Slot.State = EPNInventorySlotState::Occupied;
			Slot.bRootSlot = LocalX == 0 && LocalY == 0;
		}
	}
}

void UPNInventoryComponent::BroadcastInventoryChanged()
{
	OnInventoryChanged.Broadcast();
}