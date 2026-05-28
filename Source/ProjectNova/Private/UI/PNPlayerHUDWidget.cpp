#include "UI/PNPlayerHUDWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/Texture2D.h"
#include "Items/PNItemDataAsset.h"
#include "UI/PNPlayerHUDComponent.h"
#include "Framework/PNPlayerController.h"
#include "InputCoreTypes.h"

UPNPlayerHUDWidget::UPNPlayerHUDWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(true);
}

void UPNPlayerHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetIsFocusable(true);

	if (bBuildNativeLayout)
	{
		RebuildNativeWidgetTree();
		SetInventoryVisible(bShowInventoryOnStart);
	}

	BindToHUDComponent();
	RefreshFromHUDComponent();
}

void UPNPlayerHUDWidget::NativeDestruct()
{
	UnbindFromHUDComponent();

	Super::NativeDestruct();
}

FReply UPNPlayerHUDWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey PressedKey = InKeyEvent.GetKey();

	APNPlayerController* PNPlayerController = Cast<APNPlayerController>(GetOwningPlayer());

	if (PressedKey == EKeys::I)
	{
		if (PNPlayerController)
		{
			PNPlayerController->ToggleInventoryHUD();
			return FReply::Handled();
		}

		ToggleInventoryVisible();
		return FReply::Handled();
	}

	if (PressedKey == EKeys::Escape)
	{
		if (IsInventoryVisible())
		{
			if (PNPlayerController)
			{
				PNPlayerController->SetInventoryHUDVisible(false);
				return FReply::Handled();
			}

			SetInventoryVisible(false);
			return FReply::Handled();
		}
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UPNPlayerHUDWidget::InitializeWithHUDComponent(UPNPlayerHUDComponent* InHUDComponent)
{
	if (HUDComponent == InHUDComponent)
	{
		RefreshFromHUDComponent();
		return;
	}

	UnbindFromHUDComponent();

	HUDComponent = InHUDComponent;

	BindToHUDComponent();
	RefreshFromHUDComponent();
}

void UPNPlayerHUDWidget::SetHUDData(const FPNPlayerHUDSnapshot& InHUDData)
{
	CachedHUDData = InHUDData;

	UpdateNativeLayout();

	OnHUDDataUpdated.Broadcast(CachedHUDData);
	BP_OnHUDDataUpdated(CachedHUDData);
}

void UPNPlayerHUDWidget::RefreshFromHUDComponent()
{
	if (!HUDComponent)
	{
		SetHUDData(FPNPlayerHUDSnapshot());
		return;
	}

	HUDComponent->RefreshHUDData();
}

void UPNPlayerHUDWidget::SetInventoryVisible(bool bVisible)
{
	bInventoryVisible = bVisible;

	if (InventoryRootBorder)
	{
		InventoryRootBorder->SetVisibility(bInventoryVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	BP_OnInventoryVisibilityChanged(bInventoryVisible);
}

void UPNPlayerHUDWidget::ToggleInventoryVisible()
{
	SetInventoryVisible(!bInventoryVisible);
}

bool UPNPlayerHUDWidget::IsInventoryVisible() const
{
	return bInventoryVisible;
}

const FPNPlayerHUDSnapshot& UPNPlayerHUDWidget::GetHUDData() const
{
	return CachedHUDData;
}

UPNPlayerHUDComponent* UPNPlayerHUDWidget::GetHUDComponent() const
{
	return HUDComponent;
}

void UPNPlayerHUDWidget::BindToHUDComponent()
{
	if (!HUDComponent)
	{
		return;
	}

	HUDComponent->OnHUDDataChanged.AddUniqueDynamic(this, &UPNPlayerHUDWidget::HandleHUDDataChanged);
}

void UPNPlayerHUDWidget::UnbindFromHUDComponent()
{
	if (!HUDComponent)
	{
		return;
	}

	HUDComponent->OnHUDDataChanged.RemoveDynamic(this, &UPNPlayerHUDWidget::HandleHUDDataChanged);
}

void UPNPlayerHUDWidget::RebuildNativeWidgetTree()
{
	if (!WidgetTree)
	{
		return;
	}

	RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("PN_RootCanvas"));
	WidgetTree->RootWidget = RootCanvas;

	BuildStatsPanel();
	BuildQuickSlotsPanel();
	BuildInventoryPanel();

	UpdateNativeLayout();
}

void UPNPlayerHUDWidget::BuildStatsPanel()
{
	if (!WidgetTree || !RootCanvas)
	{
		return;
	}

	UBorder* StatsBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("PN_StatsBorder"));
	StatsBorder->SetBrushColor(FLinearColor(0.01f, 0.012f, 0.014f, 0.78f));
	StatsBorder->SetPadding(FMargin(10.0f));

	RootCanvas->AddChild(StatsBorder);

	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(StatsBorder->Slot))
	{
		CanvasSlot->SetAnchors(FAnchors(0.0f, 0.0f, 0.0f, 0.0f));
		CanvasSlot->SetAlignment(FVector2D(0.0f, 0.0f));
		CanvasSlot->SetPosition(FVector2D(24.0f, 24.0f));
		CanvasSlot->SetSize(FVector2D(260.0f, 430.0f));
	}

	StatsBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("PN_StatsBox"));
	StatsBorder->SetContent(StatsBox);

	HealthRefs = CreateStatRow(StatsBox, FText::FromString(TEXT("HEALTH")));
	StaminaRefs = CreateStatRow(StatsBox, FText::FromString(TEXT("STAMINA")));
	HungerRefs = CreateStatRow(StatsBox, FText::FromString(TEXT("HUNGER")));
	ThirstRefs = CreateStatRow(StatsBox, FText::FromString(TEXT("THIRST")));
	WeightRefs = CreateStatRow(StatsBox, FText::FromString(TEXT("WEIGHT")));
	RadiationRefs = CreateStatRow(StatsBox, FText::FromString(TEXT("RADIATION")));
	ToxicityRefs = CreateStatRow(StatsBox, FText::FromString(TEXT("TOXICITY")));
	PsyRefs = CreateStatRow(StatsBox, FText::FromString(TEXT("PSY")));
}

