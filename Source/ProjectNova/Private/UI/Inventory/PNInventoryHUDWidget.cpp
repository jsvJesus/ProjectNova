#include "UI/Inventory/PNInventoryHUDWidget.h"

#include "Components/Widget.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Items/PNItemDataAsset.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateTypes.h"
#include "Styling/CoreStyle.h"

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
	ApplySharedInventoryGridStyle();
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

	PushEquipmentDataToBoundSlots();

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

void UPNInventoryHUDWidget::ApplySharedInventoryGridStyle()
{
	if (!InventoryLayout)
	{
		return;
	}

	if (VestInventoryLayout)
	{
		VestInventoryLayout->CopyVisualStyleFrom(InventoryLayout);
	}

	if (BackpackInventoryLayout)
	{
		BackpackInventoryLayout->CopyVisualStyleFrom(InventoryLayout);
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
	const FPNPlayerHUDSnapshot& HUDData = GetHUDData();

	SetLayoutVisible(NavigationLayout, bRootVisible);

	SetLayoutVisible(InventoryLayout, bInventoryPageVisible && HUDData.MainInventory.bIsActive);
	SetLayoutVisible(VestInventoryLayout, bInventoryPageVisible && HUDData.VestInventory.bIsActive);
	SetLayoutVisible(BackpackInventoryLayout, bInventoryPageVisible && HUDData.BackpackInventory.bIsActive);
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

namespace
{
	FSlateBrush PNMakeEquipmentTextureBrush(UTexture2D* Texture, const FVector2D& ImageSize)
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
}

void UPNInventoryHUDWidget::PushEquipmentDataToBoundSlots()
{
	const FPNHUDEquipmentData& CurrentEquipmentData = GetHUDData().Equipment;

	BuildEquipmentSlotWidget(
		Equip_PrimaryWeapon1,
		EPNEquipmentSlot::PrimaryWeapon1,
		FindHUDSlotData(EPNEquipmentSlot::PrimaryWeapon1)
	);

	BuildEquipmentSlotWidget(
		Equip_PrimaryWeapon2,
		EPNEquipmentSlot::PrimaryWeapon2,
		FindHUDSlotData(EPNEquipmentSlot::PrimaryWeapon2)
	);

	BuildEquipmentSlotWidget(
		Equip_Sidearm,
		EPNEquipmentSlot::Sidearm,
		FindHUDSlotData(EPNEquipmentSlot::Sidearm)
	);

	BuildEquipmentSlotWidget(
		Equip_Knife,
		EPNEquipmentSlot::Knife,
		FindHUDSlotData(EPNEquipmentSlot::Knife)
	);

	BuildEquipmentSlotWidget(
		Equip_Helmet,
		EPNEquipmentSlot::Helmet,
		FindHUDSlotData(EPNEquipmentSlot::Helmet)
	);

	BuildEquipmentSlotWidget(
		Equip_Armor,
		EPNEquipmentSlot::Armor,
		FindHUDSlotData(EPNEquipmentSlot::Armor)
	);

	BuildEquipmentSlotWidget(
		Equip_Gloves,
		EPNEquipmentSlot::Gloves,
		FindHUDSlotData(EPNEquipmentSlot::Gloves)
	);

	BuildEquipmentSlotWidget(
		Equip_Backpack,
		EPNEquipmentSlot::Backpack,
		FindHUDSlotData(EPNEquipmentSlot::Backpack)
	);

	BuildInternalEquipmentSlotWidget(
		HelmetInternal_0,
		EPNEquipmentInternalContainer::Helmet,
		0,
		FindHUDInternalSlotData(EPNEquipmentInternalContainer::Helmet, 0)
	);

	BuildInternalEquipmentSlotWidget(
		HelmetInternal_1,
		EPNEquipmentInternalContainer::Helmet,
		1,
		FindHUDInternalSlotData(EPNEquipmentInternalContainer::Helmet, 1)
	);

	BuildInternalEquipmentSlotWidget(
		HelmetInternal_2,
		EPNEquipmentInternalContainer::Helmet,
		2,
		FindHUDInternalSlotData(EPNEquipmentInternalContainer::Helmet, 2)
	);

	BuildInternalEquipmentSlotWidget(
		HelmetInternal_3,
		EPNEquipmentInternalContainer::Helmet,
		3,
		FindHUDInternalSlotData(EPNEquipmentInternalContainer::Helmet, 3)
	);

	BuildInternalEquipmentSlotWidget(
		ArmorInternal_0,
		EPNEquipmentInternalContainer::Armor,
		0,
		FindHUDInternalSlotData(EPNEquipmentInternalContainer::Armor, 0)
	);

	BuildInternalEquipmentSlotWidget(
		ArmorInternal_1,
		EPNEquipmentInternalContainer::Armor,
		1,
		FindHUDInternalSlotData(EPNEquipmentInternalContainer::Armor, 1)
	);

	BuildInternalEquipmentSlotWidget(
		ArmorInternal_2,
		EPNEquipmentInternalContainer::Armor,
		2,
		FindHUDInternalSlotData(EPNEquipmentInternalContainer::Armor, 2)
	);

	BuildInternalEquipmentSlotWidget(
		ArmorInternal_3,
		EPNEquipmentInternalContainer::Armor,
		3,
		FindHUDInternalSlotData(EPNEquipmentInternalContainer::Armor, 3)
	);
}

void UPNInventoryHUDWidget::BuildEquipmentSlotWidget(
	USizeBox* TargetSizeBox,
	EPNEquipmentSlot EquipmentSlot,
	const FPNHUDEquipmentSlotData& SlotData
)
{
	if (!WidgetTree || !TargetSizeBox)
	{
		return;
	}

	const FVector2D RenderSize = GetEquipmentSlotRenderSize(EquipmentSlot);

	TargetSizeBox->ClearChildren();
	TargetSizeBox->SetWidthOverride(RenderSize.X);
	TargetSizeBox->SetHeightOverride(RenderSize.Y);

	UButton* SlotButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
	SlotButton->SetIsEnabled(true);

	ApplyEquipmentButtonStyle(
		SlotButton,
		SlotData.bOccupied,
		false,
		false,
		false,
		RenderSize
	);

	UOverlay* SlotOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
	SlotOverlay->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	if (SlotData.bOccupied && SlotData.Item.bValid && SlotData.Item.ItemData)
	{
		if (UTexture2D* ItemBackgroundTexture = EquipmentItemBackgroundTexture.LoadSynchronous())
		{
			UImage* ItemBackgroundImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
			ItemBackgroundImage->SetVisibility(ESlateVisibility::HitTestInvisible);

			ApplyTextureToEquipmentImage(
				ItemBackgroundImage,
				ItemBackgroundTexture,
				RenderSize
			);

			if (UOverlaySlot* ItemBackgroundOverlaySlot = SlotOverlay->AddChildToOverlay(ItemBackgroundImage))
			{
				ItemBackgroundOverlaySlot->SetHorizontalAlignment(HAlign_Fill);
				ItemBackgroundOverlaySlot->SetVerticalAlignment(VAlign_Fill);
			}
		}

		if (!SlotData.Item.ItemData->Visual.Icon.IsNull())
		{
			if (UTexture2D* ItemIconTexture = SlotData.Item.ItemData->Visual.Icon.LoadSynchronous())
			{
				UImage* ItemIconImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
				ItemIconImage->SetVisibility(ESlateVisibility::HitTestInvisible);

				const FVector2D IconSize = RenderSize * 0.78f;

				ApplyTextureToEquipmentImage(
					ItemIconImage,
					ItemIconTexture,
					IconSize
				);

				if (UOverlaySlot* ItemIconOverlaySlot = SlotOverlay->AddChildToOverlay(ItemIconImage))
				{
					ItemIconOverlaySlot->SetHorizontalAlignment(HAlign_Center);
					ItemIconOverlaySlot->SetVerticalAlignment(VAlign_Center);
				}
			}
		}
	}

	if (IsWeaponEquipmentSlot(EquipmentSlot))
	{
		UTextBlock* LabelText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		LabelText->SetText(GetWeaponSlotLabel(EquipmentSlot));
		LabelText->SetColorAndOpacity(FSlateColor(EquipmentWeaponLabelColor));
		LabelText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), EquipmentWeaponLabelFontSize));
		LabelText->SetJustification(ETextJustify::Center);
		LabelText->SetVisibility(ESlateVisibility::HitTestInvisible);

		if (UOverlaySlot* LabelOverlaySlot = SlotOverlay->AddChildToOverlay(LabelText))
		{
			LabelOverlaySlot->SetHorizontalAlignment(HAlign_Left);
			LabelOverlaySlot->SetVerticalAlignment(VAlign_Top);
			LabelOverlaySlot->SetPadding(FMargin(5.0f, 3.0f, 0.0f, 0.0f));
		}
	}

	SlotButton->AddChild(SlotOverlay);
	TargetSizeBox->AddChild(SlotButton);
}

