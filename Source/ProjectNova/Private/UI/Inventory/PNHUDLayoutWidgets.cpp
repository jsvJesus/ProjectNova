#include "UI/Inventory/PNHUDLayoutWidgets.h"

#include "Items/PNItemDataAsset.h"

UPNHUDLayoutWidget::UPNHUDLayoutWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UPNHUDLayoutWidget::SetHUDData(const FPNPlayerHUDSnapshot& InHUDData)
{
	CachedHUDData = InHUDData;
	BP_OnHUDDataUpdated(CachedHUDData);
}

void UPNHUDLayoutWidget::SetLayoutVisible(bool bVisible)
{
	SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	BP_OnLayoutVisibilityChanged(bVisible);
}

const FPNPlayerHUDSnapshot& UPNHUDLayoutWidget::GetCachedHUDData() const
{
	return CachedHUDData;
}

void UPNNavigationLayoutWidget::RequestPage(EPNInventoryHUDPage Page)
{
	OnNavigationPageRequested.Broadcast(Page);
}

void UPNNavigationLayoutWidget::SetActivePage(EPNInventoryHUDPage Page)
{
	if (ActivePage == Page)
	{
		return;
	}

	ActivePage = Page;
	BP_OnActivePageChanged(ActivePage);
}

EPNInventoryHUDPage UPNNavigationLayoutWidget::GetActivePage() const
{
	return ActivePage;
}

void UPNNavigationLayoutWidget::RequestCustomizationPage()
{
	RequestPage(EPNInventoryHUDPage::Customization);
}

void UPNNavigationLayoutWidget::RequestInventoryPage()
{
	RequestPage(EPNInventoryHUDPage::Inventory);
}

void UPNNavigationLayoutWidget::RequestCraftPage()
{
	RequestPage(EPNInventoryHUDPage::Craft);
}

void UPNNavigationLayoutWidget::RequestMissionsPage()
{
	RequestPage(EPNInventoryHUDPage::Missions);
}

void UPNNavigationLayoutWidget::RequestMapPage()
{
	RequestPage(EPNInventoryHUDPage::Map);
}

void UPNNavigationLayoutWidget::RequestOptionsPage()
{
	RequestPage(EPNInventoryHUDPage::Options);
}

void UPNInventoryGridWidget::SetInventoryData(const FPNHUDInventoryPanelData& InInventoryData)
{
	InventoryData = InInventoryData;
	BP_OnInventoryDataUpdated(InventoryData);
}

const FPNHUDInventoryPanelData& UPNInventoryGridWidget::GetInventoryData() const
{
	return InventoryData;
}

int32 UPNInventoryGridWidget::GetColumns() const
{
	return InventoryData.Columns;
}

int32 UPNInventoryGridWidget::GetRows() const
{
	return InventoryData.Rows;
}

float UPNInventoryGridWidget::GetSlotSize() const
{
	return 64.0f;
}

void UPNInventoryItemWidget::SetInventoryItemData(const FPNHUDInventoryItemData& InItemData)
{
	InventoryItemData = InItemData;
	BP_OnInventoryItemDataUpdated(InventoryItemData);
}

const FPNHUDInventoryItemData& UPNInventoryItemWidget::GetInventoryItemData() const
{
	return InventoryItemData;
}

FPNHUDItemViewData UPNInventoryItemWidget::GetItemViewData() const
{
	return InventoryItemData.Item;
}

UPNItemDataAsset* UPNInventoryItemWidget::GetItemData() const
{
	return InventoryItemData.Item.ItemData;
}

FText UPNInventoryItemWidget::GetItemName() const
{
	if (!InventoryItemData.Item.bValid || !InventoryItemData.Item.ItemData)
	{
		return FText::FromString(TEXT("Empty"));
	}

	return InventoryItemData.Item.ItemData->GetItemName();
}

