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
	DisplayTitle = FText::FromString(TEXT("Inventory"));
}

TSharedRef<SWidget> UPNInventoryGridWidget::RebuildWidget()
{
	if (bBuildNativeGrid && WidgetTree)
	{
		RootSizeBox = nullptr;
		RootBorder = nullptr;
		RootVerticalBox = nullptr;
		HeaderBox = nullptr;
		HeaderIconBorder = nullptr;
		HeaderIconImage = nullptr;
		HeaderTitleText = nullptr;
		HeaderCounterBorder = nullptr;
		HeaderCounterText = nullptr;
		GridSizeBox = nullptr;
		GridScrollBox = nullptr;
		GridOverlay = nullptr;
		SlotGridPanel = nullptr;
		ItemCanvasPanel = nullptr;
		HeaderBorder = nullptr;
		GridBorder = nullptr;

		BuildNativeGridRoot();
		UpdateNativeHeader();
		RebuildNativeSlots();
		RebuildNativeItems();
	}

	return Super::RebuildWidget();
}

void UPNInventoryGridWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	RootSizeBox = nullptr;
	RootBorder = nullptr;
	RootVerticalBox = nullptr;
	HeaderBox = nullptr;
	HeaderIconBorder = nullptr;
	HeaderIconImage = nullptr;
	HeaderTitleText = nullptr;
	HeaderCounterBorder = nullptr;
	HeaderCounterText = nullptr;
	GridSizeBox = nullptr;
	GridScrollBox = nullptr;
	GridOverlay = nullptr;
	SlotGridPanel = nullptr;
	ItemCanvasPanel = nullptr;
	HeaderBorder = nullptr;
	GridBorder = nullptr;
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

	PanelWidth = SourceWidget->PanelWidth;
	HeaderHeight = SourceWidget->HeaderHeight;
	HeaderIconSize = SourceWidget->HeaderIconSize;
	HeaderCounterWidth = SourceWidget->HeaderCounterWidth;
	HeaderBottomSpacing = SourceWidget->HeaderBottomSpacing;

	SlotsPerRow = SourceWidget->SlotsPerRow;
	SlotSize = SourceWidget->SlotSize;

	DefaultInventorySlots = SourceWidget->DefaultInventorySlots;
	DefaultVestSlots = SourceWidget->DefaultVestSlots;
	DefaultBackpackSlots = SourceWidget->DefaultBackpackSlots;

	InventoryCenterHeight = SourceWidget->InventoryCenterHeight;
	VestCenterHeight = SourceWidget->VestCenterHeight;
	BackpackMinCenterHeight = SourceWidget->BackpackMinCenterHeight;
	BackpackMaxCenterHeight = SourceWidget->BackpackMaxCenterHeight;
	bUseBackpackScrollWhenNeeded = SourceWidget->bUseBackpackScrollWhenNeeded;

	RootBackgroundTexture = SourceWidget->RootBackgroundTexture;
	HeaderBackgroundTexture = SourceWidget->HeaderBackgroundTexture;
	HeaderIconBackgroundTexture = SourceWidget->HeaderIconBackgroundTexture;
	HeaderCounterBackgroundTexture = SourceWidget->HeaderCounterBackgroundTexture;

	SlotNoneTexture = SourceWidget->SlotNoneTexture;
	SlotActiveTexture = SourceWidget->SlotActiveTexture;
	SlotHoverTexture = SourceWidget->SlotHoverTexture;
	SlotBlockTexture = SourceWidget->SlotBlockTexture;
	ItemBackgroundTexture = SourceWidget->ItemBackgroundTexture;

	RootBackgroundColor = SourceWidget->RootBackgroundColor;
	HeaderBackgroundColor = SourceWidget->HeaderBackgroundColor;
	HeaderIconBackgroundColor = SourceWidget->HeaderIconBackgroundColor;
	HeaderCounterBackgroundColor = SourceWidget->HeaderCounterBackgroundColor;

	SlotNoneColor = SourceWidget->SlotNoneColor;
	SlotActiveColor = SourceWidget->SlotActiveColor;
	SlotHoverColor = SourceWidget->SlotHoverColor;
	SlotBlockColor = SourceWidget->SlotBlockColor;
	ItemBackgroundColor = SourceWidget->ItemBackgroundColor;

	TextColor = SourceWidget->TextColor;
	CounterTextColor = SourceWidget->CounterTextColor;

	TitleFontSize = SourceWidget->TitleFontSize;
	CounterFontSize = SourceWidget->CounterFontSize;

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
	UpdateNativeHeader();
	RebuildNativeSlots();
	RebuildNativeItems();
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
	return SlotSize;
}