void UPNPlayerHUDWidget::BuildQuickSlotsPanel()
{
	if (!WidgetTree || !RootCanvas)
	{
		return;
	}

	UBorder* QuickSlotsBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("PN_QuickSlotsBorder"));
	QuickSlotsBorder->SetBrushColor(FLinearColor(0.01f, 0.012f, 0.014f, 0.82f));
	QuickSlotsBorder->SetPadding(FMargin(8.0f));

	RootCanvas->AddChild(QuickSlotsBorder);

	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(QuickSlotsBorder->Slot))
	{
		CanvasSlot->SetAnchors(FAnchors(0.5f, 1.0f, 0.5f, 1.0f));
		CanvasSlot->SetAlignment(FVector2D(0.5f, 1.0f));
		CanvasSlot->SetPosition(FVector2D(0.0f, -28.0f));
		CanvasSlot->SetSize(FVector2D(760.0f, 112.0f));
	}

	QuickSlotsBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("PN_QuickSlotsBox"));
	QuickSlotsBorder->SetContent(QuickSlotsBox);
}

void UPNPlayerHUDWidget::BuildInventoryPanel()
{
	if (!WidgetTree || !RootCanvas)
	{
		return;
	}

	InventoryRootBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("PN_InventoryRootBorder"));
	InventoryRootBorder->SetBrushColor(FLinearColor(0.006f, 0.007f, 0.008f, 0.94f));
	InventoryRootBorder->SetPadding(FMargin(14.0f));

	RootCanvas->AddChild(InventoryRootBorder);

	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(InventoryRootBorder->Slot))
	{
		CanvasSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
		CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		CanvasSlot->SetPosition(FVector2D(0.0f, 0.0f));
		CanvasSlot->SetSize(FVector2D(1320.0f, 720.0f));
	}

	UVerticalBox* InventoryRootBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("PN_InventoryRootBox"));
	InventoryRootBorder->SetContent(InventoryRootBox);

	UTextBlock* TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("PN_InventoryTitle"));
	TitleText->SetText(FText::FromString(TEXT("INVENTORY")));
	TitleText->SetColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.9f, 0.92f, 1.0f)));

	if (UVerticalBoxSlot* TitleSlot = InventoryRootBox->AddChildToVerticalBox(TitleText))
	{
		TitleSlot->SetPadding(FMargin(4.0f, 0.0f, 4.0f, 12.0f));
	}

	InventoryPanelsBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("PN_InventoryPanelsBox"));

	if (UVerticalBoxSlot* PanelsSlot = InventoryRootBox->AddChildToVerticalBox(InventoryPanelsBox))
	{
		PanelsSlot->SetPadding(FMargin(0.0f));
	}

	MainInventoryRefs = CreateInventoryPanel(InventoryPanelsBox, FText::FromString(TEXT("MAIN")));
	ContainerInventoryRefs = CreateInventoryPanel(InventoryPanelsBox, FText::FromString(TEXT("LOOT")));
}

