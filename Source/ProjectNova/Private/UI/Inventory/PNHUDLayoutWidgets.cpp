#include "UI/Inventory/PNHUDLayoutWidgets.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Items/PNItemDataAsset.h"
#include "Styling/CoreStyle.h"

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
		InventoryData.Items.Num() > 0;

	if (!bInventoryEnabled)
	{
		return 0;
	}

	return FMath::Clamp(InventoryData.SlotCount, 0, GetMaxVisualSlotCount());
}

int32 UPNInventoryGridWidget::GetMaxVisualSlotCount() const
{
	return FMath::Max(1, MaxSupportedSlotCount);
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
	const int32 SafeDefaultColumns = FMath::Max(1, DefaultVisualColumns);

	if (bUseInventoryColumnsWhenActive && IsInventoryUnlocked() && InventoryData.Columns > 0)
	{
		return FMath::Clamp(InventoryData.Columns, 1, SafeDefaultColumns);
	}

	return SafeDefaultColumns;
}

int32 UPNInventoryGridWidget::GetVisualRows() const
{
	const int32 VisualSlotCount = GetMaxVisualSlotCount();
	const int32 VisualColumns = GetVisualColumns();

	return FMath::Max(1, FMath::CeilToInt(static_cast<float>(VisualSlotCount) / static_cast<float>(VisualColumns)));
}

FVector2D UPNInventoryGridWidget::GetVisualGridSize() const
{
	const int32 VisualColumns = GetVisualColumns();
	const int32 VisualRows = GetVisualRows();

	const float Width =
		static_cast<float>(VisualColumns) * SlotSize +
		static_cast<float>(FMath::Max(0, VisualColumns - 1)) * SlotPadding;

	const float Height =
		static_cast<float>(VisualRows) * SlotSize +
		static_cast<float>(FMath::Max(0, VisualRows - 1)) * SlotPadding;

	return FVector2D(Width, Height);
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

	const float Step = SlotSize + SlotPadding;

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
			static_cast<float>(ItemData.Position.X) * Step,
			static_cast<float>(ItemData.Position.Y) * Step
		);

		VisualData.PixelSize = FVector2D(
			static_cast<float>(WidthSlots) * SlotSize + static_cast<float>(WidthSlots - 1) * SlotPadding,
			static_cast<float>(HeightSlots) * SlotSize + static_cast<float>(HeightSlots - 1) * SlotPadding
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

void UPNInventoryGridWidget::BuildPreviewInventoryData()
{
	InventoryData.Panel = EPNHUDInventoryPanel::MainInventory;
	InventoryData.InventoryType = EPNInventoryType::Inventory;
	InventoryData.bIsActive = true;
	InventoryData.bCanReceiveItems = true;
	InventoryData.bCanRemoveItems = true;
	InventoryData.Columns = GetVisualColumns();
	InventoryData.Rows = GetVisualRows();
	InventoryData.SlotCount = FMath::Clamp(PreviewUnlockedSlots, 0, GetMaxVisualSlotCount());
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

	const FVector2D GridSize = GetVisualGridSize();

	if (!RootSizeBox)
	{
		RootSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		WidgetTree->RootWidget = RootSizeBox;

		RootBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
		RootBorder->SetPadding(FMargin(RootPadding));
		RootSizeBox->AddChild(RootBorder);

		RootVerticalBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		RootBorder->SetContent(RootVerticalBox);

		HeaderBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());

		if (UVerticalBoxSlot* HeaderSlot = RootVerticalBox->AddChildToVerticalBox(HeaderBox))
		{
			HeaderSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 4.0f));
		}

		HeaderIconImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
		HeaderIconImage->SetDesiredSizeOverride(FVector2D(HeaderHeight, HeaderHeight));

		if (UHorizontalBoxSlot* IconSlot = HeaderBox->AddChildToHorizontalBox(HeaderIconImage))
		{
			IconSlot->SetPadding(FMargin(0.0f, 0.0f, 6.0f, 0.0f));
			IconSlot->SetVerticalAlignment(VAlign_Center);
		}

		HeaderTitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		HeaderTitleText->SetColorAndOpacity(FSlateColor(TextColor));

		if (UHorizontalBoxSlot* TitleSlot = HeaderBox->AddChildToHorizontalBox(HeaderTitleText))
		{
			TitleSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
			TitleSlot->SetVerticalAlignment(VAlign_Center);
		}

		HeaderCounterText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		HeaderCounterText->SetColorAndOpacity(FSlateColor(CounterTextColor));

		if (UHorizontalBoxSlot* CounterSlot = HeaderBox->AddChildToHorizontalBox(HeaderCounterText))
		{
			CounterSlot->SetPadding(FMargin(8.0f, 0.0f, 0.0f, 0.0f));
			CounterSlot->SetVerticalAlignment(VAlign_Center);
		}

		GridSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());

		if (UVerticalBoxSlot* GridSlot = RootVerticalBox->AddChildToVerticalBox(GridSizeBox))
		{
			GridSlot->SetPadding(FMargin(0.0f));
		}

		GridOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
		GridSizeBox->AddChild(GridOverlay);

		SlotGridPanel = WidgetTree->ConstructWidget<UUniformGridPanel>(UUniformGridPanel::StaticClass());
		GridOverlay->AddChild(SlotGridPanel);

		ItemCanvasPanel = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass());
		GridOverlay->AddChild(ItemCanvasPanel);
	}

	RootSizeBox->SetWidthOverride(GridSize.X + RootPadding * 2.0f);
	RootSizeBox->SetHeightOverride(GridSize.Y + HeaderHeight + 4.0f + RootPadding * 2.0f);

	if (GridSizeBox)
	{
		GridSizeBox->SetWidthOverride(GridSize.X);
		GridSizeBox->SetHeightOverride(GridSize.Y);
	}

	if (RootBorder)
	{
		ApplyTextureToBorder(
			RootBorder,
			RootBackgroundTexture.LoadSynchronous(),
			RootBackgroundColor,
			FVector2D(GridSize.X + RootPadding * 2.0f, GridSize.Y + HeaderHeight + RootPadding * 2.0f)
		);
	}
}