FText UPNInventoryGridWidget::GetDisplayTitle() const
{
	if (!InventoryData.DisplayTitle.IsEmpty())
	{
		return InventoryData.DisplayTitle;
	}

	if (!DisplayTitle.IsEmpty())
	{
		return DisplayTitle;
	}

	switch (InventoryData.Panel)
	{
	case EPNHUDInventoryPanel::MainInventory:
		return FText::FromString(TEXT("Inventory"));

	case EPNHUDInventoryPanel::Vest:
		return FText::FromString(TEXT("Vest"));

	case EPNHUDInventoryPanel::Backpack:
		return FText::FromString(TEXT("Backpack"));

	case EPNHUDInventoryPanel::OpenedContainer:
		return FText::FromString(TEXT("Container"));

	default:
		return FText::FromString(TEXT("Inventory"));
	}
}

UTexture2D* UPNInventoryGridWidget::GetHeaderIconTexture() const
{
	if (!InventoryData.DisplayIcon.IsNull())
	{
		return InventoryData.DisplayIcon.LoadSynchronous();
	}

	if (!DisplayIcon.IsNull())
	{
		return DisplayIcon.LoadSynchronous();
	}

	return nullptr;
}

FText UPNInventoryGridWidget::GetSlotCounterText() const
{
	return FText::FromString(
		FString::Printf(TEXT("%d/%d"), GetUsedSlotAreaCount(), GetMaxVisualSlotCount())
	);
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
	if (InventoryData.bIsActive && InventoryData.SlotCount > 0)
	{
		return InventoryData.SlotCount;
	}

	return GetDefaultVisualSlotCountForPanel();
}

int32 UPNInventoryGridWidget::GetOccupiedItemCount() const
{
	return InventoryData.Items.Num();
}

int32 UPNInventoryGridWidget::GetUsedSlotAreaCount() const
{
	int32 UsedArea = 0;

	for (const FPNHUDInventoryItemData& ItemData : InventoryData.Items)
	{
		if (!ItemData.Item.bValid)
		{
			continue;
		}

		const int32 Width = FMath::Max(1, ItemData.Size.GetFinalWidth());
		const int32 Height = FMath::Max(1, ItemData.Size.GetFinalHeight());

		UsedArea += Width * Height;
	}

	return FMath::Clamp(UsedArea, 0, GetMaxVisualSlotCount());
}

int32 UPNInventoryGridWidget::GetVisualColumns() const
{
	return FMath::Max(1, SlotsPerRow);
}

int32 UPNInventoryGridWidget::GetVisualRows() const
{
	const int32 VisualSlotCount = GetMaxVisualSlotCount();
	const int32 VisualColumns = GetVisualColumns();

	return FMath::Max(1, FMath::CeilToInt(static_cast<float>(VisualSlotCount) / static_cast<float>(VisualColumns)));
}

FVector2D UPNInventoryGridWidget::GetVisualGridSize() const
{
	return FVector2D(
		static_cast<float>(GetVisualColumns()) * SlotSize,
		GetGridContentHeight()
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

		if (SlotData.bUnlocked)
		{
			FindItemAtPosition(SlotData.Position, SlotData.OccupyingItem, SlotData.bRootItemSlot);
			SlotData.bOccupied = SlotData.OccupyingItem.Item.bValid;
		}

		Result.Add(SlotData);
	}

	return Result;
}

