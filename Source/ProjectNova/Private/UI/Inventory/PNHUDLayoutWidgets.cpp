#include "UI/Inventory/PNHUDLayoutWidgets.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Items/PNItemDataAsset.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateTypes.h"
#include "Widgets/SWidget.h"

namespace
{
	FSlateBrush PNMakeTextureBrush(UTexture2D* Texture, const FVector2D& ImageSize)
	{
		FSlateBrush Brush;
		Brush.ImageSize = ImageSize;

		if (!Texture)
		{
			Brush.DrawAs = ESlateBrushDrawType::NoDrawType;
			Brush.TintColor = FSlateColor(FLinearColor::Transparent);
			return Brush;
		}

		Brush.SetResourceObject(Texture);
		Brush.DrawAs = ESlateBrushDrawType::Image;
		Brush.TintColor = FSlateColor(FLinearColor::White);

		return Brush;
	}

	FSlateBrush PNMakeColorBrush(const FLinearColor& Color, const FVector2D& ImageSize)
	{
		FSlateBrush Brush;
		Brush.ImageSize = ImageSize;
		Brush.DrawAs = ESlateBrushDrawType::Box;
		Brush.TintColor = FSlateColor(Color);

		return Brush;
	}
}

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

UPNInventoryGridWidget::UPNInventoryGridWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

TSharedRef<SWidget> UPNInventoryGridWidget::RebuildWidget()
{
	if (bBuildNativeGrid && WidgetTree)
	{
		RootSizeBox = nullptr;
		SlotGridPanel = nullptr;

		BuildNativeGridRoot();
		RebuildNativeSlots();
	}

	return Super::RebuildWidget();
}

void UPNInventoryGridWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	RootSizeBox = nullptr;
	SlotGridPanel = nullptr;
}

void UPNInventoryGridWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (!bBuildNativeGrid)
	{
		return;
	}

	if (IsDesignTime() && bPreviewInDesigner)
	{
		BuildPreviewInventoryData();
	}

	RefreshNativeGrid();
}

void UPNInventoryGridWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (bBuildNativeGrid)
	{
		RefreshNativeGrid();
	}
}

void UPNInventoryGridWidget::SetInventoryData(const FPNHUDInventoryPanelData& InInventoryData)
{
	InventoryData = InInventoryData;

	if (bBuildNativeGrid)
	{
		RefreshNativeGrid();
	}

	BP_OnInventoryDataUpdated(InventoryData);
}

void UPNInventoryGridWidget::CopyVisualStyleFrom(const UPNInventoryGridWidget* SourceWidget)
{
	if (!SourceWidget || SourceWidget == this)
	{
		return;
	}

	SlotsPerRow = SourceWidget->SlotsPerRow;
	SlotSize = SourceWidget->SlotSize;

	PreviewActiveSlots = SourceWidget->PreviewActiveSlots;
	PreviewMaxVisualSlots = SourceWidget->PreviewMaxVisualSlots;

	SlotActiveTexture = SourceWidget->SlotActiveTexture;
	SlotHoverTexture = SourceWidget->SlotHoverTexture;
	SlotBlockTexture = SourceWidget->SlotBlockTexture;

	EmptyFallbackColor = SourceWidget->EmptyFallbackColor;
	BlockedFallbackColor = SourceWidget->BlockedFallbackColor;

	if (bBuildNativeGrid)
	{
		RefreshNativeGrid();
	}
}

void UPNInventoryGridWidget::RefreshNativeGrid()
{
	if (!bBuildNativeGrid || !WidgetTree)
	{
		return;
	}

	BuildNativeGridRoot();
	RebuildNativeSlots();
}

const FPNHUDInventoryPanelData& UPNInventoryGridWidget::GetInventoryData() const
{
	return InventoryData;
}

int32 UPNInventoryGridWidget::GetColumns() const
{
	return GetVisualColumns();
}

int32 UPNInventoryGridWidget::GetRows() const
{
	return GetVisualRows();
}

float UPNInventoryGridWidget::GetSlotSize() const
{
	return SlotSize;
}

int32 UPNInventoryGridWidget::GetUnlockedSlotCount() const
{
	const bool bInventoryEnabled =
		InventoryData.bCanReceiveItems ||
		InventoryData.bCanRemoveItems ||
		InventoryData.Items.Num() > 0 ||
		IsDesignTime();

	if (!bInventoryEnabled)
	{
		return 0;
	}

	return FMath::Clamp(InventoryData.SlotCount, 0, GetMaxVisualSlotCount());
}

int32 UPNInventoryGridWidget::GetMaxVisualSlotCount() const
{
	if (InventoryData.MaxVisualSlotCount > 0)
	{
		return FMath::Max(InventoryData.MaxVisualSlotCount, InventoryData.SlotCount);
	}

	if (InventoryData.SlotCount > 0)
	{
		return InventoryData.SlotCount;
	}

	return FMath::Max(PreviewMaxVisualSlots, PreviewActiveSlots);
}

int32 UPNInventoryGridWidget::GetVisualColumns() const
{
	return FMath::Max(1, SlotsPerRow);
}

int32 UPNInventoryGridWidget::GetVisualRows() const
{
	const int32 VisualSlotCount = FMath::Max(1, GetMaxVisualSlotCount());
	const int32 VisualColumns = GetVisualColumns();

	return FMath::Max(1, FMath::CeilToInt(static_cast<float>(VisualSlotCount) / static_cast<float>(VisualColumns)));
}

FVector2D UPNInventoryGridWidget::GetVisualGridSize() const
{
	return FVector2D(
		static_cast<float>(GetVisualColumns()) * SlotSize,
		static_cast<float>(GetVisualRows()) * SlotSize
	);
}

TArray<FPNHUDInventoryGridSlotData> UPNInventoryGridWidget::GetVisualGridSlots() const
{
	TArray<FPNHUDInventoryGridSlotData> Result;

	const int32 VisualSlotCount = GetMaxVisualSlotCount();
	const int32 VisualColumns = GetVisualColumns();
	const int32 UnlockedSlotCount = GetUnlockedSlotCount();

	Result.Reserve(VisualSlotCount);

	for (int32 SlotIndex = 0; SlotIndex < VisualSlotCount; ++SlotIndex)
	{
		FPNHUDInventoryGridSlotData SlotData;

		SlotData.Panel = InventoryData.Panel;
		SlotData.SlotIndex = SlotIndex;
		SlotData.Position.X = SlotIndex % VisualColumns;
		SlotData.Position.Y = SlotIndex / VisualColumns;
		SlotData.bUnlocked = SlotIndex < UnlockedSlotCount;
		SlotData.bOccupied = false;
		SlotData.bRootItemSlot = false;

		Result.Add(SlotData);
	}

	return Result;
}

