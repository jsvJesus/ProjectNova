#include "UI/PNPlayerHUDComponent.h"

#include "Characters/PNBaseCharacter.h"
#include "Equipment/PNEquipmentComponent.h"
#include "Inventory/PNInventoryComponent.h"
#include "Inventory/PNInventoryContainerComponent.h"
#include "Items/PNItemDataAsset.h"
#include "Items/PNItemInstance.h"
#include "Items/PNQuickSlotComponent.h"
#include "Stats/PNCharacterStatsComponent.h"

UPNPlayerHUDComponent::UPNPlayerHUDComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(false);
}

void UPNPlayerHUDComponent::BeginPlay()
{
	Super::BeginPlay();

	ResolveOwnerCharacter();
	BindToOwnerComponents();

	if (bRefreshOnBeginPlay)
	{
		RefreshHUDData();
	}
}

void UPNPlayerHUDComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindFromOwnerComponents();

	Super::EndPlay(EndPlayReason);
}

void UPNPlayerHUDComponent::SetOwnerCharacter(APNBaseCharacter* InOwnerCharacter)
{
	if (OwnerCharacter == InOwnerCharacter)
	{
		return;
	}

	UnbindFromOwnerComponents();

	OwnerCharacter = InOwnerCharacter;

	BindToOwnerComponents();
	RefreshHUDData();
}

void UPNPlayerHUDComponent::RefreshHUDData()
{
	ResolveOwnerCharacter();

	CachedHUDData = BuildHUDSnapshot();
	OnHUDDataChanged.Broadcast(CachedHUDData);
}

const FPNPlayerHUDSnapshot& UPNPlayerHUDComponent::GetCachedHUDData() const
{
	return CachedHUDData;
}

APNBaseCharacter* UPNPlayerHUDComponent::GetOwnerProjectNovaCharacter() const
{
	return OwnerCharacter;
}

void UPNPlayerHUDComponent::ResolveOwnerCharacter()
{
	if (OwnerCharacter)
	{
		return;
	}

	OwnerCharacter = Cast<APNBaseCharacter>(GetOwner());
}

void UPNPlayerHUDComponent::BindToOwnerComponents()
{
	ResolveOwnerCharacter();

	if (!OwnerCharacter)
	{
		return;
	}

	BindInventory(OwnerCharacter->GetInventoryComponent());
	BindInventory(OwnerCharacter->GetBackpackInventoryComponent());
	BindInventory(OwnerCharacter->GetVestInventoryComponent());

	if (UPNCharacterStatsComponent* StatsComponent = OwnerCharacter->GetCharacterStatsComponent())
	{
		StatsComponent->OnStatsChanged.AddUniqueDynamic(this, &UPNPlayerHUDComponent::HandleHUDSourceChanged);
		StatsComponent->OnHealthChanged.AddUniqueDynamic(this, &UPNPlayerHUDComponent::HandleFloatStatChanged);
		StatsComponent->OnStaminaChanged.AddUniqueDynamic(this, &UPNPlayerHUDComponent::HandleFloatStatChanged);
		StatsComponent->OnDeath.AddUniqueDynamic(this, &UPNPlayerHUDComponent::HandleHUDSourceChanged);
	}

	if (UPNEquipmentComponent* EquipmentComponent = OwnerCharacter->GetEquipmentComponent())
	{
		EquipmentComponent->OnEquipmentChanged.AddUniqueDynamic(this, &UPNPlayerHUDComponent::HandleHUDSourceChanged);
	}

	if (UPNQuickSlotComponent* QuickSlotComponent = OwnerCharacter->GetQuickSlotComponent())
	{
		QuickSlotComponent->OnQuickSlotsChanged.AddUniqueDynamic(this, &UPNPlayerHUDComponent::HandleHUDSourceChanged);
		QuickSlotComponent->OnQuickSlotSelected.AddUniqueDynamic(this, &UPNPlayerHUDComponent::HandleQuickSlotSelected);
	}

	if (UPNInventoryContainerComponent* ContainerComponent = OwnerCharacter->GetInventoryContainerComponent())
	{
		ContainerComponent->OnContainerOpened.AddUniqueDynamic(this, &UPNPlayerHUDComponent::HandleContainerOpened);
		ContainerComponent->OnContainerClosed.AddUniqueDynamic(this, &UPNPlayerHUDComponent::HandleContainerClosed);
		ContainerComponent->OnContainerTransferCompleted.AddUniqueDynamic(this, &UPNPlayerHUDComponent::HandleContainerTransferCompleted);

		BindOpenedContainerInventory(ContainerComponent->GetOpenedContainerInventory());
	}
}