TArray<FPNHUDInventoryItemVisualData> UPNInventoryGridWidget::GetVisualItemData() const
{
	TArray<FPNHUDInventoryItemVisualData> Result;
	Result.Reserve(InventoryData.Items.Num());

	int32 LayerIndex = 1;

	for (const FPNHUDInventoryItemData& ItemData : InventoryData.Items)
	{
		if (!ItemData.Item.bValid)
		{
			continue;
		}

		const int32 WidthSlots = FMath::Max(1, ItemData.Size.GetFinalWidth());
		const int32 HeightSlots = FMath::Max(1, ItemData.Size.GetFinalHeight());

		FPNHUDInventoryItemVisualData VisualData;
		VisualData.ItemData = ItemData;

		VisualData.PixelPosition = FVector2D(
			static_cast<float>(ItemData.Position.X) * SlotSize,
			static_cast<float>(ItemData.Position.Y) * SlotSize
		);

		VisualData.PixelSize = FVector2D(
			static_cast<float>(WidthSlots) * SlotSize,
			static_cast<float>(HeightSlots) * SlotSize
		);

		VisualData.Layer = LayerIndex++;

		Result.Add(VisualData);
	}

	return Result;
}

bool UPNInventoryGridWidget::IsInventoryUnlocked() const
{
	return GetUnlockedSlotCount() > 0;
}

int32 UPNInventoryGridWidget::GetDefaultVisualSlotCountForPanel() const
{
	switch (InventoryData.Panel)
	{
	case EPNHUDInventoryPanel::Vest:
		return FMath::Max(1, DefaultVestSlots);

	case EPNHUDInventoryPanel::Backpack:
		return FMath::Max(1, DefaultBackpackSlots);

	case EPNHUDInventoryPanel::MainInventory:
	case EPNHUDInventoryPanel::OpenedContainer:
	default:
		return FMath::Max(1, DefaultInventorySlots);
	}
}

float UPNInventoryGridWidget::GetGridContentHeight() const
{
	return static_cast<float>(GetVisualRows()) * SlotSize;
}

float UPNInventoryGridWidget::GetGridViewportHeight() const
{
	const float ContentHeight = GetGridContentHeight();

	switch (InventoryData.Panel)
	{
	case EPNHUDInventoryPanel::MainInventory:
		return InventoryCenterHeight;

	case EPNHUDInventoryPanel::Vest:
		return VestCenterHeight;

	case EPNHUDInventoryPanel::Backpack:
		return FMath::Clamp(ContentHeight, BackpackMinCenterHeight, BackpackMaxCenterHeight);

	default:
		return ContentHeight;
	}
}

bool UPNInventoryGridWidget::ShouldUseGridScroll() const
{
	if (InventoryData.Panel != EPNHUDInventoryPanel::Backpack)
	{
		return false;
	}

	if (!bUseBackpackScrollWhenNeeded)
	{
		return false;
	}

	return GetGridContentHeight() > GetGridViewportHeight() + 1.0f;
}

EPNInventorySlotVisualState UPNInventoryGridWidget::GetSlotVisualState(const FPNHUDInventoryGridSlotData& SlotData) const
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

void UPNInventoryGridWidget::BuildPreviewInventoryData()
{
	InventoryData.Panel = PreviewPanel;
	InventoryData.InventoryType = EPNInventoryType::Inventory;
	InventoryData.bIsActive = true;
	InventoryData.bCanReceiveItems = true;
	InventoryData.bCanRemoveItems = true;
	InventoryData.Columns = GetVisualColumns();
	InventoryData.Rows = GetVisualRows();
	InventoryData.SlotCount = FMath::Clamp(PreviewUnlockedSlots, 0, GetDefaultVisualSlotCountForPanel());
	InventoryData.Items.Reset();

	const int32 SafePreviewUsedSlots = FMath::Clamp(PreviewUsedSlots, 0, InventoryData.SlotCount);

	for (int32 Index = 0; Index < SafePreviewUsedSlots; ++Index)
	{
		FPNHUDInventoryItemData PreviewItem;
		PreviewItem.Item.bValid = true;
		PreviewItem.Item.Quantity = 1;
		PreviewItem.Position.X = Index % GetVisualColumns();
		PreviewItem.Position.Y = Index / GetVisualColumns();
		PreviewItem.Size.Width = 1;
		PreviewItem.Size.Height = 1;
		PreviewItem.Size.bRotated = false;
		PreviewItem.bRotated = false;

		InventoryData.Items.Add(PreviewItem);
	}
}