bool UPNInventoryGridWidget::IsInventoryUnlocked() const
{
	return GetUnlockedSlotCount() > 0;
}

EPNInventorySlotVisualState UPNInventoryGridWidget::GetSlotVisualState(const FPNHUDInventoryGridSlotData& SlotData) const
{
	return SlotData.bUnlocked
		? EPNInventorySlotVisualState::ActiveSlot
		: EPNInventorySlotVisualState::BlockSlot;
}

void UPNInventoryGridWidget::BuildPreviewInventoryData()
{
	InventoryData.Panel = EPNHUDInventoryPanel::MainInventory;
	InventoryData.InventoryType = EPNInventoryType::Inventory;
	InventoryData.bIsActive = true;
	InventoryData.bCanReceiveItems = true;
	InventoryData.bCanRemoveItems = true;
	InventoryData.Columns = GetVisualColumns();
	InventoryData.Rows = GetVisualRows();
	InventoryData.SlotCount = FMath::Max(0, PreviewActiveSlots);
	InventoryData.MaxVisualSlotCount = FMath::Max(InventoryData.SlotCount, PreviewMaxVisualSlots);
	InventoryData.Items.Reset();
}

void UPNInventoryGridWidget::BuildNativeGridRoot()
{
	if (!WidgetTree)
	{
		return;
	}

	const FVector2D GridSize = GetVisualGridSize();

	if (!RootSizeBox)
	{
		RootSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		WidgetTree->RootWidget = RootSizeBox;

		SlotGridPanel = WidgetTree->ConstructWidget<UUniformGridPanel>(UUniformGridPanel::StaticClass());
		RootSizeBox->AddChild(SlotGridPanel);
	}

	RootSizeBox->SetWidthOverride(GridSize.X);
	RootSizeBox->SetHeightOverride(GridSize.Y);
}

void UPNInventoryGridWidget::RebuildNativeSlots()
{
	if (!WidgetTree || !SlotGridPanel)
	{
		return;
	}

	SlotGridPanel->ClearChildren();

	const TArray<FPNHUDInventoryGridSlotData> Slots = GetVisualGridSlots();

	for (const FPNHUDInventoryGridSlotData& SlotData : Slots)
	{
		USizeBox* SlotSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		SlotSizeBox->SetWidthOverride(SlotSize);
		SlotSizeBox->SetHeightOverride(SlotSize);

		UButton* SlotButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
		SlotButton->SetIsEnabled(SlotData.bUnlocked);

		ApplySlotButtonStyle(
			SlotButton,
			GetSlotVisualState(SlotData),
			FVector2D(SlotSize, SlotSize)
		);

		SlotSizeBox->AddChild(SlotButton);

		if (UUniformGridSlot* UniformSlot = SlotGridPanel->AddChildToUniformGrid(SlotSizeBox, SlotData.Position.Y, SlotData.Position.X))
		{
			UniformSlot->SetHorizontalAlignment(HAlign_Fill);
			UniformSlot->SetVerticalAlignment(VAlign_Fill);
		}
	}
}

void UPNInventoryGridWidget::ApplySlotButtonStyle(UButton* TargetButton, EPNInventorySlotVisualState SlotState, const FVector2D& ImageSize) const
{
	if (!TargetButton)
	{
		return;
	}

	UTexture2D* NormalTexture = nullptr;
	UTexture2D* HoveredTexture = nullptr;
	UTexture2D* PressedTexture = nullptr;
	UTexture2D* DisabledTexture = nullptr;

	FLinearColor FallbackColor = EmptyFallbackColor;

	switch (SlotState)
	{
	case EPNInventorySlotVisualState::BlockSlot:
		NormalTexture = SlotBlockTexture.LoadSynchronous();
		HoveredTexture = NormalTexture;
		PressedTexture = NormalTexture;
		DisabledTexture = NormalTexture;
		FallbackColor = BlockedFallbackColor;
		break;

	case EPNInventorySlotVisualState::ActiveSlot:
	default:
		NormalTexture = SlotActiveTexture.LoadSynchronous();
		HoveredTexture = SlotHoverTexture.LoadSynchronous();
		PressedTexture = NormalTexture;
		DisabledTexture = NormalTexture;
		FallbackColor = EmptyFallbackColor;
		break;
	}

	if (!HoveredTexture)
	{
		HoveredTexture = NormalTexture;
	}

	FSlateBrush NormalBrush = NormalTexture
		? PNMakeTextureBrush(NormalTexture, ImageSize)
		: PNMakeColorBrush(FallbackColor, ImageSize);

	FSlateBrush HoveredBrush = HoveredTexture
		? PNMakeTextureBrush(HoveredTexture, ImageSize)
		: NormalBrush;

	FSlateBrush PressedBrush = PressedTexture
		? PNMakeTextureBrush(PressedTexture, ImageSize)
		: NormalBrush;

	FSlateBrush DisabledBrush = DisabledTexture
		? PNMakeTextureBrush(DisabledTexture, ImageSize)
		: NormalBrush;

	FButtonStyle ButtonStyle;
	ButtonStyle.SetNormal(NormalBrush);
	ButtonStyle.SetHovered(HoveredBrush);
	ButtonStyle.SetPressed(PressedBrush);
	ButtonStyle.SetDisabled(DisabledBrush);

	ButtonStyle.SetNormalPadding(FMargin(0.0f));
	ButtonStyle.SetPressedPadding(FMargin(0.0f));

	TargetButton->SetStyle(ButtonStyle);
	TargetButton->SetColorAndOpacity(FLinearColor::White);
	TargetButton->SetBackgroundColor(FLinearColor::White);
}

void UPNInventoryGridSlotWidget::SetSlotData(const FPNHUDInventoryGridSlotData& InSlotData)
{
	SlotData = InSlotData;
	BP_OnSlotDataUpdated(SlotData);
}