void UPNInventoryHUDWidget::BuildInternalEquipmentSlotWidget(
	USizeBox* TargetSizeBox,
	EPNEquipmentInternalContainer InternalContainer,
	int32 InternalSlotIndex,
	const FPNHUDInternalEquipmentSlotData& SlotData
)
{
	if (!WidgetTree || !TargetSizeBox)
	{
		return;
	}

	const FVector2D RenderSize(
		FMath::Max(8.0f, EquipmentInternalSlotSize.X),
		FMath::Max(8.0f, EquipmentInternalSlotSize.Y)
	);

	TargetSizeBox->ClearChildren();
	TargetSizeBox->SetWidthOverride(RenderSize.X);
	TargetSizeBox->SetHeightOverride(RenderSize.Y);

	UButton* SlotButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
	SlotButton->SetIsEnabled(SlotData.bUnlocked);

	ApplyEquipmentButtonStyle(
		SlotButton,
		SlotData.bOccupied,
		false,
		!SlotData.bUnlocked,
		true,
		RenderSize
	);

	UOverlay* SlotOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
	SlotOverlay->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	if (SlotData.bOccupied && SlotData.Item.bValid && SlotData.Item.ItemData)
	{
		if (UTexture2D* ItemBackgroundTexture = EquipmentInternalItemBackgroundTexture.LoadSynchronous())
		{
			UImage* ItemBackgroundImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
			ItemBackgroundImage->SetVisibility(ESlateVisibility::HitTestInvisible);

			ApplyTextureToEquipmentImage(
				ItemBackgroundImage,
				ItemBackgroundTexture,
				RenderSize
			);

			if (UOverlaySlot* ItemBackgroundOverlaySlot = SlotOverlay->AddChildToOverlay(ItemBackgroundImage))
			{
				ItemBackgroundOverlaySlot->SetHorizontalAlignment(HAlign_Fill);
				ItemBackgroundOverlaySlot->SetVerticalAlignment(VAlign_Fill);
			}
		}

		if (!SlotData.Item.ItemData->Visual.Icon.IsNull())
		{
			if (UTexture2D* ItemIconTexture = SlotData.Item.ItemData->Visual.Icon.LoadSynchronous())
			{
				UImage* ItemIconImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
				ItemIconImage->SetVisibility(ESlateVisibility::HitTestInvisible);

				const FVector2D IconSize = RenderSize * 0.76f;

				ApplyTextureToEquipmentImage(
					ItemIconImage,
					ItemIconTexture,
					IconSize
				);

				if (UOverlaySlot* ItemIconOverlaySlot = SlotOverlay->AddChildToOverlay(ItemIconImage))
				{
					ItemIconOverlaySlot->SetHorizontalAlignment(HAlign_Center);
					ItemIconOverlaySlot->SetVerticalAlignment(VAlign_Center);
				}
			}
		}
	}

	SlotButton->AddChild(SlotOverlay);
	TargetSizeBox->AddChild(SlotButton);
}

