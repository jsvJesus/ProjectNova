#pragma once

#include "CoreMinimal.h"
#include "PNItemTypes.h"
#include "PNItemStats.generated.h"

class UTexture2D;
class UStaticMesh;
class USkeletalMesh;

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNItemGridSize
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Grid", meta = (ClampMin = "1"))
	int32 Width = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Grid", meta = (ClampMin = "1"))
	int32 Height = 1;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNItemIdentityData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Identity")
	FName ItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Identity")
	FText ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Identity", meta = (MultiLine = "true"))
	FText Description;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNItemVisualData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Visual")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Visual")
	TSoftObjectPtr<USkeletalMesh> SkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Visual")
	TSoftObjectPtr<UStaticMesh> StaticMesh;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNItemEconomyData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Economy")
	bool bCanBuy = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Economy")
	bool bCanTrade = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Economy", meta = (ClampMin = "0"))
	int32 BuyPrice = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Economy", meta = (ClampMin = "0"))
	int32 TradePrice = 0;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNItemWeightData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Weight", meta = (ClampMin = "0.0"))
	float Weight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Weight")
	bool bMultiplyByStack = true;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNItemStackData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Stack")
	bool bCanStack = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Stack", meta = (ClampMin = "1"))
	int32 MaxStack = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Stack")
	bool bStackRequiresSameDurability = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Stack")
	bool bStackRequiresSameBatteryCharge = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Stack")
	bool bStackRequiresSameExpirationTime = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Stack")
	bool bStackRequiresSameAmmoCount = false;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNItemDurabilityData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Durability")
	bool bUsesDurability = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Durability", meta = (ClampMin = "0.0"))
	float MaxDurability = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Durability", meta = (ClampMin = "0.0"))
	float DefaultDurability = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Durability")
	bool bBrokenWhenZero = true;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNItemBatteryData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Battery")
	bool bUsesBattery = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Battery", meta = (ClampMin = "0.0"))
	float MaxCharge = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Battery", meta = (ClampMin = "0.0"))
	float DefaultCharge = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Battery", meta = (ClampMin = "0.0"))
	float DrainPerSecond = 0.0f;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNItemExpirationData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Expiration")
	bool bUsesExpiration = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Expiration", meta = (ClampMin = "0.0"))
	float ShelfLifeSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Expiration", meta = (ClampMin = "0.0"))
	float ToxicityWhenExpired = 0.0f;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNFloatModifier
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Modifier")
	bool bEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Modifier")
	float Value = 0.0f;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNItemAttributeModifiers
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Attributes")
	FPNFloatModifier Endurance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Attributes")
	FPNFloatModifier StaminaRegen;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Attributes")
	FPNFloatModifier Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Attributes")
	FPNFloatModifier HealthRegen;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Attributes")
	FPNFloatModifier Bleeding;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Attributes")
	FPNFloatModifier WoundHealing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Attributes")
	FPNFloatModifier MaxWeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Attributes")
	FPNFloatModifier Telepathy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Attributes")
	FPNFloatModifier PsiProtection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Attributes")
	FPNFloatModifier Burn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Attributes")
	FPNFloatModifier ThermalProtection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Attributes")
	FPNFloatModifier ChemicalBurn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Attributes")
	FPNFloatModifier ChemicalProtection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Attributes")
	FPNFloatModifier ElectricShock;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Attributes")
	FPNFloatModifier ElectricProtection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Attributes")
	FPNFloatModifier Radiation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Attributes")
	FPNFloatModifier RadiationProtection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Attributes")
	FPNFloatModifier Satiety;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Attributes")
	FPNFloatModifier BulletResistance;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNItemProtectionData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Protection", meta = (ClampMin = "0.0"))
	float DamageMin = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Protection", meta = (ClampMin = "0.0"))
	float DamageMax = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Protection")
	EPNProtectionArmor ProtectionLevel = EPNProtectionArmor::None;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNWeaponAttachmentSlots
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Weapon|Attachments")
	bool bUsesMagazine = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Weapon|Attachments")
	bool bUsesScope = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Weapon|Attachments")
	bool bUsesLaser = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Weapon|Attachments")
	bool bUsesFlashlight = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Weapon|Attachments")
	bool bUsesGrip = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Weapon|Attachments")
	bool bUsesMuzzle = false;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNWeaponStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Weapon")
	FName HandSocketName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Weapon")
	EPNAnimType AnimType = EPNAnimType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Weapon")
	FName ReloadAnimationId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Weapon")
	FPNWeaponAttachmentSlots AttachmentSlots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Weapon")
	FName BulletType = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Weapon")
	FName MagazineType = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Weapon")
	EPNScopeType ScopeType = EPNScopeType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Weapon", meta = (ClampMin = "0.0"))
	float Damage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Weapon", meta = (ClampMin = "0.0"))
	float Decay = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Weapon", meta = (ClampMin = "0.0"))
	float Speed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Weapon", meta = (ClampMin = "0.0"))
	float ReloadTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Weapon", meta = (ClampMin = "0.0"))
	float RateOfFire = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Weapon", meta = (ClampMin = "0.0"))
	float Spread = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Weapon", meta = (ClampMin = "0.0"))
	float Recoil = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Weapon")
	EPNFireMode DefaultFireMode = EPNFireMode::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Weapon")
	TArray<EPNFireMode> FireModes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Weapon", meta = (ClampMin = "0.0"))
	float AggroRadius = 0.0f;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNArmorSlotData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Armor|Slots", meta = (ClampMin = "0"))
	int32 DefaultUnlockedSlots = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Armor|Slots", meta = (ClampMin = "0"))
	int32 MaxSlots = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Armor|Slots")
	bool bUsesPlate = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Armor|Slots")
	bool bUsesArtifacts = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Armor|Slots")
	bool bUsesMask = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Armor|Slots")
	bool bUsesHDevice = false;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNArmorStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Armor")
	FPNItemProtectionData Protection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Armor")
	FPNArmorSlotData Slots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Armor")
	FPNItemAttributeModifiers AttributeModifiers;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNBackpackStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Backpack", meta = (ClampMin = "0"))
	int32 MaxSlots = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Backpack", meta = (ClampMin = "0.0"))
	float MaxWeight = 0.0f;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNContainerLootEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Container")
	FName ItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Container", meta = (ClampMin = "1"))
	int32 MinCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Container", meta = (ClampMin = "1"))
	int32 MaxCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Container", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float Chance = 100.0f;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNContainerStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Container")
	bool bCanOpenOnlyIfEnoughInventorySpace = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Container")
	bool bCanOpenOnlyIfEnoughWeight = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Container")
	TArray<FPNContainerLootEntry> Loot;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNConsumableStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Consumable")
	EPNAnimType UseAnimType = EPNAnimType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Consumable")
	float HealthMax = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Consumable")
	float HealthMin = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Consumable")
	float Toxicity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Consumable")
	float WaterMax = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Consumable")
	float WaterMin = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Consumable")
	float FoodMax = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Consumable")
	float FoodMin = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Consumable")
	float StaminaMax = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Consumable")
	float StaminaMin = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Consumable")
	float CureToxicity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Consumable")
	float CurePsi = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Consumable")
	float CureRadiation = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Consumable")
	float CureBleeding = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Consumable")
	FPNItemAttributeModifiers AttributeModifiers;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNMagazineStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Magazine")
	FName AmmoType = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Magazine", meta = (ClampMin = "0"))
	int32 ClipSize = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Magazine", meta = (ClampMin = "0"))
	int32 ClipSizeMin = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Magazine", meta = (ClampMin = "0"))
	int32 ClipSizeMax = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Magazine")
	TArray<FName> CompatibleWeaponIds;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNWeaponAttachmentStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|WeaponAttachment")
	TArray<FName> CompatibleWeaponIds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|WeaponAttachment")
	bool bCanUseWithASRType1 = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|WeaponAttachment")
	bool bCanUseWithASRType2 = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|WeaponAttachment")
	bool bCanUseWithSniper = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|WeaponAttachment")
	bool bCanUseWithShotgun = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|WeaponAttachment")
	bool bCanUseWithSMG = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|WeaponAttachment")
	bool bCanUseWithPistol = false;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNArmorPlateStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|ArmorPlate", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float ShieldPercent = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|ArmorPlate")
	FPNFloatModifier BulletResistance;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNArmorAttachmentStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|ArmorAttachment")
	TArray<FName> CompatibleArmorIds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|ArmorAttachment")
	TArray<FName> CompatibleHelmetIds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|ArmorAttachment")
	bool bProvidesProtection = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|ArmorAttachment")
	FPNItemProtectionData Protection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|ArmorAttachment")
	FPNItemAttributeModifiers AttributeModifiers;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNQuestItemStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Quest")
	FName QuestId = NAME_None;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNCraftIngredient
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Craft")
	FName ItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Craft", meta = (ClampMin = "1"))
	int32 Count = 1;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNRecipeStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Recipe")
	TArray<FPNCraftIngredient> RequiredItems;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Recipe")
	FName ResultItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Recipe", meta = (ClampMin = "1"))
	int32 ResultCount = 1;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNCraftStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Craft")
	bool bCanCraft = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Craft")
	TArray<FName> LinkedRecipeIds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Craft")
	TArray<FPNCraftIngredient> RequiredItems;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNUsableStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Usable")
	EPNAnimType UseAnimType = EPNAnimType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Usable")
	bool bCanPlaceWorldObject = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Usable")
	bool bUsesDeviceLogic = false;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNCharacterPartStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Character")
	bool bBuyOnlyFromStore = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Character")
	bool bCanEditInMainMenu = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Character")
	bool bCanEditOnServerCustomization = true;
};

USTRUCT(BlueprintType)
struct PROJECTNOVA_API FPNBuildStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Build")
	EPNAnimType PlaceAnimType = EPNAnimType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Build")
	bool bCanCraft = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Build")
	bool bCanPlaceInWorld = true;
};