const FPNHUDInventoryGridSlotData& UPNInventoryGridSlotWidget::GetSlotData() const
{
	return SlotData;
}

bool UPNInventoryGridSlotWidget::IsUnlocked() const
{
	return SlotData.bUnlocked;
}

bool UPNInventoryGridSlotWidget::IsOccupied() const
{
	return SlotData.bOccupied;
}

bool UPNInventoryGridSlotWidget::IsRootItemSlot() const
{
	return SlotData.bRootItemSlot;
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

UPNEquipmentLayoutWidget::UPNEquipmentLayoutWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DisplayTitle = FText::FromString(TEXT("Equipment"));
}

TSharedRef<SWidget> UPNEquipmentLayoutWidget::RebuildWidget()
{
	if (bBuildNativeEquipmentLayout && WidgetTree)
	{
		EquipmentRootSizeBox = nullptr;
		EquipmentRootBorder = nullptr;
		EquipmentRootVerticalBox = nullptr;
		EquipmentHeaderBorder = nullptr;
		EquipmentHeaderText = nullptr;
		EquipmentMainBox = nullptr;
		WeaponColumnBox = nullptr;
		CenterColumnBox = nullptr;
		GearColumnBox = nullptr;
		HelmetInternalGrid = nullptr;
		ArmorInternalGrid = nullptr;

		BuildNativeEquipmentRoot();
		RebuildNativeEquipmentSlots();
	}

	return Super::RebuildWidget();
}

void UPNEquipmentLayoutWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	EquipmentRootSizeBox = nullptr;
	EquipmentRootBorder = nullptr;
	EquipmentRootVerticalBox = nullptr;
	EquipmentHeaderBorder = nullptr;
	EquipmentHeaderText = nullptr;
	EquipmentMainBox = nullptr;
	WeaponColumnBox = nullptr;
	CenterColumnBox = nullptr;
	GearColumnBox = nullptr;
	HelmetInternalGrid = nullptr;
	ArmorInternalGrid = nullptr;
}

void UPNEquipmentLayoutWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (!bBuildNativeEquipmentLayout)
	{
		return;
	}

	if (IsDesignTime() && bPreviewInDesigner)
	{
		BuildPreviewEquipmentData();
	}

	RefreshNativeEquipmentLayout();
}

void UPNEquipmentLayoutWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (bBuildNativeEquipmentLayout)
	{
		RefreshNativeEquipmentLayout();
	}
}

void UPNEquipmentLayoutWidget::SetEquipmentData(const FPNHUDEquipmentData& InEquipmentData)
{
	EquipmentData = InEquipmentData;

	if (bBuildNativeEquipmentLayout)
	{
		RefreshNativeEquipmentLayout();
	}

	BP_OnEquipmentDataUpdated(EquipmentData);
}

void UPNEquipmentLayoutWidget::RefreshNativeEquipmentLayout()
{
	if (!bBuildNativeEquipmentLayout || !WidgetTree)
	{
		return;
	}

	BuildNativeEquipmentRoot();
	RebuildNativeEquipmentSlots();
}

const FPNHUDEquipmentData& UPNEquipmentLayoutWidget::GetEquipmentData() const
{
	return EquipmentData;
}

FPNHUDEquipmentSlotData UPNEquipmentLayoutWidget::GetEquipmentSlotData(EPNEquipmentSlot EquipmentSlot) const
{
	FPNHUDEquipmentSlotData SlotData;
	FindEquipmentSlotData(EquipmentSlot, SlotData);
	return SlotData;
}

FPNHUDInternalEquipmentSlotData UPNEquipmentLayoutWidget::GetInternalEquipmentSlotData(EPNEquipmentInternalContainer Container, int32 SlotIndex) const
{
	FPNHUDInternalEquipmentSlotData SlotData;
	FindInternalEquipmentSlotData(Container, SlotIndex, SlotData);
	return SlotData;
}

bool UPNEquipmentLayoutWidget::IsEquipmentSlotOccupied(EPNEquipmentSlot EquipmentSlot) const
{
	FPNHUDEquipmentSlotData SlotData;
	return FindEquipmentSlotData(EquipmentSlot, SlotData) && SlotData.bOccupied;
}

bool UPNEquipmentLayoutWidget::IsInternalEquipmentSlotUnlocked(EPNEquipmentInternalContainer Container, int32 SlotIndex) const
{
	FPNHUDInternalEquipmentSlotData SlotData;
	return FindInternalEquipmentSlotData(Container, SlotIndex, SlotData) && SlotData.bUnlocked;
}

bool UPNEquipmentLayoutWidget::IsInternalEquipmentSlotOccupied(EPNEquipmentInternalContainer Container, int32 SlotIndex) const
{
	FPNHUDInternalEquipmentSlotData SlotData;
	return FindInternalEquipmentSlotData(Container, SlotIndex, SlotData) && SlotData.bOccupied;
}

UPNItemDataAsset* UPNEquipmentLayoutWidget::GetEquipmentSlotItemData(EPNEquipmentSlot EquipmentSlot) const
{
	FPNHUDEquipmentSlotData SlotData;
	return FindEquipmentSlotData(EquipmentSlot, SlotData) ? SlotData.Item.ItemData : nullptr;
}

UPNItemDataAsset* UPNEquipmentLayoutWidget::GetInternalEquipmentSlotItemData(EPNEquipmentInternalContainer Container, int32 SlotIndex) const
{
	FPNHUDInternalEquipmentSlotData SlotData;
	return FindInternalEquipmentSlotData(Container, SlotIndex, SlotData) ? SlotData.Item.ItemData : nullptr;
}

FText UPNEquipmentLayoutWidget::GetEquipmentSlotLabel(EPNEquipmentSlot EquipmentSlot) const
{
	switch (EquipmentSlot)
	{
	case EPNEquipmentSlot::PrimaryWeapon1:
		return FText::FromString(TEXT("PRIMARY"));

	case EPNEquipmentSlot::PrimaryWeapon2:
		return FText::FromString(TEXT("SECONDARY"));

	case EPNEquipmentSlot::Sidearm:
		return FText::FromString(TEXT("PISTOL"));

	case EPNEquipmentSlot::Knife:
		return FText::FromString(TEXT("MELEE"));

	case EPNEquipmentSlot::Helmet:
		return FText::FromString(TEXT("HELMET"));

	case EPNEquipmentSlot::Armor:
		return FText::FromString(TEXT("ARMOR"));

	case EPNEquipmentSlot::Gloves:
		return FText::FromString(TEXT("GLOVES"));

	case EPNEquipmentSlot::Backpack:
		return FText::FromString(TEXT("BACKPACK"));

	default:
		return FText::FromString(TEXT("SLOT"));
	}
}

