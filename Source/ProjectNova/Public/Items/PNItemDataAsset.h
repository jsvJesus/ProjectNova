#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PNItemStats.h"
#include "PNItemDataAsset.generated.h"

UCLASS(BlueprintType)
class PROJECTNOVA_API UPNItemDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	static const FPrimaryAssetType AssetType;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Main")
	EPNItemType ItemType = EPNItemType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Main")
	EPNItemCategory ItemCategory = EPNItemCategory::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Main")
	EPNItemRarity Rarity = EPNItemRarity::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Main")
	FPNItemIdentityData Identity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Main")
	FPNItemVisualData Visual;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Inventory")
	FPNItemGridSize GridSize;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Inventory")
	FPNItemWeightData Weight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Inventory")
	FPNItemStackData Stack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Inventory")
	FPNItemDurabilityData Durability;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Inventory")
	FPNItemBatteryData Battery;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Inventory")
	FPNItemExpirationData Expiration;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Economy")
	FPNItemEconomyData Economy;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Weapon")
	FPNWeaponStats WeaponStats;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Armor")
	FPNArmorStats ArmorStats;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Backpack")
	FPNBackpackStats BackpackStats;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Container")
	FPNContainerStats ContainerStats;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Consumable")
	FPNConsumableStats ConsumableStats;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Magazine")
	FPNMagazineStats MagazineStats;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Weapon Attachment")
	FPNWeaponAttachmentStats WeaponAttachmentStats;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Armor Plate")
	FPNArmorPlateStats ArmorPlateStats;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Armor Attachment")
	FPNArmorAttachmentStats ArmorAttachmentStats;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Quest")
	FPNQuestItemStats QuestItemStats;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Recipe")
	FPNRecipeStats RecipeStats;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Craft")
	FPNCraftStats CraftStats;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Usable")
	FPNUsableStats UsableStats;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Character")
	FPNCharacterPartStats CharacterPartStats;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Build")
	FPNBuildStats BuildStats;

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	UFUNCTION(BlueprintPure, Category = "ProjectNova|Item")
	FName GetItemId() const;

	UFUNCTION(BlueprintPure, Category = "ProjectNova|Item")
	FText GetItemName() const;

	UFUNCTION(BlueprintPure, Category = "ProjectNova|Item")
	FText GetItemDescription() const;

	UFUNCTION(BlueprintPure, Category = "ProjectNova|Item")
	bool IsWeapon() const;

	UFUNCTION(BlueprintPure, Category = "ProjectNova|Item")
	bool IsArmor() const;

	UFUNCTION(BlueprintPure, Category = "ProjectNova|Item")
	bool IsHelmetArmor() const;

	UFUNCTION(BlueprintPure, Category = "ProjectNova|Item")
	bool IsBackpack() const;

	UFUNCTION(BlueprintPure, Category = "ProjectNova|Item")
	bool IsContainer() const;

	UFUNCTION(BlueprintPure, Category = "ProjectNova|Item")
	bool IsConsumable() const;

	UFUNCTION(BlueprintPure, Category = "ProjectNova|Item")
	bool IsWeaponAttachment() const;

	UFUNCTION(BlueprintPure, Category = "ProjectNova|Item")
	bool IsArmorAttachment() const;

	UFUNCTION(BlueprintPure, Category = "ProjectNova|Item")
	bool IsQuestItem() const;

	UFUNCTION(BlueprintPure, Category = "ProjectNova|Item")
	bool IsRecipe() const;

	UFUNCTION(BlueprintPure, Category = "ProjectNova|Item")
	bool IsBuildItem() const;

	UFUNCTION(BlueprintPure, Category = "ProjectNova|Item")
	bool UsesDurability() const;

	UFUNCTION(BlueprintPure, Category = "ProjectNova|Item")
	bool UsesBattery() const;

	UFUNCTION(BlueprintPure, Category = "ProjectNova|Item")
	bool UsesExpiration() const;

	UFUNCTION(BlueprintPure, Category = "ProjectNova|Item")
	bool CanStack() const;

	UFUNCTION(BlueprintPure, Category = "ProjectNova|Item")
	int32 GetMaxStack() const;

	UFUNCTION(BlueprintPure, Category = "ProjectNova|Item")
	float GetSingleWeight() const;

	UFUNCTION(BlueprintPure, Category = "ProjectNova|Item")
	float GetTotalWeightForCount(int32 Count) const;

	UFUNCTION(BlueprintPure, Category = "ProjectNova|Item")
	bool CanBuy() const;

	UFUNCTION(BlueprintPure, Category = "ProjectNova|Item")
	bool CanTrade() const;
};