void UPNInventoryGridWidget::BuildNativeGridRoot()
{
	if (!WidgetTree)
	{
		return;
	}

	const FVector2D GridContentSize = GetVisualGridSize();
	const float ViewportHeight = GetGridViewportHeight();
	const float TotalWidth = PanelWidth;
	const float TotalHeight = HeaderHeight + HeaderBottomSpacing + ViewportHeight;

	if (!RootSizeBox)
	{
		RootSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		WidgetTree->RootWidget = RootSizeBox;

		RootBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
		RootBorder->SetPadding(FMargin(0.0f));
		RootSizeBox->AddChild(RootBorder);

		RootVerticalBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		RootBorder->SetContent(RootVerticalBox);

		HeaderBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
		HeaderBorder->SetPadding(FMargin(0.0f));

		if (UVerticalBoxSlot* HeaderVBoxSlot = RootVerticalBox->AddChildToVerticalBox(HeaderBorder))
		{
			HeaderVBoxSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, HeaderBottomSpacing));
		}

		HeaderBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		HeaderBorder->SetContent(HeaderBox);

		USizeBox* IconSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		IconSizeBox->SetWidthOverride(HeaderIconSize);
		IconSizeBox->SetHeightOverride(HeaderHeight);

		HeaderIconBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
		HeaderIconBorder->SetPadding(FMargin(0.0f));
		HeaderIconBorder->SetHorizontalAlignment(HAlign_Fill);
		HeaderIconBorder->SetVerticalAlignment(VAlign_Fill);
		IconSizeBox->AddChild(HeaderIconBorder);

		HeaderIconImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
		HeaderIconImage->SetColorAndOpacity(FLinearColor::White);
		HeaderIconBorder->SetContent(HeaderIconImage);

		if (UHorizontalBoxSlot* IconSlot = HeaderBox->AddChildToHorizontalBox(IconSizeBox))
		{
			IconSlot->SetPadding(FMargin(0.0f));
			IconSlot->SetHorizontalAlignment(HAlign_Left);
			IconSlot->SetVerticalAlignment(VAlign_Fill);
		}

		HeaderTitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		HeaderTitleText->SetColorAndOpacity(FSlateColor(TextColor));
		HeaderTitleText->SetJustification(ETextJustify::Left);

		if (UHorizontalBoxSlot* TitleSlot = HeaderBox->AddChildToHorizontalBox(HeaderTitleText))
		{
			TitleSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
			TitleSlot->SetPadding(FMargin(12.0f, 0.0f, 8.0f, 0.0f));
			TitleSlot->SetVerticalAlignment(VAlign_Center);
		}

		USizeBox* CounterSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		CounterSizeBox->SetWidthOverride(HeaderCounterWidth);
		CounterSizeBox->SetHeightOverride(HeaderHeight);

		HeaderCounterBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
		HeaderCounterBorder->SetPadding(FMargin(0.0f));
		HeaderCounterBorder->SetHorizontalAlignment(HAlign_Center);
		HeaderCounterBorder->SetVerticalAlignment(VAlign_Center);
		CounterSizeBox->AddChild(HeaderCounterBorder);

		HeaderCounterText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		HeaderCounterText->SetColorAndOpacity(FSlateColor(CounterTextColor));
		HeaderCounterText->SetJustification(ETextJustify::Center);
		HeaderCounterBorder->SetContent(HeaderCounterText);

		if (UHorizontalBoxSlot* CounterSlot = HeaderBox->AddChildToHorizontalBox(CounterSizeBox))
		{
			CounterSlot->SetPadding(FMargin(0.0f));
			CounterSlot->SetHorizontalAlignment(HAlign_Right);
			CounterSlot->SetVerticalAlignment(VAlign_Fill);
		}

		GridBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
		GridBorder->SetPadding(FMargin(0.0f));

		if (UVerticalBoxSlot* GridVBoxSlot = RootVerticalBox->AddChildToVerticalBox(GridBorder))
		{
			GridVBoxSlot->SetPadding(FMargin(0.0f));
		}

		GridSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		GridBorder->SetContent(GridSizeBox);

		GridScrollBox = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass());
		GridScrollBox->SetOrientation(Orient_Vertical);
		GridScrollBox->SetScrollBarVisibility(ESlateVisibility::Collapsed);
		GridSizeBox->AddChild(GridScrollBox);

		GridOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
		GridScrollBox->AddChild(GridOverlay);

		SlotGridPanel = WidgetTree->ConstructWidget<UUniformGridPanel>(UUniformGridPanel::StaticClass());

		if (UOverlaySlot* SlotGridOverlaySlot = GridOverlay->AddChildToOverlay(SlotGridPanel))
		{
			SlotGridOverlaySlot->SetHorizontalAlignment(HAlign_Fill);
			SlotGridOverlaySlot->SetVerticalAlignment(VAlign_Fill);
		}

		ItemCanvasPanel = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass());
		ItemCanvasPanel->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

		if (UOverlaySlot* ItemCanvasOverlaySlot = GridOverlay->AddChildToOverlay(ItemCanvasPanel))
		{
			ItemCanvasOverlaySlot->SetHorizontalAlignment(HAlign_Fill);
			ItemCanvasOverlaySlot->SetVerticalAlignment(VAlign_Fill);
		}
	}

	RootSizeBox->SetWidthOverride(TotalWidth);
	RootSizeBox->SetHeightOverride(TotalHeight);

	if (GridSizeBox)
	{
		GridSizeBox->SetWidthOverride(TotalWidth);
		GridSizeBox->SetHeightOverride(ViewportHeight);
	}

	if (RootBorder)
	{
		ApplyTextureToBorder(
			RootBorder,
			RootBackgroundTexture.LoadSynchronous(),
			RootBackgroundColor,
			FVector2D(TotalWidth, TotalHeight)
		);
	}

	if (HeaderBorder)
	{
		ApplyTextureToBorder(
			HeaderBorder,
			HeaderBackgroundTexture.LoadSynchronous(),
			HeaderBackgroundColor,
			FVector2D(TotalWidth, HeaderHeight)
		);
	}

	if (HeaderIconBorder)
	{
		ApplyTextureToBorder(
			HeaderIconBorder,
			HeaderIconBackgroundTexture.LoadSynchronous(),
			HeaderIconBackgroundColor,
			FVector2D(HeaderIconSize, HeaderHeight)
		);
	}

	if (HeaderCounterBorder)
	{
		ApplyTextureToBorder(
			HeaderCounterBorder,
			HeaderCounterBackgroundTexture.LoadSynchronous(),
			HeaderCounterBackgroundColor,
			FVector2D(HeaderCounterWidth, HeaderHeight)
		);
	}

	if (GridBorder)
	{
		ApplyTextureToBorder(
			GridBorder,
			nullptr,
			FLinearColor(0.0f, 0.0f, 0.0f, 0.0f),
			FVector2D(GridContentSize.X, ViewportHeight)
		);
	}
}