FPNHUDStatWidgetRefs UPNPlayerHUDWidget::CreateStatRow(UVerticalBox* Parent, const FText& Label)
{
	FPNHUDStatWidgetRefs Refs;

	if (!WidgetTree || !Parent)
	{
		return Refs;
	}

	UVerticalBox* RowBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());

	if (UVerticalBoxSlot* RowSlot = Parent->AddChildToVerticalBox(RowBox))
	{
		RowSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
	}

	Refs.LabelText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Refs.LabelText->SetText(Label);
	Refs.LabelText->SetColorAndOpacity(FSlateColor(FLinearColor(0.70f, 0.76f, 0.78f, 1.0f)));
	RowBox->AddChildToVerticalBox(Refs.LabelText);

	Refs.ProgressBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass());
	Refs.ProgressBar->SetPercent(0.0f);
	Refs.ProgressBar->SetFillColorAndOpacity(FLinearColor(0.38f, 0.78f, 0.52f, 1.0f));

	if (UVerticalBoxSlot* BarSlot = RowBox->AddChildToVerticalBox(Refs.ProgressBar))
	{
		BarSlot->SetPadding(FMargin(0.0f, 2.0f, 0.0f, 2.0f));
	}

	Refs.ValueText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Refs.ValueText->SetText(FText::FromString(TEXT("0 / 0")));
	Refs.ValueText->SetColorAndOpacity(FSlateColor(FLinearColor(0.88f, 0.9f, 0.9f, 1.0f)));
	RowBox->AddChildToVerticalBox(Refs.ValueText);

	return Refs;
}

FPNHUDInventoryPanelWidgetRefs UPNPlayerHUDWidget::CreateInventoryPanel(UHorizontalBox* Parent, const FText& Header)
{
	FPNHUDInventoryPanelWidgetRefs Refs;

	if (!WidgetTree || !Parent)
	{
		return Refs;
	}

	Refs.RootBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	Refs.RootBorder->SetBrushColor(FLinearColor(0.025f, 0.028f, 0.032f, 0.94f));
	Refs.RootBorder->SetPadding(FMargin(8.0f));

	if (UHorizontalBoxSlot* PanelSlot = Parent->AddChildToHorizontalBox(Refs.RootBorder))
	{
		PanelSlot->SetPadding(FMargin(4.0f));
		PanelSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}

	UVerticalBox* PanelBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	Refs.RootBorder->SetContent(PanelBox);

	Refs.HeaderText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Refs.HeaderText->SetText(Header);
	Refs.HeaderText->SetColorAndOpacity(FSlateColor(FLinearColor(0.86f, 0.91f, 0.94f, 1.0f)));
	PanelBox->AddChildToVerticalBox(Refs.HeaderText);

	Refs.WeightText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Refs.WeightText->SetText(FText::FromString(TEXT("0 / 0 kg")));
	Refs.WeightText->SetColorAndOpacity(FSlateColor(FLinearColor(0.55f, 0.62f, 0.64f, 1.0f)));

	if (UVerticalBoxSlot* WeightSlot = PanelBox->AddChildToVerticalBox(Refs.WeightText))
	{
		WeightSlot->SetPadding(FMargin(0.0f, 2.0f, 0.0f, 8.0f));
	}

	Refs.ItemList = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass());
	PanelBox->AddChildToVerticalBox(Refs.ItemList);

	return Refs;
}