void UPNPlayerHUDComponent::UnbindFromOwnerComponents()
{
	if (!OwnerCharacter)
	{
		UnbindOpenedContainerInventory();
		return;
	}

	UnbindInventory(OwnerCharacter->GetInventoryComponent());
	UnbindInventory(OwnerCharacter->GetBackpackInventoryComponent());
	UnbindInventory(OwnerCharacter->GetVestInventoryComponent());

	if (UPNCharacterStatsComponent* StatsComponent = OwnerCharacter->GetCharacterStatsComponent())
	{
		StatsComponent->OnStatsChanged.RemoveDynamic(this, &UPNPlayerHUDComponent::HandleHUDSourceChanged);
		StatsComponent->OnHealthChanged.RemoveDynamic(this, &UPNPlayerHUDComponent::HandleFloatStatChanged);
		StatsComponent->OnStaminaChanged.RemoveDynamic(this, &UPNPlayerHUDComponent::HandleFloatStatChanged);
		StatsComponent->OnDeath.RemoveDynamic(this, &UPNPlayerHUDComponent::HandleHUDSourceChanged);
	}

	if (UPNEquipmentComponent* EquipmentComponent = OwnerCharacter->GetEquipmentComponent())
	{
		EquipmentComponent->OnEquipmentChanged.RemoveDynamic(this, &UPNPlayerHUDComponent::HandleHUDSourceChanged);
	}

	if (UPNQuickSlotComponent* QuickSlotComponent = OwnerCharacter->GetQuickSlotComponent())
	{
		QuickSlotComponent->OnQuickSlotsChanged.RemoveDynamic(this, &UPNPlayerHUDComponent::HandleHUDSourceChanged);
		QuickSlotComponent->OnQuickSlotSelected.RemoveDynamic(this, &UPNPlayerHUDComponent::HandleQuickSlotSelected);
	}

	if (UPNInventoryContainerComponent* ContainerComponent = OwnerCharacter->GetInventoryContainerComponent())
	{
		ContainerComponent->OnContainerOpened.RemoveDynamic(this, &UPNPlayerHUDComponent::HandleContainerOpened);
		ContainerComponent->OnContainerClosed.RemoveDynamic(this, &UPNPlayerHUDComponent::HandleContainerClosed);
		ContainerComponent->OnContainerTransferCompleted.RemoveDynamic(this, &UPNPlayerHUDComponent::HandleContainerTransferCompleted);
	}

	UnbindOpenedContainerInventory();
}

void UPNPlayerHUDComponent::BindInventory(UPNInventoryComponent* InventoryComponent)
{
	if (!InventoryComponent)
	{
		return;
	}

	InventoryComponent->OnInventoryChanged.AddUniqueDynamic(this, &UPNPlayerHUDComponent::HandleHUDSourceChanged);
}

void UPNPlayerHUDComponent::UnbindInventory(UPNInventoryComponent* InventoryComponent)
{
	if (!InventoryComponent)
	{
		return;
	}

	InventoryComponent->OnInventoryChanged.RemoveDynamic(this, &UPNPlayerHUDComponent::HandleHUDSourceChanged);
}

void UPNPlayerHUDComponent::BindOpenedContainerInventory(UPNInventoryComponent* InventoryComponent)
{
	if (BoundOpenedContainerInventory == InventoryComponent)
	{
		return;
	}

	UnbindOpenedContainerInventory();

	BoundOpenedContainerInventory = InventoryComponent;
	BindInventory(BoundOpenedContainerInventory);
}