void UPNInventoryGridWidget::UpdateNativeHeader()
{
	if (!HeaderTitleText || !HeaderCounterText || !HeaderIconImage)
	{
		return;
	}

	HeaderTitleText->SetText(GetDisplayTitle());
	HeaderTitleText->SetColorAndOpacity(FSlateColor(TextColor));

	HeaderCounterText->SetText(GetSlotCounterText());
	HeaderCounterText->SetColorAndOpacity(FSlateColor(CounterTextColor));

	UTexture2D* IconTexture = DisplayIcon.LoadSynchronous();
	if (IconTexture)
	{
		ApplyTextureToImage(HeaderIconImage, IconTexture, FVector2D(HeaderHeight, HeaderHeight));
		HeaderIconImage->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		HeaderIconImage->SetVisibility(ESlateVisibility::Collapsed);
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

		UBorder* SlotBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
		SlotBorder->SetPadding(FMargin(0.0f));

		UTexture2D* SlotTexture = nullptr;
		FLinearColor SlotColor = SlotUnlockedColor;

		if (!SlotData.bUnlocked)
		{
			SlotTexture = SlotLockedTexture.LoadSynchronous();
			SlotColor = SlotLockedColor;
		}
		else if (SlotData.bOccupied)
		{
			SlotTexture = SlotOccupiedTexture.LoadSynchronous();
			SlotColor = SlotOccupiedColor;
		}
		else
		{
			SlotTexture = SlotUnlockedTexture.LoadSynchronous();
			SlotColor = SlotUnlockedColor;
		}

		ApplyTextureToBorder(SlotBorder, SlotTexture, SlotColor, FVector2D(SlotSize, SlotSize));

		SlotSizeBox->AddChild(SlotBorder);

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

	const TArray<FPNHUDInventoryItemVisualData> Items = GetVisualItemData();

	for (const FPNHUDInventoryItemVisualData& VisualData : Items)
	{
		USizeBox* ItemSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		ItemSizeBox->SetWidthOverride(VisualData.PixelSize.X);
		ItemSizeBox->SetHeightOverride(VisualData.PixelSize.Y);

		UBorder* ItemBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
		ItemBorder->SetPadding(FMargin(2.0f));
		ApplyTextureToBorder(
			ItemBorder,
			nullptr,
			FLinearColor(0.12f, 0.13f, 0.14f, 0.96f),
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
			ApplyTextureToImage(ItemImage, IconTexture, VisualData.PixelSize);
			ItemBorder->SetContent(ItemImage);
		}
		else
		{
			UTextBlock* ItemText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());

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
		Brush.SetImageSize(ImageSize);
	}

	TargetBorder->SetBrush(Brush);
	TargetBorder->SetBrushColor(Color);
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