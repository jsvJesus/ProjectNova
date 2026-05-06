#include "Inventory/PNLootTableDataAsset.h"

#include "Items/PNItemDataAsset.h"
#include "Items/PNItemInstance.h"

TArray<UPNItemInstance*> UPNLootTableDataAsset::GenerateLoot(UObject* Outer) const
{
	FRandomStream RandomStream;
	RandomStream.GenerateNewSeed();

	return GenerateLootInternal(Outer, RandomStream);
}

TArray<UPNItemInstance*> UPNLootTableDataAsset::GenerateLootWithSeed(UObject* Outer, int32 Seed) const
{
	FRandomStream RandomStream(Seed);

	return GenerateLootInternal(Outer, RandomStream);
}

TArray<UPNItemInstance*> UPNLootTableDataAsset::GenerateLootInternal(UObject* Outer, FRandomStream& RandomStream) const
{
	TArray<UPNItemInstance*> GeneratedItems;

	if (!Outer)
	{
		return GeneratedItems;
	}

	if (!RollChance(TableChancePercent, RandomStream))
	{
		return GeneratedItems;
	}

	const int32 SafeMaxGeneratedStacks = FMath::Max(0, MaxGeneratedStacks);
	if (SafeMaxGeneratedStacks <= 0)
	{
		return GeneratedItems;
	}

	for (const FPNLootTableCategoryRule& CategoryRule : Categories)
	{
		if (GeneratedItems.Num() >= SafeMaxGeneratedStacks)
		{
			break;
		}

		if (!RollChance(CategoryRule.CategoryChancePercent, RandomStream))
		{
			continue;
		}

		const int32 RollCount = RollInt(CategoryRule.MinRolls, CategoryRule.MaxRolls, RandomStream);

		for (int32 RollIndex = 0; RollIndex < RollCount; ++RollIndex)
		{
			if (GeneratedItems.Num() >= SafeMaxGeneratedStacks)
			{
				break;
			}

			const FPNLootTableEntry* PickedEntry = PickEntryFromCategory(CategoryRule, RandomStream);
			if (!PickedEntry)
			{
				continue;
			}

			UPNItemInstance* NewInstance = BuildLootItemInstance(Outer, *PickedEntry, RandomStream);
			if (!NewInstance || !NewInstance->IsValidItem())
			{
				continue;
			}

			GeneratedItems.Add(NewInstance);
		}
	}

	return GeneratedItems;
}

const FPNLootTableEntry* UPNLootTableDataAsset::PickEntryFromCategory(const FPNLootTableCategoryRule& CategoryRule, FRandomStream& RandomStream) const
{
	float TotalWeight = 0.0f;

	for (const FPNLootTableEntry& Entry : CategoryRule.Entries)
	{
		if (!Entry.ItemData)
		{
			continue;
		}

		if (Entry.Weight <= 0.0f)
		{
			continue;
		}

		if (!RollChance(Entry.ChancePercent, RandomStream))
		{
			continue;
		}

		if (CategoryRule.bEnforceCategoryFilter && !DoesItemMatchSpawnCategory(Entry.ItemData, CategoryRule.SpawnCategory))
		{
			continue;
		}

		TotalWeight += Entry.Weight;
	}

	if (TotalWeight <= 0.0f)
	{
		return nullptr;
	}

	const float Roll = RandomStream.FRandRange(0.0f, TotalWeight);
	float AccumulatedWeight = 0.0f;

	for (const FPNLootTableEntry& Entry : CategoryRule.Entries)
	{
		if (!Entry.ItemData)
		{
			continue;
		}

		if (Entry.Weight <= 0.0f)
		{
			continue;
		}

		if (CategoryRule.bEnforceCategoryFilter && !DoesItemMatchSpawnCategory(Entry.ItemData, CategoryRule.SpawnCategory))
		{
			continue;
		}

		AccumulatedWeight += Entry.Weight;

		if (Roll <= AccumulatedWeight)
		{
			return &Entry;
		}
	}

	return nullptr;
}

