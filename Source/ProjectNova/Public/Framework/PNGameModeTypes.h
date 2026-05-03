#pragma once

#include "CoreMinimal.h"
#include "PNGameModeTypes.generated.h"

UENUM(BlueprintType)
enum class EPNGameModeType : uint8
{
	None        UMETA(DisplayName = "None"),

	SoloRaidPVE UMETA(DisplayName = "Solo Raid PVE"),
	TeamRaidPVP UMETA(DisplayName = "Team Raid PVP/PVE"),
	OpenWorld   UMETA(DisplayName = "Open World"),
	DeathMatch  UMETA(DisplayName = "DeathMatch")
};

UENUM(BlueprintType)
enum class EPNMatchPhase : uint8
{
	None              UMETA(DisplayName = "None"),

	WaitingForPlayers UMETA(DisplayName = "Waiting For Players"),
	Preparing         UMETA(DisplayName = "Preparing"),
	InProgress        UMETA(DisplayName = "In Progress"),
	ExtractionOpen    UMETA(DisplayName = "Extraction Open"),
	Ending            UMETA(DisplayName = "Ending"),
	Finished          UMETA(DisplayName = "Finished")
};

UENUM(BlueprintType)
enum class EPNRaidState : uint8
{
	None         UMETA(DisplayName = "None"),

	Alive        UMETA(DisplayName = "Alive"),
	Dead         UMETA(DisplayName = "Dead"),
	Extracted    UMETA(DisplayName = "Extracted"),
	Disconnected UMETA(DisplayName = "Disconnected")
};