void UPNPlayerHUDComponent::UnbindOpenedContainerInventory()
{
	if (!BoundOpenedContainerInventory)
	{
		return;
	}

	UnbindInventory(BoundOpenedContainerInventory);
	BoundOpenedContainerInventory = nullptr;
}

FPNPlayerHUDSnapshot UPNPlayerHUDComponent::BuildHUDSnapshot() const
{
	FPNPlayerHUDSnapshot Snapshot;

	Snapshot.bHasOwnerCharacter = OwnerCharacter != nullptr;

	if (!OwnerCharacter)
	{
		return Snapshot;
	}

	UPNInventoryComponent* InventoryComponent = OwnerCharacter->GetInventoryComponent();
	UPNInventoryComponent* BackpackInventoryComponent = OwnerCharacter->GetBackpackInventoryComponent();
	UPNInventoryComponent* VestInventoryComponent = OwnerCharacter->GetVestInventoryComponent();

	UPNEquipmentComponent* EquipmentComponent = OwnerCharacter->GetEquipmentComponent();

	UPNItemDataAsset* EquippedBackpackData = EquipmentComponent
		? EquipmentComponent->GetEquippedItemData(EPNEquipmentSlot::Backpack)
		: nullptr;

	UPNItemDataAsset* EquippedArmorData = EquipmentComponent
		? EquipmentComponent->GetEquippedItemData(EPNEquipmentSlot::Armor)
		: nullptr;

	Snapshot.bHasInventory = InventoryComponent != nullptr;
	Snapshot.bHasBackpackInventory = BackpackInventoryComponent != nullptr;
	Snapshot.bHasVestInventory = VestInventoryComponent != nullptr;
	Snapshot.bHasStats = OwnerCharacter->GetCharacterStatsComponent() != nullptr;
	Snapshot.bHasEquipment = OwnerCharacter->GetEquipmentComponent() != nullptr;
	Snapshot.bHasQuickSlots = OwnerCharacter->GetQuickSlotComponent() != nullptr;
	Snapshot.bHasContainerComponent = OwnerCharacter->GetInventoryContainerComponent() != nullptr;

	Snapshot.Stats = BuildStatsData();

	Snapshot.MainInventory = BuildInventoryPanelData(
		InventoryComponent,
		EPNHUDInventoryPanel::MainInventory,
		nullptr
	);

	Snapshot.BackpackInventory = BuildInventoryPanelData(
		BackpackInventoryComponent,
		EPNHUDInventoryPanel::Backpack,
		EquippedBackpackData
	);

	Snapshot.VestInventory = BuildInventoryPanelData(
		VestInventoryComponent,
		EPNHUDInventoryPanel::Vest,
		EquippedArmorData
	);

	const int32 SelectedQuickSlotIndex = OwnerCharacter->GetQuickSlotComponent()
		? OwnerCharacter->GetQuickSlotComponent()->GetSelectedQuickSlotIndex()
		: INDEX_NONE;

	const TArray<FPNInventoryQuickSlotEntry>* QuickSlotEntries = nullptr;

	if (OwnerCharacter->GetQuickSlotComponent())
	{
		QuickSlotEntries = &OwnerCharacter->GetQuickSlotComponent()->GetQuickSlots();
	}
	else if (InventoryComponent)
	{
		QuickSlotEntries = &InventoryComponent->GetQuickSlots();
	}

	if (QuickSlotEntries)
	{
		for (const FPNInventoryQuickSlotEntry& QuickSlotEntry : *QuickSlotEntries)
		{
			Snapshot.QuickSlots.Add(BuildQuickSlotData(QuickSlotEntry, SelectedQuickSlotIndex));
		}
	}

	Snapshot.Equipment = BuildEquipmentData();
	Snapshot.Container = BuildContainerData();

	return Snapshot;
}

