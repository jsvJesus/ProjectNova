#include "Inventory/PNSafeLockerActor.h"

APNSafeLockerActor::APNSafeLockerActor()
{
	ContainerType = EPNInventoryType::SafeLocker;

	DisplayName = FText::FromString(TEXT("Safe Locker"));
	ActionText = FText::FromString(TEXT("Open"));

	ContainerSettings.InventoryType = EPNInventoryType::SafeLocker;
	ContainerSettings.GridSize.Columns = 10;
	ContainerSettings.GridSize.Rows = 10;
	ContainerSettings.bUseWeightLimit = false;
	ContainerSettings.MaxWeight = 0.0f;
	ContainerSettings.bAllowItemRotation = true;
	ContainerSettings.bAllowStacking = true;
	ContainerSettings.bCanReceiveItems = true;
	ContainerSettings.bCanRemoveItems = true;
	ContainerSettings.bCanDropItems = false;
	ContainerSettings.bCanTradeItems = false;
}