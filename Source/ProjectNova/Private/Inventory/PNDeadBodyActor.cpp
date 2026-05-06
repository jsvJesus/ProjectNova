#include "Inventory/PNDeadBodyActor.h"

APNDeadBodyActor::APNDeadBodyActor()
{
	ContainerType = EPNInventoryType::DeadBody;

	DisplayName = FText::FromString(TEXT("Dead Body"));
	ActionText = FText::FromString(TEXT("Search"));

	ContainerSettings.InventoryType = EPNInventoryType::DeadBody;
	ContainerSettings.GridSize.Columns = 8;
	ContainerSettings.GridSize.Rows = 8;
	ContainerSettings.bUseWeightLimit = false;
	ContainerSettings.MaxWeight = 0.0f;
	ContainerSettings.bAllowItemRotation = true;
	ContainerSettings.bAllowStacking = true;

	// Пока труп — источник лута. Класть предметы в труп запрещаем.
	ContainerSettings.bCanReceiveItems = false;
	ContainerSettings.bCanRemoveItems = true;
	ContainerSettings.bCanDropItems = false;
	ContainerSettings.bCanTradeItems = false;
}