FPNHUDCharacterStatsData UPNPlayerHUDComponent::BuildStatsData() const
{
	FPNHUDCharacterStatsData StatsData;

	if (!OwnerCharacter)
	{
		return StatsData;
	}

	UPNCharacterStatsComponent* StatsComponent = OwnerCharacter->GetCharacterStatsComponent();

	if (!StatsComponent)
	{
		StatsData.Weight.Set(GetTotalCarriedWeight(), 0.0f);
		StatsData.bIsDead = OwnerCharacter->IsDead();
		StatsData.bIsSprinting = OwnerCharacter->IsSprinting();
		return StatsData;
	}

	const FPNCharacterCurrentStats& CurrentStats = StatsComponent->GetCurrentStats();
	const FPNCharacterAttributeStats& FinalStats = StatsComponent->GetFinalStats();

	StatsData.Health.Set(CurrentStats.Health, FinalStats.Health);
	StatsData.Stamina.Set(CurrentStats.Stamina, FinalStats.Endurance);

	StatsData.Hunger.Set(CurrentStats.Hunger, 100.0f);
	StatsData.Thirst.Set(CurrentStats.Thirst, 100.0f);

	StatsData.Weight.Set(GetTotalCarriedWeight(), StatsComponent->GetMaxWeight());

	StatsData.Radiation.Set(CurrentStats.Radiation, 100.0f);
	StatsData.Toxicity.Set(CurrentStats.Toxicity, 100.0f);
	StatsData.Psy.Set(CurrentStats.Psy, 100.0f);

	StatsData.Bleeding.Set(CurrentStats.Bleeding, 100.0f);
	StatsData.Wounds.Set(CurrentStats.Wounds, 100.0f);
	StatsData.Burn.Set(CurrentStats.Burn, 100.0f);
	StatsData.ChemicalBurn.Set(CurrentStats.ChemicalBurn, 100.0f);
	StatsData.ElectricShock.Set(CurrentStats.ElectricShock, 100.0f);

	StatsData.bIsDead = StatsComponent->IsDead();
	StatsData.bIsSprinting = OwnerCharacter->IsSprinting();
	StatsData.bCanSprint = StatsComponent->CanSprint();

	return StatsData;
}

