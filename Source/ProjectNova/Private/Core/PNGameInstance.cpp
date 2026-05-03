#include "Core/PNGameInstance.h"

void UPNGameInstance::Init()
{
	Super::Init();

	UE_LOG(LogTemp, Log, TEXT("ProjectNova GameInstance Init"));
}

void UPNGameInstance::Shutdown()
{
	UE_LOG(LogTemp, Log, TEXT("ProjectNova GameInstance Shutdown"));

	Super::Shutdown();
}

void UPNGameInstance::SetAccountId(const FString& NewAccountId)
{
	AccountId = NewAccountId;
}

void UPNGameInstance::SetPlayerNickname(const FString& NewNickname)
{
	PlayerNickname = NewNickname.IsEmpty() ? TEXT("Player") : NewNickname;
}

void UPNGameInstance::SetSelectedCharacterId(FName NewCharacterId)
{
	SelectedCharacterId = NewCharacterId;
}

void UPNGameInstance::SetSelectedGameModeType(EPNGameModeType NewGameModeType)
{
	SelectedGameModeType = NewGameModeType;
}

void UPNGameInstance::SetSelectedMapName(FName NewMapName)
{
	SelectedMapName = NewMapName;
}

bool UPNGameInstance::HasSelectedCharacter() const
{
	return !SelectedCharacterId.IsNone();
}

bool UPNGameInstance::IsRaidModeSelected() const
{
	return SelectedGameModeType == EPNGameModeType::SoloRaidPVE
		|| SelectedGameModeType == EPNGameModeType::TeamRaidPVP;
}

bool UPNGameInstance::IsTeamModeSelected() const
{
	return SelectedGameModeType == EPNGameModeType::TeamRaidPVP
		|| SelectedGameModeType == EPNGameModeType::DeathMatch;
}