FText UPNEquipmentLayoutWidget::GetEquipmentSlotHotkeyText(EPNEquipmentSlot EquipmentSlot) const
{
	switch (EquipmentSlot)
	{
	case EPNEquipmentSlot::PrimaryWeapon1:
		return FText::FromString(TEXT("1"));

	case EPNEquipmentSlot::PrimaryWeapon2:
		return FText::FromString(TEXT("2"));

	case EPNEquipmentSlot::Sidearm:
		return FText::FromString(TEXT("3"));

	case EPNEquipmentSlot::Knife:
		return FText::FromString(TEXT("4"));

	default:
		return FText::GetEmpty();
	}
}

void UPNEquipmentLayoutWidget::BuildPreviewEquipmentData()
{
	EquipmentData.EquipmentSlots.Reset();
	EquipmentData.HelmetInternalSlots.Reset();
	EquipmentData.ArmorInternalSlots.Reset();

	const EPNEquipmentSlot PreviewSlots[] =
	{
		EPNEquipmentSlot::PrimaryWeapon1,
		EPNEquipmentSlot::PrimaryWeapon2,
		EPNEquipmentSlot::Sidearm,
		EPNEquipmentSlot::Knife,
		EPNEquipmentSlot::Helmet,
		EPNEquipmentSlot::Armor,
		EPNEquipmentSlot::Gloves,
		EPNEquipmentSlot::Backpack
	};

	for (EPNEquipmentSlot EquipmentSlot : PreviewSlots)
	{
		FPNHUDEquipmentSlotData SlotData;
		SlotData.Slot = EquipmentSlot;
		SlotData.bOccupied = false;
		EquipmentData.EquipmentSlots.Add(SlotData);
	}

	const int32 SafeHelmetUnlockedSlots = FMath::Clamp(PreviewHelmetUnlockedSlots, 0, 4);
	const int32 SafeArmorUnlockedSlots = FMath::Clamp(PreviewArmorUnlockedSlots, 0, 4);

	for (int32 Index = 0; Index < 4; ++Index)
	{
		FPNHUDInternalEquipmentSlotData HelmetSlot;
		HelmetSlot.Container = EPNEquipmentInternalContainer::Helmet;
		HelmetSlot.SlotIndex = Index;
		HelmetSlot.bUnlocked = Index < SafeHelmetUnlockedSlots;
		HelmetSlot.bOccupied = false;
		EquipmentData.HelmetInternalSlots.Add(HelmetSlot);

		FPNHUDInternalEquipmentSlotData ArmorSlot;
		ArmorSlot.Container = EPNEquipmentInternalContainer::Armor;
		ArmorSlot.SlotIndex = Index;
		ArmorSlot.bUnlocked = Index < SafeArmorUnlockedSlots;
		ArmorSlot.bOccupied = false;
		EquipmentData.ArmorInternalSlots.Add(ArmorSlot);
	}
}

void UPNEquipmentLayoutWidget::BuildNativeEquipmentRoot()
{
	if (!WidgetTree)
	{
		return;
	}

	if (!EquipmentRootSizeBox)
	{
		EquipmentRootSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		WidgetTree->RootWidget = EquipmentRootSizeBox;

		EquipmentRootBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
		EquipmentRootBorder->SetPadding(FMargin(0.0f));
		EquipmentRootSizeBox->AddChild(EquipmentRootBorder);

		EquipmentRootVerticalBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		EquipmentRootBorder->SetContent(EquipmentRootVerticalBox);

		EquipmentHeaderBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
		EquipmentHeaderBorder->SetPadding(FMargin(0.0f));

		if (UVerticalBoxSlot* HeaderSlot = EquipmentRootVerticalBox->AddChildToVerticalBox(EquipmentHeaderBorder))
		{
			HeaderSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, HeaderBottomSpacing));
			HeaderSlot->SetHorizontalAlignment(HAlign_Fill);
			HeaderSlot->SetVerticalAlignment(VAlign_Fill);
		}

		EquipmentHeaderText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		EquipmentHeaderText->SetJustification(ETextJustify::Center);
		EquipmentHeaderText->SetColorAndOpacity(FSlateColor(TextColor));
		EquipmentHeaderText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), TitleFontSize));
		EquipmentHeaderBorder->SetContent(EquipmentHeaderText);

		EquipmentMainBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());

		if (UVerticalBoxSlot* MainSlot = EquipmentRootVerticalBox->AddChildToVerticalBox(EquipmentMainBox))
		{
			MainSlot->SetPadding(FMargin(0.0f));
			MainSlot->SetHorizontalAlignment(HAlign_Center);
			MainSlot->SetVerticalAlignment(VAlign_Top);
		}

		WeaponColumnBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		CenterColumnBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		GearColumnBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());

		if (UHorizontalBoxSlot* WeaponColumnSlot = EquipmentMainBox->AddChildToHorizontalBox(WeaponColumnBox))
		{
			WeaponColumnSlot->SetPadding(FMargin(0.0f, 0.0f, ColumnSpacing, 0.0f));
			WeaponColumnSlot->SetHorizontalAlignment(HAlign_Center);
			WeaponColumnSlot->SetVerticalAlignment(VAlign_Top);
		}

		if (UHorizontalBoxSlot* CenterColumnSlot = EquipmentMainBox->AddChildToHorizontalBox(CenterColumnBox))
		{
			CenterColumnSlot->SetPadding(FMargin(0.0f));
			CenterColumnSlot->SetHorizontalAlignment(HAlign_Center);
			CenterColumnSlot->SetVerticalAlignment(VAlign_Top);
		}

		if (UHorizontalBoxSlot* GearColumnSlot = EquipmentMainBox->AddChildToHorizontalBox(GearColumnBox))
		{
			GearColumnSlot->SetPadding(FMargin(ColumnSpacing, 0.0f, 0.0f, 0.0f));
			GearColumnSlot->SetHorizontalAlignment(HAlign_Center);
			GearColumnSlot->SetVerticalAlignment(VAlign_Top);
		}
	}

	EquipmentRootSizeBox->SetWidthOverride(PanelWidth);
	EquipmentRootSizeBox->SetHeightOverride(PanelHeight);

	if (EquipmentRootBorder)
	{
		ApplyEquipmentTextureToBorderImageOnly(
			EquipmentRootBorder,
			RootBackgroundTexture.LoadSynchronous(),
			FVector2D(PanelWidth, PanelHeight)
		);
	}

	if (EquipmentHeaderBorder)
	{
		ApplyEquipmentTextureToBorderImageOnly(
			EquipmentHeaderBorder,
			HeaderBackgroundTexture.LoadSynchronous(),
			FVector2D(PanelWidth, HeaderHeight)
		);
	}

	if (EquipmentHeaderText)
	{
		EquipmentHeaderText->SetText(FText::FromString(DisplayTitle.ToString().ToUpper()));
		EquipmentHeaderText->SetColorAndOpacity(FSlateColor(TextColor));
		EquipmentHeaderText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), TitleFontSize));
	}
}