FPNHUDInventoryPanelData UPNPlayerHUDComponent::BuildInventoryPanelData(
	UPNInventoryComponent* InventoryComponent,
	EPNHUDInventoryPanel Panel,
	UPNItemDataAsset* SourceItemData
) const
{
	FPNHUDInventoryPanelData PanelData;
	PanelData.Panel = Panel;

	switch (Panel)
	{
	case EPNHUDInventoryPanel::MainInventory:
		PanelData.DisplayTitle = FText::FromString(TEXT("Inventory"));
		break;

	case EPNHUDInventoryPanel::Vest:
		PanelData.DisplayTitle = FText::FromString(TEXT("Armor"));
		break;

	case EPNHUDInventoryPanel::Backpack:
		PanelData.DisplayTitle = FText::FromString(TEXT("Backpack"));
		break;

	case EPNHUDInventoryPanel::OpenedContainer:
		PanelData.DisplayTitle = FText::FromString(TEXT("Container"));
		break;

	default:
		PanelData.DisplayTitle = FText::FromString(TEXT("Inventory"));
		break;
	}

	if (SourceItemData)
	{
		const FText SourceItemName = SourceItemData->GetItemName();
		if (!SourceItemName.IsEmpty())
		{
			PanelData.DisplayTitle = SourceItemName;
		}

		PanelData.DisplayIcon = SourceItemData->Visual.Icon;
	}

	if (!InventoryComponent)
	{
		return PanelData;
	}

	PanelData.InventoryType = InventoryComponent->Settings.InventoryType;
	PanelData.Columns = InventoryComponent->GetColumns();
	PanelData.Rows = InventoryComponent->GetRows();
	PanelData.SlotCount = InventoryComponent->GetSlotCount();

	PanelData.bUsesWeightLimit = InventoryComponent->UsesWeightLimit();
	PanelData.bCanReceiveItems = InventoryComponent->CanReceiveItems();
	PanelData.bCanRemoveItems = InventoryComponent->CanRemoveItems();

	PanelData.Weight.Set(InventoryComponent->GetCurrentWeight(), InventoryComponent->GetMaxWeight());

	PanelData.bIsActive =
		PanelData.bCanReceiveItems ||
		PanelData.bCanRemoveItems ||
		InventoryComponent->GetInventoryItemCount() > 0;

	if (Panel == EPNHUDInventoryPanel::Backpack || Panel == EPNHUDInventoryPanel::Vest)
	{
		PanelData.bIsActive = SourceItemData != nullptr && PanelData.bIsActive;
	}

	const TArray<FPNRepInventoryItemEntry>& ReplicatedItems = InventoryComponent->GetReplicatedItems();

	if (ReplicatedItems.Num() > 0)
	{
		for (const FPNRepInventoryItemEntry& RepEntry : ReplicatedItems)
		{
			if (!RepEntry.IsValid())
			{
				continue;
			}

			FPNHUDInventoryItemData HUDItem;
			HUDItem.Item = BuildItemViewDataFromRep(RepEntry.InstanceData);
			HUDItem.Position = RepEntry.Position;
			HUDItem.Size = RepEntry.Size;
			HUDItem.bRotated = RepEntry.bRotated;

			PanelData.Items.Add(HUDItem);
		}

		return PanelData;
	}

	const TArray<FPNInventoryItemEntry>& RuntimeItems = InventoryComponent->GetItems();

	for (const FPNInventoryItemEntry& RuntimeEntry : RuntimeItems)
	{
		if (!RuntimeEntry.ItemInstance)
		{
			continue;
		}

		FPNHUDInventoryItemData HUDItem;
		HUDItem.Item = BuildItemViewDataFromInstance(RuntimeEntry.ItemInstance);
		HUDItem.Position = RuntimeEntry.Position;
		HUDItem.Size = RuntimeEntry.Size;
		HUDItem.bRotated = RuntimeEntry.bRotated;

		PanelData.Items.Add(HUDItem);
	}

	return PanelData;
}

FPNHUDItemViewData UPNPlayerHUDComponent::BuildItemViewDataFromRep(const FPNRepItemInstanceData& InstanceData) const
{
	FPNHUDItemViewData ItemViewData;

	if (!InstanceData.IsValid())
	{
		return ItemViewData;
	}

	ItemViewData.bValid = true;
	ItemViewData.ItemData = InstanceData.ItemData;
	ItemViewData.Quantity = InstanceData.Quantity;
	ItemViewData.AmmoInMagazine = InstanceData.AmmoInMagazine;

	UPNItemDataAsset* ItemData = InstanceData.ItemData;

	if (!ItemData)
	{
		return ItemViewData;
	}

	ItemViewData.TotalWeight = ItemData->GetTotalWeightForCount(InstanceData.Quantity);

	ItemViewData.DurabilityPercent = CalculateDurabilityPercent(ItemData, InstanceData.CurrentDurability);
	ItemViewData.BatteryPercent = CalculateBatteryPercent(ItemData, InstanceData.CurrentBatteryCharge);
	ItemViewData.ExpirationPercent = CalculateExpirationPercent(ItemData, InstanceData.RemainingShelfLifeSeconds);

	ItemViewData.bBroken = ItemData->UsesDurability()
		&& ItemData->Durability.bBrokenWhenZero
		&& InstanceData.CurrentDurability <= 0.0f;

	ItemViewData.bExpired = ItemData->UsesExpiration()
		&& InstanceData.RemainingShelfLifeSeconds <= 0.0f;

	return ItemViewData;
}