UPNItemInstance* UPNLootTableDataAsset::BuildLootItemInstance(UObject* Outer, const FPNLootTableEntry& Entry, FRandomStream& RandomStream) const
{
	if (!Outer || !Entry.ItemData)
	{
		return nullptr;
	}

	UPNItemDataAsset* ItemData = Entry.ItemData;
	const int32 RolledQuantity = RollInt(Entry.MinQuantity, Entry.MaxQuantity, RandomStream);

	int32 FinalQuantity = FMath::Max(1, RolledQuantity);
	if (Entry.bClampQuantityToMaxStack)
	{
		FinalQuantity = FMath::Clamp(FinalQuantity, 1, ItemData->GetMaxStack());
	}

	UPNItemInstance* NewInstance = NewObject<UPNItemInstance>(Outer);
	if (!NewInstance)
	{
		return nullptr;
	}

	NewInstance->Initialize(ItemData, FinalQuantity);

	if (ItemData->UsesDurability())
	{
		const float MinPercent = FMath::Clamp(Entry.RuntimeRollSettings.MinDurabilityPercent, 0.0f, 1.0f);
		const float MaxPercent = FMath::Clamp(Entry.RuntimeRollSettings.MaxDurabilityPercent, 0.0f, 1.0f);
		const float RolledPercent = RollFloat(MinPercent, MaxPercent, RandomStream);
		NewInstance->SetDurability(ItemData->Durability.MaxDurability * RolledPercent);
	}

	if (ItemData->UsesBattery())
	{
		const float MinPercent = FMath::Clamp(Entry.RuntimeRollSettings.MinBatteryPercent, 0.0f, 1.0f);
		const float MaxPercent = FMath::Clamp(Entry.RuntimeRollSettings.MaxBatteryPercent, 0.0f, 1.0f);
		const float RolledPercent = RollFloat(MinPercent, MaxPercent, RandomStream);
		NewInstance->SetBatteryCharge(ItemData->Battery.MaxCharge * RolledPercent);
	}

	if (ItemData->UsesExpiration())
	{
		const float MinPercent = FMath::Clamp(Entry.RuntimeRollSettings.MinShelfLifePercent, 0.0f, 1.0f);
		const float MaxPercent = FMath::Clamp(Entry.RuntimeRollSettings.MaxShelfLifePercent, 0.0f, 1.0f);
		const float RolledPercent = RollFloat(MinPercent, MaxPercent, RandomStream);
		NewInstance->SetRemainingShelfLife(ItemData->Expiration.ShelfLifeSeconds * RolledPercent);
	}

	if (ItemData->ItemCategory == EPNItemCategory::Mag && Entry.RuntimeRollSettings.bRandomizeMagazineAmmo)
	{
		const int32 MinAmmo = FMath::Max(0, ItemData->MagazineStats.ClipSizeMin);
		const int32 MaxAmmo = ItemData->MagazineStats.ClipSizeMax > 0
			? ItemData->MagazineStats.ClipSizeMax
			: ItemData->MagazineStats.ClipSize;

		NewInstance->SetAmmoInMagazine(RollInt(MinAmmo, MaxAmmo, RandomStream));
	}

	return NewInstance;
}