void UPNPlayerHUDWidget::UpdateNativeLayout()
{
	if (!bBuildNativeLayout)
	{
		return;
	}

	UpdateStatsPanel();
	UpdateQuickSlotsPanel();

	UpdateInventoryPanel(CachedHUDData.MainInventory, MainInventoryRefs, true);
	UpdateInventoryPanel(CachedHUDData.Container.Inventory, ContainerInventoryRefs, CachedHUDData.Container.bIsOpen);
}

void UPNPlayerHUDWidget::UpdateStatsPanel()
{
	SetStatWidget(HealthRefs, CachedHUDData.Stats.Health);
	SetStatWidget(StaminaRefs, CachedHUDData.Stats.Stamina);
	SetStatWidget(HungerRefs, CachedHUDData.Stats.Hunger);
	SetStatWidget(ThirstRefs, CachedHUDData.Stats.Thirst);
	SetStatWidget(WeightRefs, CachedHUDData.Stats.Weight);
	SetStatWidget(RadiationRefs, CachedHUDData.Stats.Radiation);
	SetStatWidget(ToxicityRefs, CachedHUDData.Stats.Toxicity);
	SetStatWidget(PsyRefs, CachedHUDData.Stats.Psy);
}

void UPNPlayerHUDWidget::UpdateQuickSlotsPanel()
{
	if (!WidgetTree || !QuickSlotsBox)
	{
		return;
	}

	QuickSlotsBox->ClearChildren();
	QuickSlotWidgets.Reset();

	for (const FPNHUDQuickSlotData& QuickSlotData : CachedHUDData.QuickSlots)
	{
		FPNHUDQuickSlotWidgetRefs Refs;

		Refs.RootBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
		Refs.RootBorder->SetPadding(FMargin(6.0f));

		if (UHorizontalBoxSlot* QuickSlotBoxSlot = QuickSlotsBox->AddChildToHorizontalBox(Refs.RootBorder))
		{
			QuickSlotBoxSlot->SetPadding(FMargin(4.0f));
			QuickSlotBoxSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		}

		UVerticalBox* SlotBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		Refs.RootBorder->SetContent(SlotBox);

		Refs.IndexText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Refs.IndexText->SetColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.66f, 0.68f, 1.0f)));
		SlotBox->AddChildToVerticalBox(Refs.IndexText);

		Refs.IconImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
		Refs.IconImage->SetDesiredSizeOverride(FVector2D(48.0f, 48.0f));

		if (UVerticalBoxSlot* IconSlot = SlotBox->AddChildToVerticalBox(Refs.IconImage))
		{
			IconSlot->SetPadding(FMargin(0.0f, 4.0f));
		}

		Refs.ItemText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Refs.ItemText->SetColorAndOpacity(FSlateColor(FLinearColor(0.86f, 0.90f, 0.92f, 1.0f)));
		SlotBox->AddChildToVerticalBox(Refs.ItemText);

		Refs.QuantityText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Refs.QuantityText->SetColorAndOpacity(FSlateColor(FLinearColor(0.55f, 0.62f, 0.64f, 1.0f)));
		SlotBox->AddChildToVerticalBox(Refs.QuantityText);

		SetQuickSlotWidget(Refs, QuickSlotData);
		QuickSlotWidgets.Add(Refs);
	}
}

void UPNPlayerHUDWidget::UpdateInventoryPanel(const FPNHUDInventoryPanelData& Data, FPNHUDInventoryPanelWidgetRefs& Refs, bool bForceVisible)
{
	if (!Refs.RootBorder || !Refs.WeightText || !Refs.ItemList)
	{
		return;
	}

	const bool bVisible = bForceVisible || Data.bIsActive;
	Refs.RootBorder->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	Refs.WeightText->SetText(FText::FromString(
		FString::Printf(TEXT("%.1f / %.1f kg"), Data.Weight.Current, Data.Weight.Max)
	));

	Refs.ItemList->ClearChildren();

	for (const FPNHUDInventoryItemData& HUDItem : Data.Items)
	{
		AddInventoryItemRow(Refs.ItemList, HUDItem);
	}
}