int32 UPNInventoryItemWidget::GetQuantity() const
{
	return InventoryItemData.Item.Quantity;
}

void UPNEquipmentLayoutWidget::SetEquipmentData(const FPNHUDEquipmentData& InEquipmentData)
{
	EquipmentData = InEquipmentData;
	BP_OnEquipmentDataUpdated(EquipmentData);
}

const FPNHUDEquipmentData& UPNEquipmentLayoutWidget::GetEquipmentData() const
{
	return EquipmentData;
}

void UPNEquipmentSlotWidget::SetEquipmentSlotData(const FPNHUDEquipmentSlotData& InSlotData)
{
	EquipmentSlotData = InSlotData;
	BP_OnEquipmentSlotDataUpdated(EquipmentSlotData);
}

const FPNHUDEquipmentSlotData& UPNEquipmentSlotWidget::GetEquipmentSlotData() const
{
	return EquipmentSlotData;
}

UPNItemDataAsset* UPNEquipmentSlotWidget::GetItemData() const
{
	return EquipmentSlotData.Item.ItemData;
}

void UPNInternalEquipmentSlotWidget::SetInternalEquipmentSlotData(const FPNHUDInternalEquipmentSlotData& InSlotData)
{
	InternalSlotData = InSlotData;
	BP_OnInternalEquipmentSlotDataUpdated(InternalSlotData);
}

const FPNHUDInternalEquipmentSlotData& UPNInternalEquipmentSlotWidget::GetInternalEquipmentSlotData() const
{
	return InternalSlotData;
}

UPNItemDataAsset* UPNInternalEquipmentSlotWidget::GetItemData() const
{
	return InternalSlotData.Item.ItemData;
}

void UPNContainerLayoutWidget::SetContainerData(const FPNHUDContainerData& InContainerData)
{
	ContainerData = InContainerData;
	BP_OnContainerDataUpdated(ContainerData);
}

const FPNHUDContainerData& UPNContainerLayoutWidget::GetContainerData() const
{
	return ContainerData;
}

bool UPNContainerLayoutWidget::IsContainerOpen() const
{
	return ContainerData.bIsOpen;
}

void UPNQuickSlotLayoutWidget::SetQuickSlotsData(const TArray<FPNHUDQuickSlotData>& InQuickSlotsData)
{
	QuickSlotsData = InQuickSlotsData;
	BP_OnQuickSlotsDataUpdated(QuickSlotsData);
}

const TArray<FPNHUDQuickSlotData>& UPNQuickSlotLayoutWidget::GetQuickSlotsData() const
{
	return QuickSlotsData;
}

void UPNQuickSlotWidget::SetQuickSlotData(const FPNHUDQuickSlotData& InQuickSlotData)
{
	QuickSlotData = InQuickSlotData;
	BP_OnQuickSlotDataUpdated(QuickSlotData);
}

const FPNHUDQuickSlotData& UPNQuickSlotWidget::GetQuickSlotData() const
{
	return QuickSlotData;
}

UPNItemDataAsset* UPNQuickSlotWidget::GetItemData() const
{
	return QuickSlotData.Item.ItemData;
}

void UPNPlayerStatsLayoutWidget::SetStatsData(const FPNHUDCharacterStatsData& InStatsData)
{
	StatsData = InStatsData;
	BP_OnStatsDataUpdated(StatsData);
}

const FPNHUDCharacterStatsData& UPNPlayerStatsLayoutWidget::GetStatsData() const
{
	return StatsData;
}

void UPNPlayerInfoLayoutWidget::SetPlayerInfoData(const FPNHUDPlayerInfoData& InPlayerInfoData)
{
	PlayerInfoData = InPlayerInfoData;
	BP_OnPlayerInfoDataUpdated(PlayerInfoData);
}

const FPNHUDPlayerInfoData& UPNPlayerInfoLayoutWidget::GetPlayerInfoData() const
{
	return PlayerInfoData;
}