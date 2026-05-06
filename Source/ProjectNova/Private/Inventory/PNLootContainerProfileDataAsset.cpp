#include "Inventory/PNLootContainerProfileDataAsset.h"

UPNLootContainerProfileDataAsset::UPNLootContainerProfileDataAsset()
{
	ProfileType = EPNLootContainerProfileType::Custom;

	ProfileName = FText::FromString(TEXT("Loot Container Profile"));
	DisplayName = FText::FromString(TEXT("Loot Container"));
	ActionText = FText::FromString(TEXT("Open"));

	ContainerType = EPNInventoryType::LootContainer;

	ContainerSettings.InventoryType = EPNInventoryType::LootContainer;
	ContainerSettings.GridSize.Columns = 8;
	ContainerSettings.GridSize.Rows = 6;
	ContainerSettings.bUseWeightLimit = false;
	ContainerSettings.MaxWeight = 0.0f;
	ContainerSettings.bAllowItemRotation = true;
	ContainerSettings.bAllowStacking = true;

	ApplyLootOnlyRules();

	RespawnProfile.bSpawnLootOnBeginPlay = true;
	RespawnProfile.bClearInventoryBeforeLootRespawn = true;
	RespawnProfile.bRespawnWhenEmpty = true;
	RespawnProfile.LootRespawnSeconds = 300.0f;
	RespawnProfile.bCanOpenEmptyLootContainer = false;
	RespawnProfile.bUseFixedLootSeed = false;
	RespawnProfile.FixedLootSeed = 1337;
}