FPNHUDItemViewData UPNPlayerHUDComponent::BuildItemViewDataFromInstance(const UPNItemInstance* ItemInstance) const
{
	FPNHUDItemViewData ItemViewData;

	if (!ItemInstance || !ItemInstance->IsValidItem())
	{
		return ItemViewData;
	}

	ItemViewData.bValid = true;
	ItemViewData.ItemData = ItemInstance->GetItemData();
	ItemViewData.Quantity = ItemInstance->Quantity;
	ItemViewData.TotalWeight = ItemInstance->GetTotalWeight();

	ItemViewData.DurabilityPercent = ItemInstance->GetDurabilityPercent();
	ItemViewData.BatteryPercent = ItemInstance->GetBatteryChargePercent();
	ItemViewData.ExpirationPercent = ItemInstance->GetExpirationPercent();

	ItemViewData.AmmoInMagazine = ItemInstance->AmmoInMagazine;
	ItemViewData.bBroken = ItemInstance->IsBroken();
	ItemViewData.bExpired = ItemInstance->IsExpired();

	return ItemViewData;
}

FPNHUDQuickSlotData UPNPlayerHUDComponent::BuildQuickSlotData(
	const FPNInventoryQuickSlotEntry& QuickSlotEntry,
	int32 SelectedQuickSlotIndex
) const
{
	FPNHUDQuickSlotData QuickSlotData;

	QuickSlotData.SlotIndex = QuickSlotEntry.SlotIndex;
	QuickSlotData.bValidSlot = QuickSlotEntry.IsValidSlot();
	QuickSlotData.bOccupied = QuickSlotEntry.IsOccupied();
	QuickSlotData.bSelected = QuickSlotEntry.SlotIndex == SelectedQuickSlotIndex;
	QuickSlotData.Item = BuildItemViewDataFromRep(QuickSlotEntry.InstanceData);

	return QuickSlotData;
}

FPNHUDEquipmentData UPNPlayerHUDComponent::BuildEquipmentData() const
{
	FPNHUDEquipmentData EquipmentData;

	if (!OwnerCharacter || !OwnerCharacter->GetEquipmentComponent())
	{
		return EquipmentData;
	}

	UPNEquipmentComponent* EquipmentComponent = OwnerCharacter->GetEquipmentComponent();

	for (const FPNEquipmentSlotEntry& EquipmentSlotEntry : EquipmentComponent->GetEquipmentSlots())
	{
		FPNHUDEquipmentSlotData HUDSlot;
		HUDSlot.Slot = EquipmentSlotEntry.Slot;
		HUDSlot.bOccupied = EquipmentSlotEntry.IsOccupied();
		HUDSlot.Item = BuildItemViewDataFromRep(EquipmentSlotEntry.InstanceData);

		EquipmentData.EquipmentSlots.Add(HUDSlot);
	}

	for (const FPNEquipmentInternalSlotEntry& InternalSlotEntry : EquipmentComponent->GetHelmetInternalSlots())
	{
		FPNHUDInternalEquipmentSlotData HUDSlot;
		HUDSlot.Container = InternalSlotEntry.Container;
		HUDSlot.SlotIndex = InternalSlotEntry.SlotIndex;
		HUDSlot.bOccupied = InternalSlotEntry.IsOccupied();
		HUDSlot.Item = BuildItemViewDataFromRep(InternalSlotEntry.InstanceData);

		EquipmentData.HelmetInternalSlots.Add(HUDSlot);
	}

	for (const FPNEquipmentInternalSlotEntry& InternalSlotEntry : EquipmentComponent->GetArmorInternalSlots())
	{
		FPNHUDInternalEquipmentSlotData HUDSlot;
		HUDSlot.Container = InternalSlotEntry.Container;
		HUDSlot.SlotIndex = InternalSlotEntry.SlotIndex;
		HUDSlot.bOccupied = InternalSlotEntry.IsOccupied();
		HUDSlot.Item = BuildItemViewDataFromRep(InternalSlotEntry.InstanceData);

		EquipmentData.ArmorInternalSlots.Add(HUDSlot);
	}

	return EquipmentData;
}

