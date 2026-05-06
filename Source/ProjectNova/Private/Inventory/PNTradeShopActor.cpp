#include "Inventory/PNTradeShopActor.h"

APNTradeShopActor::APNTradeShopActor()
{
	ContainerType = EPNInventoryType::TradeShop;

	DisplayName = FText::FromString(TEXT("Trader"));
	ActionText = FText::FromString(TEXT("Trade"));

	ContainerSettings.InventoryType = EPNInventoryType::TradeShop;
	ContainerSettings.GridSize.Columns = 10;
	ContainerSettings.GridSize.Rows = 8;
	ContainerSettings.bUseWeightLimit = false;
	ContainerSettings.MaxWeight = 0.0f;
	ContainerSettings.bAllowItemRotation = true;
	ContainerSettings.bAllowStacking = true;

	// Нельзя просто так перетаскивать вещи из торговца.
	// Позже Trade/Economy API будет покупать/продавать через отдельные серверные проверки.
	ContainerSettings.bCanReceiveItems = false;
	ContainerSettings.bCanRemoveItems = false;
	ContainerSettings.bCanDropItems = false;
	ContainerSettings.bCanTradeItems = true;
}