bool UPNLootTableDataAsset::DoesItemMatchSpawnCategory(UPNItemDataAsset* ItemData, EPNLootSpawnCategory Category) const
{
	if (!ItemData)
	{
		return false;
	}

	switch (Category)
	{
	case EPNLootSpawnCategory::None:
		return true;

	case EPNLootSpawnCategory::Household:
		return ItemData->ItemType == EPNItemType::IT_Consumables
			|| ItemData->ItemCategory == EPNItemCategory::Food
			|| ItemData->ItemCategory == EPNItemCategory::Water
			|| ItemData->ItemCategory == EPNItemCategory::Medicine
			|| ItemData->ItemCategory == EPNItemCategory::Melee
			|| ItemData->ItemCategory == EPNItemCategory::Usable
			|| ItemData->ItemCategory == EPNItemCategory::Components
			|| ItemData->ItemCategory == EPNItemCategory::Resource;

	case EPNLootSpawnCategory::Kitchen:
		return ItemData->ItemCategory == EPNItemCategory::Food
			|| ItemData->ItemCategory == EPNItemCategory::Water
			|| ItemData->ItemCategory == EPNItemCategory::Medicine
			|| ItemData->ItemCategory == EPNItemCategory::Melee
			|| ItemData->ItemCategory == EPNItemCategory::Usable;

	case EPNLootSpawnCategory::Medical:
		return ItemData->ItemCategory == EPNItemCategory::Medicine;

	case EPNLootSpawnCategory::Workshop:
		return ItemData->ItemCategory == EPNItemCategory::Components
			|| ItemData->ItemCategory == EPNItemCategory::Resource
			|| ItemData->ItemCategory == EPNItemCategory::Craft
			|| ItemData->ItemCategory == EPNItemCategory::Usable
			|| ItemData->ItemCategory == EPNItemCategory::Melee
			|| ItemData->ItemType == EPNItemType::IT_Builds;

	case EPNLootSpawnCategory::Industrial:
		return ItemData->ItemCategory == EPNItemCategory::Components
			|| ItemData->ItemCategory == EPNItemCategory::Resource
			|| ItemData->ItemType == EPNItemType::IT_Builds;

	case EPNLootSpawnCategory::Military:
		return ItemData->ItemType == EPNItemType::IT_Weapon
			|| ItemData->ItemType == EPNItemType::IT_Weapon_ATTM
			|| ItemData->ItemType == EPNItemType::IT_Armor_ATTM
			|| ItemData->ItemType == EPNItemType::IT_Armor
			|| ItemData->ItemType == EPNItemType::IT_HArmor
			|| ItemData->ItemType == EPNItemType::IT_Backpack
			|| ItemData->ItemCategory == EPNItemCategory::Medicine
			|| ItemData->ItemCategory == EPNItemCategory::Mag;

	case EPNLootSpawnCategory::FoodWaterMedicine:
		return ItemData->ItemCategory == EPNItemCategory::Food
			|| ItemData->ItemCategory == EPNItemCategory::Water
			|| ItemData->ItemCategory == EPNItemCategory::Medicine;

	case EPNLootSpawnCategory::Food:
		return ItemData->ItemCategory == EPNItemCategory::Food;

	case EPNLootSpawnCategory::Water:
		return ItemData->ItemCategory == EPNItemCategory::Water;

	case EPNLootSpawnCategory::Medicine:
		return ItemData->ItemCategory == EPNItemCategory::Medicine;

	case EPNLootSpawnCategory::AmmoAndMagazines:
		return ItemData->ItemCategory == EPNItemCategory::Mag
			|| (ItemData->ItemType == EPNItemType::IT_Items && ItemData->ItemCategory == EPNItemCategory::Resource);

	case EPNLootSpawnCategory::AllWeapons:
		return ItemData->ItemType == EPNItemType::IT_Weapon;

	case EPNLootSpawnCategory::PrimaryWeapons:
		return ItemData->ItemType == EPNItemType::IT_Weapon && IsPrimaryWeaponCategory(ItemData->ItemCategory);

	case EPNLootSpawnCategory::Pistols:
		return ItemData->ItemType == EPNItemType::IT_Weapon && IsPistolCategory(ItemData->ItemCategory);

	case EPNLootSpawnCategory::Melee:
		return ItemData->ItemType == EPNItemType::IT_Weapon && ItemData->ItemCategory == EPNItemCategory::Melee;

	case EPNLootSpawnCategory::Armor:
		return ItemData->ItemType == EPNItemType::IT_Armor;

	case EPNLootSpawnCategory::Helmet:
		return ItemData->ItemType == EPNItemType::IT_HArmor;

	case EPNLootSpawnCategory::Backpack:
		return ItemData->ItemType == EPNItemType::IT_Backpack;

	case EPNLootSpawnCategory::AllGear:
		return ItemData->ItemType == EPNItemType::IT_Armor
			|| ItemData->ItemType == EPNItemType::IT_HArmor
			|| ItemData->ItemType == EPNItemType::IT_Backpack
			|| ItemData->ItemType == EPNItemType::IT_Armor_ATTM;

	case EPNLootSpawnCategory::WeaponAttachments:
		return ItemData->ItemType == EPNItemType::IT_Weapon_ATTM;

	case EPNLootSpawnCategory::ArmorAttachments:
		return ItemData->ItemType == EPNItemType::IT_Armor_ATTM;

	case EPNLootSpawnCategory::CraftItems:
		return ItemData->ItemCategory == EPNItemCategory::Craft;

	case EPNLootSpawnCategory::Components:
		return ItemData->ItemCategory == EPNItemCategory::Components;

	case EPNLootSpawnCategory::Resources:
		return ItemData->ItemCategory == EPNItemCategory::Resource;

	case EPNLootSpawnCategory::Recipes:
		return ItemData->ItemCategory == EPNItemCategory::Recipes;

	case EPNLootSpawnCategory::UsableDevices:
		return ItemData->ItemCategory == EPNItemCategory::Usable;

	case EPNLootSpawnCategory::BuildParts:
		return ItemData->ItemType == EPNItemType::IT_Builds || ItemData->ItemCategory == EPNItemCategory::Parts;

	case EPNLootSpawnCategory::MiscItems:
		return ItemData->ItemType == EPNItemType::IT_Items
			&& ItemData->ItemCategory != EPNItemCategory::Quest;

	default:
		return false;
	}
}

bool UPNLootTableDataAsset::RollChance(float ChancePercent, FRandomStream& RandomStream) const
{
	if (ChancePercent <= 0.0f)
	{
		return false;
	}

	if (ChancePercent >= 100.0f)
	{
		return true;
	}

	return RandomStream.FRandRange(0.0f, 100.0f) <= ChancePercent;
}

int32 UPNLootTableDataAsset::RollInt(int32 MinValue, int32 MaxValue, FRandomStream& RandomStream) const
{
	const int32 ActualMin = FMath::Min(MinValue, MaxValue);
	const int32 ActualMax = FMath::Max(MinValue, MaxValue);

	return RandomStream.RandRange(ActualMin, ActualMax);
}

float UPNLootTableDataAsset::RollFloat(float MinValue, float MaxValue, FRandomStream& RandomStream) const
{
	const float ActualMin = FMath::Min(MinValue, MaxValue);
	const float ActualMax = FMath::Max(MinValue, MaxValue);

	return RandomStream.FRandRange(ActualMin, ActualMax);
}

bool UPNLootTableDataAsset::IsPrimaryWeaponCategory(EPNItemCategory Category) const
{
	return Category == EPNItemCategory::ASR_Type1
		|| Category == EPNItemCategory::ASR_Type2
		|| Category == EPNItemCategory::SNP
		|| Category == EPNItemCategory::SHTG
		|| Category == EPNItemCategory::SMG
		|| Category == EPNItemCategory::MG
		|| Category == EPNItemCategory::RPG;
}

bool UPNLootTableDataAsset::IsPistolCategory(EPNItemCategory Category) const
{
	return Category == EPNItemCategory::HG_Single
		|| Category == EPNItemCategory::HG_Knife
		|| Category == EPNItemCategory::HG_Shield
		|| Category == EPNItemCategory::HG_Items;
}
