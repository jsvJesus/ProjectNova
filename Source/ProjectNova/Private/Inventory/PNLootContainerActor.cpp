#include "Inventory/PNLootContainerActor.h"

APNLootContainerActor::APNLootContainerActor()
{
	ContainerType = EPNInventoryType::LootContainer;

	DisplayName = FText::FromString(TEXT("Loot Container"));
	ActionText = FText::FromString(TEXT("Open"));

	ContainerSettings.InventoryType = EPNInventoryType::LootContainer;
	ContainerSettings.GridSize.Columns = 8;
	ContainerSettings.GridSize.Rows = 6;
	ContainerSettings.bUseWeightLimit = false;
	ContainerSettings.MaxWeight = 0.0f;
	ContainerSettings.bAllowItemRotation = true;
	ContainerSettings.bAllowStacking = true;
	ContainerSettings.bCanReceiveItems = true;
	ContainerSettings.bCanRemoveItems = true;
	ContainerSettings.bCanDropItems = false;
	ContainerSettings.bCanTradeItems = false;
}