void UPNLootContainerProfileDataAsset::ApplyDefaultPreset()
{
	ContainerType = EPNInventoryType::LootContainer;

	ContainerSettings.InventoryType = EPNInventoryType::LootContainer;
	ContainerSettings.bUseWeightLimit = false;
	ContainerSettings.MaxWeight = 0.0f;
	ContainerSettings.bAllowItemRotation = true;
	ContainerSettings.bAllowStacking = true;

	RespawnProfile.bSpawnLootOnBeginPlay = true;
	RespawnProfile.bClearInventoryBeforeLootRespawn = true;
	RespawnProfile.bRespawnWhenEmpty = true;
	RespawnProfile.bCanOpenEmptyLootContainer = false;
	RespawnProfile.bUseFixedLootSeed = false;

	switch (ProfileType)
	{
	case EPNLootContainerProfileType::Fridge:
		ProfileName = FText::FromString(TEXT("Fridge"));
		DisplayName = FText::FromString(TEXT("Fridge"));
		ActionText = FText::FromString(TEXT("Search"));
		SetProfileInventorySize(5, 4);
		RespawnProfile.LootRespawnSeconds = 900.0f;
		break;

	case EPNLootContainerProfileType::KitchenCabinet:
		ProfileName = FText::FromString(TEXT("Kitchen Cabinet"));
		DisplayName = FText::FromString(TEXT("Kitchen Cabinet"));
		ActionText = FText::FromString(TEXT("Search"));
		SetProfileInventorySize(5, 4);
		RespawnProfile.LootRespawnSeconds = 900.0f;
		break;

	case EPNLootContainerProfileType::MedicineCabinet:
		ProfileName = FText::FromString(TEXT("Medicine Cabinet"));
		DisplayName = FText::FromString(TEXT("Medicine Cabinet"));
		ActionText = FText::FromString(TEXT("Search"));
		SetProfileInventorySize(4, 3);
		RespawnProfile.LootRespawnSeconds = 1200.0f;
		break;

	case EPNLootContainerProfileType::CivilianCabinet:
		ProfileName = FText::FromString(TEXT("Civilian Cabinet"));
		DisplayName = FText::FromString(TEXT("Cabinet"));
		ActionText = FText::FromString(TEXT("Search"));
		SetProfileInventorySize(6, 5);
		RespawnProfile.LootRespawnSeconds = 900.0f;
		break;

	case EPNLootContainerProfileType::CivilianShelf:
		ProfileName = FText::FromString(TEXT("Civilian Shelf"));
		DisplayName = FText::FromString(TEXT("Shelf"));
		ActionText = FText::FromString(TEXT("Search"));
		SetProfileInventorySize(6, 4);
		RespawnProfile.LootRespawnSeconds = 900.0f;
		break;

	case EPNLootContainerProfileType::WorkshopShelf:
		ProfileName = FText::FromString(TEXT("Workshop Shelf"));
		DisplayName = FText::FromString(TEXT("Workshop Shelf"));
		ActionText = FText::FromString(TEXT("Search"));
		SetProfileInventorySize(7, 5);
		RespawnProfile.LootRespawnSeconds = 1200.0f;
		break;

	case EPNLootContainerProfileType::ToolBox:
		ProfileName = FText::FromString(TEXT("Tool Box"));
		DisplayName = FText::FromString(TEXT("Tool Box"));
		ActionText = FText::FromString(TEXT("Search"));
		SetProfileInventorySize(5, 3);
		RespawnProfile.LootRespawnSeconds = 1200.0f;
		break;

	case EPNLootContainerProfileType::IndustrialCrate:
		ProfileName = FText::FromString(TEXT("Industrial Crate"));
		DisplayName = FText::FromString(TEXT("Industrial Crate"));
		ActionText = FText::FromString(TEXT("Search"));
		SetProfileInventorySize(8, 6);
		RespawnProfile.LootRespawnSeconds = 1500.0f;
		break;

	case EPNLootContainerProfileType::MilitaryCrate:
		ProfileName = FText::FromString(TEXT("Military Crate"));
		DisplayName = FText::FromString(TEXT("Military Crate"));
		ActionText = FText::FromString(TEXT("Search"));
		SetProfileInventorySize(8, 6);
		RespawnProfile.LootRespawnSeconds = 1800.0f;
		break;

	case EPNLootContainerProfileType::WeaponCrate:
		ProfileName = FText::FromString(TEXT("Weapon Crate"));
		DisplayName = FText::FromString(TEXT("Weapon Crate"));
		ActionText = FText::FromString(TEXT("Search"));
		SetProfileInventorySize(10, 6);
		RespawnProfile.LootRespawnSeconds = 2400.0f;
		break;

	case EPNLootContainerProfileType::AmmoBox:
		ProfileName = FText::FromString(TEXT("Ammo Box"));
		DisplayName = FText::FromString(TEXT("Ammo Box"));
		ActionText = FText::FromString(TEXT("Search"));
		SetProfileInventorySize(5, 4);
		RespawnProfile.LootRespawnSeconds = 1800.0f;
		break;

	case EPNLootContainerProfileType::GearCrate:
		ProfileName = FText::FromString(TEXT("Gear Crate"));
		DisplayName = FText::FromString(TEXT("Gear Crate"));
		ActionText = FText::FromString(TEXT("Search"));
		SetProfileInventorySize(8, 6);
		RespawnProfile.LootRespawnSeconds = 1800.0f;
		break;

	case EPNLootContainerProfileType::BackpackCache:
		ProfileName = FText::FromString(TEXT("Backpack Cache"));
		DisplayName = FText::FromString(TEXT("Backpack Cache"));
		ActionText = FText::FromString(TEXT("Search"));
		SetProfileInventorySize(8, 6);
		RespawnProfile.LootRespawnSeconds = 1800.0f;
		break;

	case EPNLootContainerProfileType::ArmorCache:
		ProfileName = FText::FromString(TEXT("Armor Cache"));
		DisplayName = FText::FromString(TEXT("Armor Cache"));
		ActionText = FText::FromString(TEXT("Search"));
		SetProfileInventorySize(8, 6);
		RespawnProfile.LootRespawnSeconds = 1800.0f;
		break;

	case EPNLootContainerProfileType::TrashPile:
		ProfileName = FText::FromString(TEXT("Trash Pile"));
		DisplayName = FText::FromString(TEXT("Trash Pile"));
		ActionText = FText::FromString(TEXT("Search"));
		SetProfileInventorySize(4, 3);
		RespawnProfile.LootRespawnSeconds = 600.0f;
		break;

	case EPNLootContainerProfileType::StashBag:
		ProfileName = FText::FromString(TEXT("Stash Bag"));
		DisplayName = FText::FromString(TEXT("Stash Bag"));
		ActionText = FText::FromString(TEXT("Search"));
		SetProfileInventorySize(6, 4);
		RespawnProfile.LootRespawnSeconds = 1200.0f;
		break;

	case EPNLootContainerProfileType::Custom:
	default:
		if (ProfileName.IsEmpty())
		{
			ProfileName = FText::FromString(TEXT("Custom Loot Container"));
		}

		if (DisplayName.IsEmpty())
		{
			DisplayName = FText::FromString(TEXT("Loot Container"));
		}

		if (ActionText.IsEmpty())
		{
			ActionText = FText::FromString(TEXT("Open"));
		}
		break;
	}

	ApplyLootOnlyRules();
}

bool UPNLootContainerProfileDataAsset::IsValidLootProfile() const
{
	return ContainerType == EPNInventoryType::LootContainer
		&& ContainerSettings.GridSize.Columns > 0
		&& ContainerSettings.GridSize.Rows > 0;
}

void UPNLootContainerProfileDataAsset::ApplyLootOnlyRules()
{
	ContainerSettings.InventoryType = EPNInventoryType::LootContainer;

	// LootContainer только отдаёт предметы игроку.
	// Игрок не может складывать предметы внутрь холодильника/ящика/стеллажа.
	ContainerSettings.bCanReceiveItems = false;
	ContainerSettings.bCanRemoveItems = true;
	ContainerSettings.bCanDropItems = false;
	ContainerSettings.bCanTradeItems = false;
}

void UPNLootContainerProfileDataAsset::SetProfileInventorySize(int32 Columns, int32 Rows)
{
	ContainerSettings.GridSize.Columns = FMath::Max(1, Columns);
	ContainerSettings.GridSize.Rows = FMath::Max(1, Rows);
}