void UPNEquipmentLayoutWidget::RebuildNativeEquipmentSlots()
{
	if (!WidgetTree || !WeaponColumnBox || !CenterColumnBox || !GearColumnBox)
	{
		return;
	}

	WeaponColumnBox->ClearChildren();
	CenterColumnBox->ClearChildren();
	GearColumnBox->ClearChildren();

	AddEquipmentSlotToBox(EPNEquipmentSlot::PrimaryWeapon1, WeaponColumnBox);
	AddEquipmentSlotToBox(EPNEquipmentSlot::PrimaryWeapon2, WeaponColumnBox);
	AddEquipmentSlotToBox(EPNEquipmentSlot::Sidearm, WeaponColumnBox);
	AddEquipmentSlotToBox(EPNEquipmentSlot::Knife, WeaponColumnBox);

	AddSectionTitle(CenterColumnBox, FText::FromString(TEXT("HELMET INTERNAL")));

	HelmetInternalGrid = WidgetTree->ConstructWidget<UUniformGridPanel>(UUniformGridPanel::StaticClass());

	if (UVerticalBoxSlot* HelmetGridSlot = CenterColumnBox->AddChildToVerticalBox(HelmetInternalGrid))
	{
		HelmetGridSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, SlotSpacing * 2.0f));
		HelmetGridSlot->SetHorizontalAlignment(HAlign_Center);
		HelmetGridSlot->SetVerticalAlignment(VAlign_Top);
	}

	for (int32 Index = 0; Index < 4; ++Index)
	{
		AddInternalSlotToGrid(EPNEquipmentInternalContainer::Helmet, Index, HelmetInternalGrid);
	}

	AddSectionTitle(CenterColumnBox, FText::FromString(TEXT("ARMOR INTERNAL")));

	ArmorInternalGrid = WidgetTree->ConstructWidget<UUniformGridPanel>(UUniformGridPanel::StaticClass());

	if (UVerticalBoxSlot* ArmorGridSlot = CenterColumnBox->AddChildToVerticalBox(ArmorInternalGrid))
	{
		ArmorGridSlot->SetPadding(FMargin(0.0f));
		ArmorGridSlot->SetHorizontalAlignment(HAlign_Center);
		ArmorGridSlot->SetVerticalAlignment(VAlign_Top);
	}

	for (int32 Index = 0; Index < 4; ++Index)
	{
		AddInternalSlotToGrid(EPNEquipmentInternalContainer::Armor, Index, ArmorInternalGrid);
	}

	AddEquipmentSlotToBox(EPNEquipmentSlot::Helmet, GearColumnBox);
	AddEquipmentSlotToBox(EPNEquipmentSlot::Armor, GearColumnBox);
	AddEquipmentSlotToBox(EPNEquipmentSlot::Gloves, GearColumnBox);
	AddEquipmentSlotToBox(EPNEquipmentSlot::Backpack, GearColumnBox);
}

void UPNEquipmentLayoutWidget::AddEquipmentSlotToBox(EPNEquipmentSlot EquipmentSlot, UVerticalBox* TargetBox)
{
	if (!WidgetTree || !TargetBox)
	{
		return;
	}

	FPNHUDEquipmentSlotData SlotData;
	FindEquipmentSlotData(EquipmentSlot, SlotData);

	USizeBox* SlotSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	SlotSizeBox->SetWidthOverride(SlotSize);
	SlotSizeBox->SetHeightOverride(SlotSize);

	UButton* SlotButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
	SlotButton->SetIsEnabled(true);

	ApplyEquipmentButtonStyle(
		SlotButton,
		GetEquipmentSlotVisualState(SlotData),
		false,
		FVector2D(SlotSize, SlotSize)
	);

	UOverlay* SlotOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
	SlotOverlay->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	UTexture2D* IconTexture = nullptr;

	if (SlotData.Item.bValid && SlotData.Item.ItemData && !SlotData.Item.ItemData->Visual.Icon.IsNull())
	{
		IconTexture = SlotData.Item.ItemData->Visual.Icon.LoadSynchronous();
	}
	else
	{
		IconTexture = GetEquipmentSlotPlaceholderTexture(EquipmentSlot);
	}

	if (IconTexture)
	{
		UImage* IconImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
		IconImage->SetVisibility(ESlateVisibility::HitTestInvisible);

		const FVector2D IconSize(SlotSize * 0.76f, SlotSize * 0.76f);
		ApplyEquipmentTextureToImage(IconImage, IconTexture, IconSize);

		if (UOverlaySlot* IconOverlaySlot = SlotOverlay->AddChildToOverlay(IconImage))
		{
			IconOverlaySlot->SetHorizontalAlignment(HAlign_Center);
			IconOverlaySlot->SetVerticalAlignment(VAlign_Center);
		}
	}

	const FText HotkeyText = GetEquipmentSlotHotkeyText(EquipmentSlot);

	if (!HotkeyText.IsEmpty())
	{
		UTextBlock* HotkeyBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		HotkeyBlock->SetText(HotkeyText);
		HotkeyBlock->SetColorAndOpacity(FSlateColor(TextColor));
		HotkeyBlock->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), HotkeyFontSize));
		HotkeyBlock->SetJustification(ETextJustify::Center);
		HotkeyBlock->SetVisibility(ESlateVisibility::HitTestInvisible);

		if (UOverlaySlot* HotkeyOverlaySlot = SlotOverlay->AddChildToOverlay(HotkeyBlock))
		{
			HotkeyOverlaySlot->SetHorizontalAlignment(HAlign_Left);
			HotkeyOverlaySlot->SetVerticalAlignment(VAlign_Top);
			HotkeyOverlaySlot->SetPadding(FMargin(5.0f, 2.0f, 0.0f, 0.0f));
		}
	}

	UTextBlock* LabelBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	LabelBlock->SetText(GetEquipmentSlotLabel(EquipmentSlot));
	LabelBlock->SetColorAndOpacity(FSlateColor(TextColor));
	LabelBlock->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), LabelFontSize));
	LabelBlock->SetJustification(ETextJustify::Center);
	LabelBlock->SetVisibility(ESlateVisibility::HitTestInvisible);

	if (UOverlaySlot* LabelOverlaySlot = SlotOverlay->AddChildToOverlay(LabelBlock))
	{
		LabelOverlaySlot->SetHorizontalAlignment(HAlign_Center);
		LabelOverlaySlot->SetVerticalAlignment(VAlign_Bottom);
		LabelOverlaySlot->SetPadding(FMargin(2.0f, 0.0f, 2.0f, 3.0f));
	}

	SlotButton->AddChild(SlotOverlay);
	SlotSizeBox->AddChild(SlotButton);

	if (UVerticalBoxSlot* BoxSlot = TargetBox->AddChildToVerticalBox(SlotSizeBox))
	{
		BoxSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, SlotSpacing));
		BoxSlot->SetHorizontalAlignment(HAlign_Center);
		BoxSlot->SetVerticalAlignment(VAlign_Top);
	}
}