FPNHUDEquipmentSlotData UPNInventoryHUDWidget::FindHUDSlotData(EPNEquipmentSlot EquipmentSlot) const
{
	FPNHUDEquipmentSlotData Result;
	Result.Slot = EquipmentSlot;

	const FPNHUDEquipmentData& CurrentEquipmentData = GetHUDData().Equipment;

	for (const FPNHUDEquipmentSlotData& SlotData : CurrentEquipmentData.EquipmentSlots)
	{
		if (SlotData.Slot == EquipmentSlot)
		{
			return SlotData;
		}
	}

	return Result;
}

FPNHUDInternalEquipmentSlotData UPNInventoryHUDWidget::FindHUDInternalSlotData(
	EPNEquipmentInternalContainer InternalContainer,
	int32 InternalSlotIndex
) const
{
	FPNHUDInternalEquipmentSlotData Result;
	Result.Container = InternalContainer;
	Result.SlotIndex = InternalSlotIndex;
	Result.bUnlocked = false;
	Result.bOccupied = false;

	const FPNHUDEquipmentData& CurrentEquipmentData = GetHUDData().Equipment;

	const TArray<FPNHUDInternalEquipmentSlotData>* SourceSlots = nullptr;

	switch (InternalContainer)
	{
	case EPNEquipmentInternalContainer::Helmet:
		SourceSlots = &CurrentEquipmentData.HelmetInternalSlots;
		break;

	case EPNEquipmentInternalContainer::Armor:
		SourceSlots = &CurrentEquipmentData.ArmorInternalSlots;
		break;

	default:
		return Result;
	}

	for (const FPNHUDInternalEquipmentSlotData& SlotData : *SourceSlots)
	{
		if (SlotData.Container == InternalContainer && SlotData.SlotIndex == InternalSlotIndex)
		{
			return SlotData;
		}
	}

	return Result;
}