void UPNPlayerHUDWidget::AddInventoryItemRow(UScrollBox* ItemList, const FPNHUDInventoryItemData& ItemData)
{
	if (!WidgetTree || !ItemList)
	{
		return;
	}

	UBorder* RowBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	RowBorder->SetBrushColor(FLinearColor(0.045f, 0.05f, 0.055f, 0.96f));
	RowBorder->SetPadding(FMargin(6.0f));

	UTextBlock* RowText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());

	const FString ItemName = GetItemNameString(ItemData.Item);
	const FString QuantityText = ItemData.Item.Quantity > 1
		? FString::Printf(TEXT(" x%d"), ItemData.Item.Quantity)
		: FString();

	const FString RowString = FString::Printf(
		TEXT("%s%s  [%dx%d]  X:%d Y:%d"),
		*ItemName,
		*QuantityText,
		ItemData.Size.Width,
		ItemData.Size.Height,
		ItemData.Position.X,
		ItemData.Position.Y
	);

	RowText->SetText(FText::FromString(RowString));
	RowText->SetColorAndOpacity(FSlateColor(FLinearColor(0.84f, 0.88f, 0.90f, 1.0f)));

	RowBorder->SetContent(RowText);
	ItemList->AddChild(RowBorder);
}

void UPNPlayerHUDWidget::SetStatWidget(FPNHUDStatWidgetRefs& Refs, const FPNHUDValuePercent& Value)
{
	if (Refs.ProgressBar)
	{
		Refs.ProgressBar->SetPercent(Value.Percent);
	}

	if (Refs.ValueText)
	{
		Refs.ValueText->SetText(MakeValueText(Value));
	}
}

void UPNPlayerHUDWidget::SetQuickSlotWidget(FPNHUDQuickSlotWidgetRefs& Refs, const FPNHUDQuickSlotData& QuickSlotData)
{
	if (Refs.RootBorder)
	{
		const FLinearColor BorderColor = QuickSlotData.bSelected
			? FLinearColor(0.72f, 0.58f, 0.22f, 0.95f)
			: FLinearColor(0.035f, 0.04f, 0.045f, 0.95f);

		Refs.RootBorder->SetBrushColor(BorderColor);
	}

	if (Refs.IndexText)
	{
		Refs.IndexText->SetText(FText::FromString(
			QuickSlotData.SlotIndex != INDEX_NONE
				? FString::Printf(TEXT("%d"), QuickSlotData.SlotIndex + 1)
				: FString(TEXT("-"))
		));
	}

	if (Refs.ItemText)
	{
		Refs.ItemText->SetText(MakeItemText(QuickSlotData.Item));
	}

	if (Refs.QuantityText)
	{
		Refs.QuantityText->SetText(FText::FromString(
			QuickSlotData.Item.Quantity > 1
				? FString::Printf(TEXT("x%d"), QuickSlotData.Item.Quantity)
				: FString()
		));
	}

	SetItemIcon(Refs.IconImage, QuickSlotData.Item);
}

FText UPNPlayerHUDWidget::MakeValueText(const FPNHUDValuePercent& Value) const
{
	return FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), Value.Current, Value.Max));
}

FText UPNPlayerHUDWidget::MakeItemText(const FPNHUDItemViewData& Item) const
{
	if (!Item.bValid || !Item.ItemData)
	{
		return FText::FromString(TEXT("Empty"));
	}

	return Item.ItemData->GetItemName();
}

FString UPNPlayerHUDWidget::GetItemNameString(const FPNHUDItemViewData& Item) const
{
	if (!Item.bValid || !Item.ItemData)
	{
		return FString(TEXT("Empty"));
	}

	return Item.ItemData->GetItemName().ToString();
}

void UPNPlayerHUDWidget::SetItemIcon(UImage* Image, const FPNHUDItemViewData& Item) const
{
	if (!Image)
	{
		return;
	}

	if (!Item.bValid || !Item.ItemData || Item.ItemData->Visual.Icon.IsNull())
	{
		Image->SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	UTexture2D* IconTexture = Item.ItemData->Visual.Icon.LoadSynchronous();
	if (!IconTexture)
	{
		Image->SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	FSlateBrush Brush;
	Brush.SetResourceObject(IconTexture);
	Brush.SetImageSize(FVector2D(48.0f, 48.0f));

	Image->SetBrush(Brush);
	Image->SetVisibility(ESlateVisibility::Visible);
}

void UPNPlayerHUDWidget::HandleHUDDataChanged(const FPNPlayerHUDSnapshot& HUDData)
{
	SetHUDData(HUDData);
}