void UPNEquipmentLayoutWidget::AddInternalSlotToGrid(EPNEquipmentInternalContainer Container, int32 SlotIndex, UUniformGridPanel* TargetGrid)
{
	if (!WidgetTree || !TargetGrid)
	{
		return;
	}

	FPNHUDInternalEquipmentSlotData SlotData;
	FindInternalEquipmentSlotData(Container, SlotIndex, SlotData);

	USizeBox* SlotSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	SlotSizeBox->SetWidthOverride(InternalSlotSize);
	SlotSizeBox->SetHeightOverride(InternalSlotSize);

	UButton* SlotButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
	SlotButton->SetIsEnabled(SlotData.bUnlocked);

	ApplyEquipmentButtonStyle(
		SlotButton,
		GetInternalSlotVisualState(SlotData),
		true,
		FVector2D(InternalSlotSize, InternalSlotSize)
	);

	UOverlay* SlotOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
	SlotOverlay->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	UTexture2D* IconTexture = nullptr;

	if (SlotData.Item.bValid && SlotData.Item.ItemData && !SlotData.Item.ItemData->Visual.Icon.IsNull())
	{
		IconTexture = SlotData.Item.ItemData->Visual.Icon.LoadSynchronous();
	}
	else if (SlotData.bUnlocked)
	{
		IconTexture = GetInternalSlotPlaceholderTexture(Container);
	}

	if (IconTexture)
	{
		UImage* IconImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
		IconImage->SetVisibility(ESlateVisibility::HitTestInvisible);

		const FVector2D IconSize(InternalSlotSize * 0.72f, InternalSlotSize * 0.72f);
		ApplyEquipmentTextureToImage(IconImage, IconTexture, IconSize);

		if (UOverlaySlot* IconSlot = SlotOverlay->AddChildToOverlay(IconImage))
		{
			IconSlot->SetHorizontalAlignment(HAlign_Center);
			IconSlot->SetVerticalAlignment(VAlign_Center);
		}
	}

	if (!SlotData.bUnlocked)
	{
		UTextBlock* LockedText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		LockedText->SetText(FText::FromString(TEXT("LOCK")));
		LockedText->SetColorAndOpacity(FSlateColor(LockedTextColor));
		LockedText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), LabelFontSize));
		LockedText->SetJustification(ETextJustify::Center);
		LockedText->SetVisibility(ESlateVisibility::HitTestInvisible);

		if (UOverlaySlot* LockedSlot = SlotOverlay->AddChildToOverlay(LockedText))
		{
			LockedSlot->SetHorizontalAlignment(HAlign_Center);
			LockedSlot->SetVerticalAlignment(VAlign_Center);
		}
	}

	SlotButton->AddChild(SlotOverlay);
	SlotSizeBox->AddChild(SlotButton);

	const int32 SafeColumns = FMath::Max(1, InternalGridColumns);
	const int32 Row = SlotIndex / SafeColumns;
	const int32 Column = SlotIndex % SafeColumns;

	if (UUniformGridSlot* GridSlot = TargetGrid->AddChildToUniformGrid(SlotSizeBox, Row, Column))
	{
		GridSlot->SetHorizontalAlignment(HAlign_Center);
		GridSlot->SetVerticalAlignment(VAlign_Center);
	}
}

void UPNEquipmentLayoutWidget::AddSectionTitle(UVerticalBox* TargetBox, const FText& TitleText)
{
	if (!WidgetTree || !TargetBox)
	{
		return;
	}

	UTextBlock* TitleBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	TitleBlock->SetText(TitleText);
	TitleBlock->SetColorAndOpacity(FSlateColor(TextColor));
	TitleBlock->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), LabelFontSize));
	TitleBlock->SetJustification(ETextJustify::Center);

	if (UVerticalBoxSlot* TitleSlot = TargetBox->AddChildToVerticalBox(TitleBlock))
	{
		TitleSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 4.0f));
		TitleSlot->SetHorizontalAlignment(HAlign_Center);
		TitleSlot->SetVerticalAlignment(VAlign_Top);
	}
}