FText UPNInventoryHUDWidget::GetWeaponSlotLabel(EPNEquipmentSlot EquipmentSlot) const
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

bool UPNInventoryHUDWidget::IsWeaponEquipmentSlot(EPNEquipmentSlot EquipmentSlot) const
{
	return EquipmentSlot == EPNEquipmentSlot::PrimaryWeapon1
		|| EquipmentSlot == EPNEquipmentSlot::PrimaryWeapon2
		|| EquipmentSlot == EPNEquipmentSlot::Sidearm
		|| EquipmentSlot == EPNEquipmentSlot::Knife;
}

void UPNInventoryHUDWidget::ApplyEquipmentButtonStyle(
	UButton* TargetButton,
	bool bOccupied,
	bool bHoveredSlot,
	bool bLocked,
	bool bInternalSlot,
	const FVector2D& ImageSize
) const
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
		if (bLocked)
		{
			NormalTexture = EquipmentInternalSlotLockedTexture.LoadSynchronous();
			HoveredTexture = NormalTexture;
			PressedTexture = NormalTexture;
			DisabledTexture = NormalTexture;
		}
		else
		{
			NormalTexture = EquipmentInternalSlotBackgroundTexture.LoadSynchronous();
			HoveredTexture = EquipmentInternalSlotHoverTexture.LoadSynchronous();
			PressedTexture = NormalTexture;
			DisabledTexture = NormalTexture;
		}
	}
	else
	{
		NormalTexture = EquipmentSlotBackgroundTexture.LoadSynchronous();
		HoveredTexture = EquipmentSlotHoverTexture.LoadSynchronous();
		PressedTexture = NormalTexture;
		DisabledTexture = NormalTexture;
	}

	if (!HoveredTexture)
	{
		HoveredTexture = NormalTexture;
	}

	FButtonStyle ButtonStyle;
	ButtonStyle.SetNormal(PNMakeEquipmentTextureBrush(NormalTexture, ImageSize));
	ButtonStyle.SetHovered(PNMakeEquipmentTextureBrush(HoveredTexture, ImageSize));
	ButtonStyle.SetPressed(PNMakeEquipmentTextureBrush(PressedTexture, ImageSize));
	ButtonStyle.SetDisabled(PNMakeEquipmentTextureBrush(DisabledTexture, ImageSize));

	ButtonStyle.SetNormalPadding(FMargin(0.0f));
	ButtonStyle.SetPressedPadding(FMargin(0.0f));

	TargetButton->SetStyle(ButtonStyle);
	TargetButton->SetColorAndOpacity(FLinearColor::White);
	TargetButton->SetBackgroundColor(FLinearColor::White);
}

void UPNInventoryHUDWidget::ApplyTextureToEquipmentImage(
	UImage* TargetImage,
	UTexture2D* Texture,
	const FVector2D& ImageSize
) const
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
	Brush.DrawAs = ESlateBrushDrawType::Image;
	Brush.TintColor = FSlateColor(FLinearColor::White);

	TargetImage->SetBrush(Brush);
	TargetImage->SetColorAndOpacity(FLinearColor::White);
	TargetImage->SetVisibility(ESlateVisibility::Visible);
}

FVector2D UPNInventoryHUDWidget::GetEquipmentSlotRenderSize(EPNEquipmentSlot EquipmentSlot) const
{
	switch (EquipmentSlot)
	{
	case EPNEquipmentSlot::PrimaryWeapon1:
	case EPNEquipmentSlot::PrimaryWeapon2:
		return FVector2D(
			FMath::Max(8.0f, EquipmentPrimaryWeaponSlotSize.X),
			FMath::Max(8.0f, EquipmentPrimaryWeaponSlotSize.Y)
		);

	case EPNEquipmentSlot::Sidearm:
	case EPNEquipmentSlot::Knife:
	case EPNEquipmentSlot::Helmet:
	case EPNEquipmentSlot::Armor:
	case EPNEquipmentSlot::Gloves:
	case EPNEquipmentSlot::Backpack:
	default:
		return FVector2D(
			FMath::Max(8.0f, EquipmentDefaultSlotSize.X),
			FMath::Max(8.0f, EquipmentDefaultSlotSize.Y)
		);
	}
}