void UPNInventoryGridWidget::UpdateNativeHeader()
{
	if (!HeaderTitleText || !HeaderCounterText)
	{
		return;
	}

	const FString UpperTitle = GetDisplayTitle().ToString().ToUpper();

	HeaderTitleText->SetText(FText::FromString(UpperTitle));
	HeaderTitleText->SetColorAndOpacity(FSlateColor(TextColor));
	HeaderTitleText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), TitleFontSize));

	HeaderCounterText->SetText(GetSlotCounterText());
	HeaderCounterText->SetColorAndOpacity(FSlateColor(CounterTextColor));
	HeaderCounterText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), CounterFontSize));

	if (HeaderIconImage)
	{
		UTexture2D* HeaderIconTexture = GetHeaderIconTexture();
		HeaderIconImage->SetColorAndOpacity(FLinearColor::White);

		ApplyTextureToImage(
			HeaderIconImage,
			HeaderIconTexture,
			FVector2D(HeaderIconSize, HeaderHeight)
		);
	}
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

void UPNInventoryGridWidget::RebuildNativeItems()
{
	if (!WidgetTree || !ItemCanvasPanel)
	{
		return;
	}

	ItemCanvasPanel->ClearChildren();
	ItemCanvasPanel->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	const TArray<FPNHUDInventoryItemVisualData> Items = GetVisualItemData();

	for (const FPNHUDInventoryItemVisualData& VisualData : Items)
	{
		USizeBox* ItemSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		ItemSizeBox->SetWidthOverride(VisualData.PixelSize.X);
		ItemSizeBox->SetHeightOverride(VisualData.PixelSize.Y);
		ItemSizeBox->SetVisibility(ESlateVisibility::HitTestInvisible);

		UBorder* ItemBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
		ItemBorder->SetPadding(FMargin(0.0f));
		ItemBorder->SetVisibility(ESlateVisibility::HitTestInvisible);

		ApplyTextureToBorder(
			ItemBorder,
			ItemBackgroundTexture.LoadSynchronous(),
			ItemBackgroundColor,
			VisualData.PixelSize
		);

		UTexture2D* IconTexture = nullptr;

		if (VisualData.ItemData.Item.ItemData && !VisualData.ItemData.Item.ItemData->Visual.Icon.IsNull())
		{
			IconTexture = VisualData.ItemData.Item.ItemData->Visual.Icon.LoadSynchronous();
		}

		if (IconTexture)
		{
			UImage* ItemImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
			ItemImage->SetVisibility(ESlateVisibility::HitTestInvisible);
			ApplyTextureToImage(ItemImage, IconTexture, VisualData.PixelSize);
			ItemBorder->SetContent(ItemImage);
		}
		else
		{
			UTextBlock* ItemText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
			ItemText->SetVisibility(ESlateVisibility::HitTestInvisible);

			FText NameText = FText::FromString(TEXT("Item"));
			if (VisualData.ItemData.Item.ItemData)
			{
				NameText = VisualData.ItemData.Item.ItemData->GetItemName();
			}

			ItemText->SetText(NameText);
			ItemText->SetColorAndOpacity(FSlateColor(TextColor));
			ItemBorder->SetContent(ItemText);
		}

		ItemSizeBox->AddChild(ItemBorder);

		if (UCanvasPanelSlot* CanvasSlot = ItemCanvasPanel->AddChildToCanvas(ItemSizeBox))
		{
			CanvasSlot->SetPosition(VisualData.PixelPosition);
			CanvasSlot->SetSize(VisualData.PixelSize);
			CanvasSlot->SetZOrder(VisualData.Layer);
		}
	}
}