void UPNEquipmentLayoutWidget::ApplyEquipmentButtonStyle(UButton* TargetButton, EPNInventorySlotVisualState SlotState, bool bInternalSlot, const FVector2D& ImageSize) const
{
	if (!TargetButton)
	{
		return;
	}

	UTexture2D* NormalTexture = nullptr;
	UTexture2D* HoveredTexture = nullptr;
	UTexture2D* PressedTexture = nullptr;
	UTexture2D* DisabledTexture = nullptr;

	if (bInternalSlot)
	{
		switch (SlotState)
		{
		case EPNInventorySlotVisualState::ActiveSlot:
			NormalTexture = InternalSlotActiveTexture.LoadSynchronous();
			HoveredTexture = InternalSlotHoverTexture.LoadSynchronous();
			PressedTexture = NormalTexture;
			DisabledTexture = NormalTexture;
			break;

		case EPNInventorySlotVisualState::BlockSlot:
			NormalTexture = InternalSlotBlockTexture.LoadSynchronous();
			HoveredTexture = NormalTexture;
			PressedTexture = NormalTexture;
			DisabledTexture = NormalTexture;
			break;

		case EPNInventorySlotVisualState::NoneSlot:
		default:
			NormalTexture = InternalSlotNoneTexture.LoadSynchronous();
			HoveredTexture = InternalSlotHoverTexture.LoadSynchronous();
			PressedTexture = NormalTexture;
			DisabledTexture = NormalTexture;
			break;
		}
	}
	else
	{
		switch (SlotState)
		{
		case EPNInventorySlotVisualState::ActiveSlot:
			NormalTexture = EquipmentSlotActiveTexture.LoadSynchronous();
			HoveredTexture = EquipmentSlotHoverTexture.LoadSynchronous();
			PressedTexture = NormalTexture;
			DisabledTexture = NormalTexture;
			break;

		case EPNInventorySlotVisualState::BlockSlot:
			NormalTexture = EquipmentSlotBlockTexture.LoadSynchronous();
			HoveredTexture = NormalTexture;
			PressedTexture = NormalTexture;
			DisabledTexture = NormalTexture;
			break;

		case EPNInventorySlotVisualState::NoneSlot:
		default:
			NormalTexture = EquipmentSlotNoneTexture.LoadSynchronous();
			HoveredTexture = EquipmentSlotHoverTexture.LoadSynchronous();
			PressedTexture = NormalTexture;
			DisabledTexture = NormalTexture;
			break;
		}
	}

	if (!HoveredTexture)
	{
		HoveredTexture = NormalTexture;
	}

	FButtonStyle ButtonStyle;
	ButtonStyle.SetNormal(PNMakeTextureBrush(NormalTexture, ImageSize));
	ButtonStyle.SetHovered(PNMakeTextureBrush(HoveredTexture, ImageSize));
	ButtonStyle.SetPressed(PNMakeTextureBrush(PressedTexture, ImageSize));
	ButtonStyle.SetDisabled(PNMakeTextureBrush(DisabledTexture, ImageSize));

	ButtonStyle.SetNormalPadding(FMargin(0.0f));
	ButtonStyle.SetPressedPadding(FMargin(0.0f));

	TargetButton->SetStyle(ButtonStyle);
	TargetButton->SetColorAndOpacity(FLinearColor::White);
	TargetButton->SetBackgroundColor(FLinearColor::White);
}

