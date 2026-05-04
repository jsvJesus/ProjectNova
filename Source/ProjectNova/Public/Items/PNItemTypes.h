#pragma once

#include "CoreMinimal.h"
#include "PNItemTypes.generated.h"

UENUM(BlueprintType)
enum class EPNItemType : uint8
{
	None            UMETA(DisplayName = "None"),

	IT_Weapon       UMETA(DisplayName = "Weapon"),
	IT_Armor        UMETA(DisplayName = "Armor"),
	IT_HArmor       UMETA(DisplayName = "Helmet Armor"),
	IT_Backpack     UMETA(DisplayName = "Backpack"),
	IT_Container    UMETA(DisplayName = "Container"),
	IT_Consumables  UMETA(DisplayName = "Consumables"),
	IT_Weapon_ATTM  UMETA(DisplayName = "Weapon Attachment"),
	IT_Armor_ATTM   UMETA(DisplayName = "Armor Attachment"),
	IT_Items        UMETA(DisplayName = "Items"),
	IT_Characters   UMETA(DisplayName = "Characters"),
	IT_Builds       UMETA(DisplayName = "Builds"),

	IT_Gloves       UMETA(DisplayName = "Gloves")
};

UENUM(BlueprintType)
enum class EPNItemCategory : uint8
{
	None UMETA(DisplayName = "None"),

	// Weapon
	ASR_Type1       UMETA(DisplayName = "Assault Rifle Type 1"),
	ASR_Type2       UMETA(DisplayName = "Assault Rifle Type 2"),
	SNP             UMETA(DisplayName = "Sniper Rifle"),
	SHTG            UMETA(DisplayName = "Shotgun"),
	SMG             UMETA(DisplayName = "SMG"),
	HG_Single       UMETA(DisplayName = "Handgun Single"),
	HG_Knife        UMETA(DisplayName = "Handgun Knife"),
	HG_Shield       UMETA(DisplayName = "Handgun Shield"),
	HG_Items        UMETA(DisplayName = "Handgun Item"),
	MG              UMETA(DisplayName = "Machine Gun"),
	Melee           UMETA(DisplayName = "Melee"),
	RPG             UMETA(DisplayName = "RPG"),

	// Armor / Equipment
	Armor           UMETA(DisplayName = "Armor"),
	Helmet          UMETA(DisplayName = "Helmet"),
	Backpack        UMETA(DisplayName = "Backpack"),

	// Container
	Container       UMETA(DisplayName = "Container"),

	// Consumables
	Food            UMETA(DisplayName = "Food"),
	Water           UMETA(DisplayName = "Water"),
	Medicine        UMETA(DisplayName = "Medicine"),

	// Weapon Attachments
	Scope           UMETA(DisplayName = "Scope"),
	Mag             UMETA(DisplayName = "Magazine"),
	Laser           UMETA(DisplayName = "Laser"),
	FlashLight      UMETA(DisplayName = "Flashlight"),
	Grip            UMETA(DisplayName = "Grip"),
	Muzzle          UMETA(DisplayName = "Muzzle"),

	// Armor Attachments
	Plate           UMETA(DisplayName = "Armor Plate"),
	Artifacts       UMETA(DisplayName = "Artifact"),
	Mask            UMETA(DisplayName = "Mask"),
	HDevice         UMETA(DisplayName = "Helmet Device"),

	// Items
	Quest           UMETA(DisplayName = "Quest Item"),
	Craft           UMETA(DisplayName = "Craft Item"),
	Components      UMETA(DisplayName = "Components"),
	Resource        UMETA(DisplayName = "Resource"),
	Recipes         UMETA(DisplayName = "Recipe"),
	Usable          UMETA(DisplayName = "Usable"),

	// Characters
	Head            UMETA(DisplayName = "Head"),
	Body            UMETA(DisplayName = "Body"),
	Legs            UMETA(DisplayName = "Legs"),

	// Builds
	Parts           UMETA(DisplayName = "Build Parts"),

	// Equipment
	Gloves          UMETA(DisplayName = "Gloves")
};

UENUM(BlueprintType)
enum class EPNAnimType : uint8
{
	None            UMETA(DisplayName = "None"),

	Unarmed         UMETA(DisplayName = "Unarmed"),
	Gun             UMETA(DisplayName = "Gun"),
	Rifle_1         UMETA(DisplayName = "Rifle 1"),
	Rifle_2         UMETA(DisplayName = "Rifle 2"),
	Shotgun         UMETA(DisplayName = "Shotgun"),
	Pistol_Single   UMETA(DisplayName = "Pistol Single"),
	Pistol_Knife    UMETA(DisplayName = "Pistol Knife"),
	Pistol_Shield   UMETA(DisplayName = "Pistol Shield"),
	RPG             UMETA(DisplayName = "RPG"),
	Knife           UMETA(DisplayName = "Knife"),
	Sniper          UMETA(DisplayName = "Sniper"),
	Spear           UMETA(DisplayName = "Spear"),
	FlashLight      UMETA(DisplayName = "Flashlight"),
	Binoculars      UMETA(DisplayName = "Binoculars"),
	Lighter         UMETA(DisplayName = "Lighter"),
	Box             UMETA(DisplayName = "Box"),
	Device          UMETA(DisplayName = "Device"),
	Candle          UMETA(DisplayName = "Candle"),
	Lantern_1       UMETA(DisplayName = "Lantern 1"),
	Lantern_2       UMETA(DisplayName = "Lantern 2"),
	Lantern_Knife   UMETA(DisplayName = "Lantern Knife"),
	GasSpray        UMETA(DisplayName = "Gas Spray")
};

UENUM(BlueprintType)
enum class EPNItemRarity : uint8
{
	None       UMETA(DisplayName = "None"),

	Common     UMETA(DisplayName = "Common"),
	Uncommon   UMETA(DisplayName = "Uncommon"),
	Rare       UMETA(DisplayName = "Rare"),
	Epic       UMETA(DisplayName = "Epic"),
	Legendary  UMETA(DisplayName = "Legendary"),
	Ancient    UMETA(DisplayName = "Ancient"),
	Broken     UMETA(DisplayName = "Broken")
};

UENUM(BlueprintType)
enum class EPNFireMode : uint8
{
	None      = 0   UMETA(DisplayName = "None"),

	Auto      = 100 UMETA(DisplayName = "Automatic"),
	SemiAuto  = 101 UMETA(DisplayName = "Semi Automatic"),
	Single    = 102 UMETA(DisplayName = "Single")
};

UENUM(BlueprintType)
enum class EPNProtectionArmor : uint8
{
	None UMETA(DisplayName = "None"),

	V1   UMETA(DisplayName = "Protection Level 1"),
	V2   UMETA(DisplayName = "Protection Level 2"),
	V3   UMETA(DisplayName = "Protection Level 3"),
	V4   UMETA(DisplayName = "Protection Level 4")
};

UENUM(BlueprintType)
enum class EPNScopeType : uint8
{
	None    UMETA(DisplayName = "None"),

	Assault UMETA(DisplayName = "Assault"),
	Sniper  UMETA(DisplayName = "Sniper"),
	Pistol  UMETA(DisplayName = "Pistol")
};