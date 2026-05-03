#include "Items/PNItemDataAsset.h"

const FPrimaryAssetType UPNItemDataAsset::AssetType = TEXT("PNItem");

FPrimaryAssetId UPNItemDataAsset::GetPrimaryAssetId() const
{
	const FName AssetName = Identity.ItemId.IsNone() ? GetFName() : Identity.ItemId;
	return FPrimaryAssetId(AssetType, AssetName);
}

FName UPNItemDataAsset::GetItemId() const
{
	return Identity.ItemId;
}

FText UPNItemDataAsset::GetItemName() const
{
	return Identity.ItemName;
}

FText UPNItemDataAsset::GetItemDescription() const
{
	return Identity.Description;
}

bool UPNItemDataAsset::IsWeapon() const
{
	return ItemType == EPNItemType::IT_Weapon;
}

bool UPNItemDataAsset::IsArmor() const
{
	return ItemType == EPNItemType::IT_Armor;
}

bool UPNItemDataAsset::IsHelmetArmor() const
{
	return ItemType == EPNItemType::IT_HArmor;
}

bool UPNItemDataAsset::IsBackpack() const
{
	return ItemType == EPNItemType::IT_Backpack;
}

bool UPNItemDataAsset::IsContainer() const
{
	return ItemType == EPNItemType::IT_Container;
}

bool UPNItemDataAsset::IsConsumable() const
{
	return ItemType == EPNItemType::IT_Consumables;
}

bool UPNItemDataAsset::IsWeaponAttachment() const
{
	return ItemType == EPNItemType::IT_Weapon_ATTM;
}

bool UPNItemDataAsset::IsArmorAttachment() const
{
	return ItemType == EPNItemType::IT_Armor_ATTM;
}

bool UPNItemDataAsset::IsQuestItem() const
{
	return ItemType == EPNItemType::IT_Items && ItemCategory == EPNItemCategory::Quest;
}

bool UPNItemDataAsset::IsRecipe() const
{
	return ItemType == EPNItemType::IT_Items && ItemCategory == EPNItemCategory::Recipes;
}

bool UPNItemDataAsset::IsBuildItem() const
{
	return ItemType == EPNItemType::IT_Builds;
}

bool UPNItemDataAsset::UsesDurability() const
{
	return Durability.bUsesDurability;
}

bool UPNItemDataAsset::UsesBattery() const
{
	return Battery.bUsesBattery;
}

bool UPNItemDataAsset::UsesExpiration() const
{
	return Expiration.bUsesExpiration;
}

bool UPNItemDataAsset::CanStack() const
{
	return Stack.bCanStack && Stack.MaxStack > 1;
}

int32 UPNItemDataAsset::GetMaxStack() const
{
	if (!CanStack())
	{
		return 1;
	}

	return FMath::Max(1, Stack.MaxStack);
}

float UPNItemDataAsset::GetSingleWeight() const
{
	return FMath::Max(0.0f, Weight.Weight);
}

float UPNItemDataAsset::GetTotalWeightForCount(int32 Count) const
{
	const int32 SafeCount = FMath::Max(1, Count);

	if (!Weight.bMultiplyByStack)
	{
		return GetSingleWeight();
	}

	return GetSingleWeight() * static_cast<float>(SafeCount);
}

bool UPNItemDataAsset::CanBuy() const
{
	return Economy.bCanBuy && Economy.BuyPrice > 0;
}

bool UPNItemDataAsset::CanTrade() const
{
	return Economy.bCanTrade && Economy.TradePrice > 0;
}