void UPNEquipmentLayoutWidget::ApplyEquipmentTextureToImage(UImage* TargetImage, UTexture2D* Texture, const FVector2D& ImageSize) const
{
	if (!TargetImage)
	{
		return;
	}

	if (!Texture)
	{
		TargetImage->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	FSlateBrush Brush;
	Brush.SetResourceObject(Texture);
	Brush.SetImageSize(ImageSize);

	TargetImage->SetBrush(Brush);
	TargetImage->SetColorAndOpacity(FLinearColor::White);
	TargetImage->SetVisibility(ESlateVisibility::Visible);
}

void UPNEquipmentLayoutWidget::ApplyEquipmentTextureToBorderImageOnly(UBorder* TargetBorder, UTexture2D* Texture, const FVector2D& ImageSize) const
{
	if (!TargetBorder)
	{
		return;
	}

	TargetBorder->SetBrush(PNMakeTextureBrush(Texture, ImageSize));
	TargetBorder->SetBrushColor(FLinearColor::White);
}

bool UPNEquipmentLayoutWidget::FindEquipmentSlotData(EPNEquipmentSlot EquipmentSlot, FPNHUDEquipmentSlotData& OutSlotData) const
{
	OutSlotData = FPNHUDEquipmentSlotData();
	OutSlotData.Slot = EquipmentSlot;

	for (const FPNHUDEquipmentSlotData& SlotData : EquipmentData.EquipmentSlots)
	{
		if (SlotData.Slot == EquipmentSlot)
		{
			OutSlotData = SlotData;
			return true;
		}
	}

	return false;
}

bool UPNEquipmentLayoutWidget::FindInternalEquipmentSlotData(EPNEquipmentInternalContainer Container, int32 SlotIndex, FPNHUDInternalEquipmentSlotData& OutSlotData) const
{
	OutSlotData = FPNHUDInternalEquipmentSlotData();
	OutSlotData.Container = Container;
	OutSlotData.SlotIndex = SlotIndex;

	const TArray<FPNHUDInternalEquipmentSlotData>* SourceSlots = nullptr;

	switch (Container)
	{
	case EPNEquipmentInternalContainer::Helmet:
		SourceSlots = &EquipmentData.HelmetInternalSlots;
		break;

	case EPNEquipmentInternalContainer::Armor:
		SourceSlots = &EquipmentData.ArmorInternalSlots;
		break;

	default:
		return false;
	}

	for (const FPNHUDInternalEquipmentSlotData& SlotData : *SourceSlots)
	{
		if (SlotData.Container == Container && SlotData.SlotIndex == SlotIndex)
		{
			OutSlotData = SlotData;
			return true;
		}
	}

	return false;
}

EPNInventorySlotVisualState UPNEquipmentLayoutWidget::GetEquipmentSlotVisualState(const FPNHUDEquipmentSlotData& SlotData) const
{
	if (SlotData.bOccupied)
	{
		return EPNInventorySlotVisualState::ActiveSlot;
	}

	return EPNInventorySlotVisualState::NoneSlot;
}

EPNInventorySlotVisualState UPNEquipmentLayoutWidget::GetInternalSlotVisualState(const FPNHUDInternalEquipmentSlotData& SlotData) const
{
	if (!SlotData.bUnlocked)
	{
		return EPNInventorySlotVisualState::BlockSlot;
	}

	if (SlotData.bOccupied)
	{
		return EPNInventorySlotVisualState::ActiveSlot;
	}

	return EPNInventorySlotVisualState::NoneSlot;
}

UTexture2D* UPNEquipmentLayoutWidget::GetEquipmentSlotPlaceholderTexture(EPNEquipmentSlot EquipmentSlot) const
{
	switch (EquipmentSlot)
	{
	case EPNEquipmentSlot::PrimaryWeapon1:
		return PrimaryWeapon1Icon.LoadSynchronous();

	case EPNEquipmentSlot::PrimaryWeapon2:
		return PrimaryWeapon2Icon.LoadSynchronous();

	case EPNEquipmentSlot::Sidearm:
		return SidearmIcon.LoadSynchronous();

	case EPNEquipmentSlot::Knife:
		return KnifeIcon.LoadSynchronous();

	case EPNEquipmentSlot::Helmet:
		return HelmetIcon.LoadSynchronous();

	case EPNEquipmentSlot::Armor:
		return ArmorIcon.LoadSynchronous();

	case EPNEquipmentSlot::Gloves:
		return GlovesIcon.LoadSynchronous();

	case EPNEquipmentSlot::Backpack:
		return BackpackIcon.LoadSynchronous();

	default:
		return nullptr;
	}
}

UTexture2D* UPNEquipmentLayoutWidget::GetInternalSlotPlaceholderTexture(EPNEquipmentInternalContainer Container) const
{
	switch (Container)
	{
	case EPNEquipmentInternalContainer::Helmet:
		return HelmetInternalIcon.LoadSynchronous();

	case EPNEquipmentInternalContainer::Armor:
		return ArmorInternalIcon.LoadSynchronous();

	default:
		return nullptr;
	}
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

FPNHUDValuePercent UPNPlayerStatsLayoutWidget::GetStatData(EPNHUDCharacterStatType StatType) const
{
	switch (StatType)
	{
	case EPNHUDCharacterStatType::Health:
		return StatsData.Health;

	case EPNHUDCharacterStatType::Stamina:
		return StatsData.Stamina;

	case EPNHUDCharacterStatType::Hunger:
		return StatsData.Hunger;

	case EPNHUDCharacterStatType::Thirst:
		return StatsData.Thirst;

	case EPNHUDCharacterStatType::Weight:
		return StatsData.Weight;

	case EPNHUDCharacterStatType::Radiation:
		return StatsData.Radiation;

	case EPNHUDCharacterStatType::Toxicity:
		return StatsData.Toxicity;

	case EPNHUDCharacterStatType::Psy:
		return StatsData.Psy;

	case EPNHUDCharacterStatType::Bleeding:
		return StatsData.Bleeding;

	case EPNHUDCharacterStatType::Wounds:
		return StatsData.Wounds;

	case EPNHUDCharacterStatType::Burn:
		return StatsData.Burn;

	case EPNHUDCharacterStatType::ChemicalBurn:
		return StatsData.ChemicalBurn;

	case EPNHUDCharacterStatType::ElectricShock:
		return StatsData.ElectricShock;

	default:
		return FPNHUDValuePercent();
	}
}

float UPNPlayerStatsLayoutWidget::GetStatPercent(EPNHUDCharacterStatType StatType) const
{
	return FMath::Clamp(GetStatData(StatType).Percent, 0.0f, 1.0f);
}

FText UPNPlayerStatsLayoutWidget::GetStatValueText(EPNHUDCharacterStatType StatType) const
{
	const FPNHUDValuePercent Value = GetStatData(StatType);

	if (StatType == EPNHUDCharacterStatType::Weight)
	{
		return FText::FromString(
			FString::Printf(TEXT("%.1f/%.1f kg"), Value.Current, Value.Max)
		);
	}

	return FText::FromString(
		FString::Printf(TEXT("%.0f/%.0f"), Value.Current, Value.Max)
	);
}

float UPNPlayerStatsLayoutWidget::GetHealthPercent() const
{
	return GetStatPercent(EPNHUDCharacterStatType::Health);
}

FText UPNPlayerStatsLayoutWidget::GetHealthValueText() const
{
	return GetStatValueText(EPNHUDCharacterStatType::Health);
}

float UPNPlayerStatsLayoutWidget::GetStaminaPercent() const
{
	return GetStatPercent(EPNHUDCharacterStatType::Stamina);
}

FText UPNPlayerStatsLayoutWidget::GetStaminaValueText() const
{
	return GetStatValueText(EPNHUDCharacterStatType::Stamina);
}

float UPNPlayerStatsLayoutWidget::GetHungerPercent() const
{
	return GetStatPercent(EPNHUDCharacterStatType::Hunger);
}

FText UPNPlayerStatsLayoutWidget::GetHungerValueText() const
{
	return GetStatValueText(EPNHUDCharacterStatType::Hunger);
}

float UPNPlayerStatsLayoutWidget::GetThirstPercent() const
{
	return GetStatPercent(EPNHUDCharacterStatType::Thirst);
}

FText UPNPlayerStatsLayoutWidget::GetThirstValueText() const
{
	return GetStatValueText(EPNHUDCharacterStatType::Thirst);
}

float UPNPlayerStatsLayoutWidget::GetWeightPercent() const
{
	return GetStatPercent(EPNHUDCharacterStatType::Weight);
}

FText UPNPlayerStatsLayoutWidget::GetWeightValueText() const
{
	return GetStatValueText(EPNHUDCharacterStatType::Weight);
}

float UPNPlayerStatsLayoutWidget::GetRadiationPercent() const
{
	return GetStatPercent(EPNHUDCharacterStatType::Radiation);
}

FText UPNPlayerStatsLayoutWidget::GetRadiationValueText() const
{
	return GetStatValueText(EPNHUDCharacterStatType::Radiation);
}

float UPNPlayerStatsLayoutWidget::GetChemPercent() const
{
	return GetStatPercent(EPNHUDCharacterStatType::Toxicity);
}

FText UPNPlayerStatsLayoutWidget::GetChemValueText() const
{
	return GetStatValueText(EPNHUDCharacterStatType::Toxicity);
}

float UPNPlayerStatsLayoutWidget::GetPsyPercent() const
{
	return GetStatPercent(EPNHUDCharacterStatType::Psy);
}

FText UPNPlayerStatsLayoutWidget::GetPsyValueText() const
{
	return GetStatValueText(EPNHUDCharacterStatType::Psy);
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