void UPNInventoryGridWidget::ApplyTextureToImage(UImage* TargetImage, UTexture2D* Texture, const FVector2D& ImageSize) const
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
	TargetImage->SetVisibility(ESlateVisibility::Visible);
}

void UPNInventoryGridWidget::ApplyTextureToBorder(UBorder* TargetBorder, UTexture2D* Texture, const FLinearColor& Color, const FVector2D& ImageSize) const
{
	if (!TargetBorder)
	{
		return;
	}

	FSlateBrush Brush = *FCoreStyle::Get().GetBrush(TEXT("WhiteBrush"));

	if (Texture)
	{
		Brush.SetResourceObject(Texture);
	}

	Brush.SetImageSize(ImageSize);

	TargetBorder->SetBrush(Brush);
	TargetBorder->SetBrushColor(Color);
}

void UPNInventoryGridWidget::ApplySlotButtonStyle(UButton* TargetButton, EPNInventorySlotVisualState SlotState, const FVector2D& ImageSize) const
{
	if (!TargetButton)
	{
		return;
	}

	TSoftObjectPtr<UTexture2D> NormalTexture;
	FLinearColor NormalColor = SlotNoneColor;

	switch (SlotState)
	{
	case EPNInventorySlotVisualState::ActiveSlot:
		NormalTexture = SlotActiveTexture;
		NormalColor = SlotActiveColor;
		break;

	case EPNInventorySlotVisualState::BlockSlot:
		NormalTexture = SlotBlockTexture;
		NormalColor = SlotBlockColor;
		break;

	case EPNInventorySlotVisualState::NoneSlot:
	case EPNInventorySlotVisualState::HoverSlot:
	default:
		NormalTexture = SlotNoneTexture;
		NormalColor = SlotNoneColor;
		break;
	}

	const bool bBlocked = SlotState == EPNInventorySlotVisualState::BlockSlot;

	TSoftObjectPtr<UTexture2D> HoverTexture = bBlocked ? SlotBlockTexture : SlotHoverTexture;
	FLinearColor HoverColor = bBlocked ? SlotBlockColor : SlotHoverColor;

	auto MakeBrush = [ImageSize](UTexture2D* Texture, const FLinearColor& Color)
	{
		FSlateBrush Brush = *FCoreStyle::Get().GetBrush(TEXT("WhiteBrush"));

		if (Texture)
		{
			Brush.SetResourceObject(Texture);
		}

		Brush.SetImageSize(ImageSize);
		Brush.TintColor = FSlateColor(Color);

		return Brush;
	};

	FButtonStyle ButtonStyle;
	ButtonStyle.SetNormal(MakeBrush(NormalTexture.LoadSynchronous(), NormalColor));
	ButtonStyle.SetHovered(MakeBrush(HoverTexture.LoadSynchronous(), HoverColor));
	ButtonStyle.SetPressed(MakeBrush(HoverTexture.LoadSynchronous(), HoverColor));
	ButtonStyle.SetDisabled(MakeBrush(SlotBlockTexture.LoadSynchronous(), SlotBlockColor));

	ButtonStyle.NormalPadding = FMargin(0.0f);
	ButtonStyle.PressedPadding = FMargin(0.0f);

	TargetButton->SetStyle(ButtonStyle);
	TargetButton->SetBackgroundColor(FLinearColor::White);
	TargetButton->SetColorAndOpacity(FLinearColor::White);
}

bool UPNInventoryGridWidget::FindItemAtPosition(const FPNInventoryGridPosition& Position, FPNHUDInventoryItemData& OutItem, bool& bOutRootSlot) const
{
	OutItem = FPNHUDInventoryItemData();
	bOutRootSlot = false;

	for (const FPNHUDInventoryItemData& ItemData : InventoryData.Items)
	{
		if (!ItemData.Item.bValid)
		{
			continue;
		}

		const int32 Width = FMath::Max(1, ItemData.Size.GetFinalWidth());
		const int32 Height = FMath::Max(1, ItemData.Size.GetFinalHeight());

		const bool bInsideX =
			Position.X >= ItemData.Position.X &&
			Position.X < ItemData.Position.X + Width;

		const bool bInsideY =
			Position.Y >= ItemData.Position.Y &&
			Position.Y < ItemData.Position.Y + Height;

		if (bInsideX && bInsideY)
		{
			OutItem = ItemData;
			bOutRootSlot = Position.X == ItemData.Position.X && Position.Y == ItemData.Position.Y;
			return true;
		}
	}

	return false;
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