FPNHUDContainerData UPNPlayerHUDComponent::BuildContainerData() const
{
	FPNHUDContainerData ContainerData;

	if (!OwnerCharacter || !OwnerCharacter->GetInventoryContainerComponent())
	{
		return ContainerData;
	}

	UPNInventoryContainerComponent* ContainerComponent = OwnerCharacter->GetInventoryContainerComponent();

	ContainerData.bIsOpen = ContainerComponent->IsContainerOpen();
	ContainerData.OpenedContainerType = ContainerComponent->GetOpenedContainerType();

	ContainerData.Inventory = BuildInventoryPanelData(
		ContainerComponent->GetOpenedContainerInventory(),
		EPNHUDInventoryPanel::OpenedContainer
	);

	return ContainerData;
}

float UPNPlayerHUDComponent::GetTotalCarriedWeight() const
{
	if (!OwnerCharacter)
	{
		return 0.0f;
	}

	float TotalWeight = 0.0f;

	if (OwnerCharacter->GetInventoryComponent())
	{
		TotalWeight += OwnerCharacter->GetInventoryComponent()->GetCurrentWeight();
	}

	if (OwnerCharacter->GetBackpackInventoryComponent())
	{
		TotalWeight += OwnerCharacter->GetBackpackInventoryComponent()->GetCurrentWeight();
	}

	if (OwnerCharacter->GetVestInventoryComponent())
	{
		TotalWeight += OwnerCharacter->GetVestInventoryComponent()->GetCurrentWeight();
	}

	return TotalWeight;
}

float UPNPlayerHUDComponent::CalculateDurabilityPercent(UPNItemDataAsset* ItemData, float CurrentDurability) const
{
	if (!ItemData || !ItemData->UsesDurability())
	{
		return 1.0f;
	}

	const float MaxDurability = FMath::Max(0.0f, ItemData->Durability.MaxDurability);

	if (MaxDurability <= 0.0f)
	{
		return 0.0f;
	}

	return FMath::Clamp(CurrentDurability / MaxDurability, 0.0f, 1.0f);
}

float UPNPlayerHUDComponent::CalculateBatteryPercent(UPNItemDataAsset* ItemData, float CurrentBatteryCharge) const
{
	if (!ItemData || !ItemData->UsesBattery())
	{
		return 1.0f;
	}

	const float MaxCharge = FMath::Max(0.0f, ItemData->Battery.MaxCharge);

	if (MaxCharge <= 0.0f)
	{
		return 0.0f;
	}

	return FMath::Clamp(CurrentBatteryCharge / MaxCharge, 0.0f, 1.0f);
}

float UPNPlayerHUDComponent::CalculateExpirationPercent(UPNItemDataAsset* ItemData, float RemainingShelfLifeSeconds) const
{
	if (!ItemData || !ItemData->UsesExpiration())
	{
		return 1.0f;
	}

	const float ShelfLifeSeconds = FMath::Max(0.0f, ItemData->Expiration.ShelfLifeSeconds);

	if (ShelfLifeSeconds <= 0.0f)
	{
		return 0.0f;
	}

	return FMath::Clamp(RemainingShelfLifeSeconds / ShelfLifeSeconds, 0.0f, 1.0f);
}

void UPNPlayerHUDComponent::HandleHUDSourceChanged()
{
	RefreshHUDData();
}

void UPNPlayerHUDComponent::HandleFloatStatChanged(float OldValue, float NewValue)
{
	RefreshHUDData();
}

void UPNPlayerHUDComponent::HandleQuickSlotSelected(int32 SlotIndex, UPNItemDataAsset* ItemData)
{
	RefreshHUDData();
}

void UPNPlayerHUDComponent::HandleContainerOpened(const FPNInventoryContainerOpenResponse& Response)
{
	BindOpenedContainerInventory(Response.ContainerInventory);
	RefreshHUDData();
}

void UPNPlayerHUDComponent::HandleContainerClosed()
{
	UnbindOpenedContainerInventory();
	RefreshHUDData();
}

void UPNPlayerHUDComponent::HandleContainerTransferCompleted(const FPNInventoryContainerTransferResponse& Response)
{
	RefreshHUDData();
}