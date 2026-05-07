#include "UI/Inventory/PNInventoryHUDWidget.h"

#include "Components/Widget.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"

UPNInventoryHUDWidget::UPNInventoryHUDWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bBuildNativeLayout = false;
	bShowInventoryOnStart = false;
	ActivePage = EPNInventoryHUDPage::Inventory;
}

void UPNInventoryHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindNavigation();
	ApplyPageVisibility();
	RefreshAllLayouts();
}

void UPNInventoryHUDWidget::NativeDestruct()
{
	UnbindNavigation();

	Super::NativeDestruct();
}

void UPNInventoryHUDWidget::SetHUDData(const FPNPlayerHUDSnapshot& InHUDData)
{
	Super::SetHUDData(InHUDData);

	CachedPlayerInfoData = BuildPlayerInfoData();

	PushHUDDataToLayouts();
	PushPlayerInfoToLayout();
	ApplyPageVisibility();
}

void UPNInventoryHUDWidget::SetInventoryVisible(bool bVisible)
{
	Super::SetInventoryVisible(bVisible);

	ApplyPageVisibility();
}

void UPNInventoryHUDWidget::SetActivePage(EPNInventoryHUDPage NewPage)
{
	if (NewPage == EPNInventoryHUDPage::None)
	{
		NewPage = EPNInventoryHUDPage::Inventory;
	}

	if (ActivePage == NewPage)
	{
		ApplyPageVisibility();
		return;
	}

	ActivePage = NewPage;

	if (NavigationLayout)
	{
		NavigationLayout->SetActivePage(ActivePage);
	}

	ApplyPageVisibility();

	BP_OnActivePageChanged(ActivePage);
}

EPNInventoryHUDPage UPNInventoryHUDWidget::GetActivePage() const
{
	return ActivePage;
}

void UPNInventoryHUDWidget::RefreshAllLayouts()
{
	CachedPlayerInfoData = BuildPlayerInfoData();

	PushHUDDataToLayouts();
	PushPlayerInfoToLayout();
	ApplyPageVisibility();
}

const FPNHUDPlayerInfoData& UPNInventoryHUDWidget::GetPlayerInfoData() const
{
	return CachedPlayerInfoData;
}

void UPNInventoryHUDWidget::BindNavigation()
{
	if (!NavigationLayout)
	{
		return;
	}

	NavigationLayout->OnNavigationPageRequested.AddUniqueDynamic(
		this,
		&UPNInventoryHUDWidget::HandleNavigationPageRequested
	);

	NavigationLayout->SetActivePage(ActivePage);
}

void UPNInventoryHUDWidget::UnbindNavigation()
{
	if (!NavigationLayout)
	{
		return;
	}

	NavigationLayout->OnNavigationPageRequested.RemoveDynamic(
		this,
		&UPNInventoryHUDWidget::HandleNavigationPageRequested
	);
}

void UPNInventoryHUDWidget::PushHUDDataToLayouts()
{
	const FPNPlayerHUDSnapshot& HUDData = GetHUDData();

	if (NavigationLayout)
	{
		NavigationLayout->SetHUDData(HUDData);
	}

	if (InventoryLayout)
	{
		InventoryLayout->SetHUDData(HUDData);
		InventoryLayout->SetInventoryData(HUDData.MainInventory);
	}

	if (VestInventoryLayout)
	{
		VestInventoryLayout->SetHUDData(HUDData);
		VestInventoryLayout->SetInventoryData(HUDData.VestInventory);
	}

	if (BackpackInventoryLayout)
	{
		BackpackInventoryLayout->SetHUDData(HUDData);
		BackpackInventoryLayout->SetInventoryData(HUDData.BackpackInventory);
	}

	if (EquipmentLayout)
	{
		EquipmentLayout->SetHUDData(HUDData);
		EquipmentLayout->SetEquipmentData(HUDData.Equipment);
	}

	if (ContainerLayout)
	{
		ContainerLayout->SetHUDData(HUDData);
		ContainerLayout->SetContainerData(HUDData.Container);
	}

	if (QuickSlotLayout)
	{
		QuickSlotLayout->SetHUDData(HUDData);
		QuickSlotLayout->SetQuickSlotsData(HUDData.QuickSlots);
	}

	if (PlayerStatsLayout)
	{
		PlayerStatsLayout->SetHUDData(HUDData);
		PlayerStatsLayout->SetStatsData(HUDData.Stats);
	}

	if (PlayerInfoLayout)
	{
		PlayerInfoLayout->SetHUDData(HUDData);
	}
}

void UPNInventoryHUDWidget::PushPlayerInfoToLayout()
{
	if (PlayerInfoLayout)
	{
		PlayerInfoLayout->SetPlayerInfoData(CachedPlayerInfoData);
	}
}

void UPNInventoryHUDWidget::ApplyPageVisibility()
{
	const bool bRootVisible = IsInventoryVisible();

	const bool bInventoryPageVisible = bRootVisible && ActivePage == EPNInventoryHUDPage::Inventory;
	const bool bCustomizationVisible = bRootVisible && ActivePage == EPNInventoryHUDPage::Customization;
	const bool bCraftVisible = bRootVisible && ActivePage == EPNInventoryHUDPage::Craft;
	const bool bMissionsVisible = bRootVisible && ActivePage == EPNInventoryHUDPage::Missions;
	const bool bMapVisible = bRootVisible && ActivePage == EPNInventoryHUDPage::Map;
	const bool bOptionsVisible = bRootVisible && ActivePage == EPNInventoryHUDPage::Options;

	SetLayoutVisible(NavigationLayout, bRootVisible);

	SetLayoutVisible(InventoryLayout, bInventoryPageVisible);
	SetLayoutVisible(VestInventoryLayout, bInventoryPageVisible);
	SetLayoutVisible(BackpackInventoryLayout, bInventoryPageVisible);
	SetLayoutVisible(EquipmentLayout, bInventoryPageVisible);
	SetLayoutVisible(ContainerLayout, bInventoryPageVisible);
	SetLayoutVisible(QuickSlotLayout, bInventoryPageVisible);
	SetLayoutVisible(PlayerStatsLayout, bInventoryPageVisible);
	SetLayoutVisible(PlayerInfoLayout, bInventoryPageVisible);

	SetWidgetVisible(CustomizationLayout, bCustomizationVisible);
	SetWidgetVisible(CraftLayout, bCraftVisible);
	SetWidgetVisible(MissionsLayout, bMissionsVisible);
	SetWidgetVisible(MapLayout, bMapVisible);
	SetWidgetVisible(OptionsLayout, bOptionsVisible);
}

void UPNInventoryHUDWidget::SetWidgetVisible(UWidget* Widget, bool bVisible) const
{
	if (!Widget)
	{
		return;
	}

	Widget->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UPNInventoryHUDWidget::SetLayoutVisible(UPNHUDLayoutWidget* Layout, bool bVisible) const
{
	if (!Layout)
	{
		return;
	}

	Layout->SetLayoutVisible(bVisible);
}

FPNHUDPlayerInfoData UPNInventoryHUDWidget::BuildPlayerInfoData() const
{
	FPNHUDPlayerInfoData InfoData;

	InfoData.PlayerName = FText::FromString(TEXT("Player"));
	InfoData.RankText = FText::FromString(TEXT("Rookie"));
	InfoData.ClanName = FText::FromString(TEXT("No Clan"));
	InfoData.GameDollars = 0;
	InfoData.GameCoins = 0;

	APlayerController* OwningController = GetOwningPlayer();
	if (!OwningController)
	{
		return InfoData;
	}

	APlayerState* OwningPlayerState = OwningController->PlayerState;
	if (!OwningPlayerState)
	{
		return InfoData;
	}

	const FString PlayerNameString = OwningPlayerState->GetPlayerName();
	if (!PlayerNameString.IsEmpty())
	{
		InfoData.PlayerName = FText::FromString(PlayerNameString);
	}

	return InfoData;
}

void UPNInventoryHUDWidget::HandleNavigationPageRequested(EPNInventoryHUDPage RequestedPage)
{
	SetActivePage(RequestedPage);
}