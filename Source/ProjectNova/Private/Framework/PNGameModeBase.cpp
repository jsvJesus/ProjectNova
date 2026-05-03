#include "Framework/PNGameModeBase.h"

#include "Characters/PNPlayerCharacter.h"
#include "Framework/PNGameStateBase.h"
#include "Framework/PNPlayerController.h"
#include "Framework/PNPlayerState.h"

APNGameModeBase::APNGameModeBase()
{
	GameStateClass = APNGameStateBase::StaticClass();
	PlayerControllerClass = APNPlayerController::StaticClass();
	PlayerStateClass = APNPlayerState::StaticClass();
	DefaultPawnClass = APNPlayerCharacter::StaticClass();

	GameModeType = EPNGameModeType::None;
	bAllowRespawn = true;
	bIsPVPEnabled = false;
	bIsPVEEnabled = true;
	bUseExtractionRules = false;
}

void APNGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	APNGameStateBase* PNGameState = GetGameState<APNGameStateBase>();
	if (PNGameState)
	{
		PNGameState->SetCurrentGameModeType(GameModeType);
		PNGameState->SetMatchPhase(EPNMatchPhase::InProgress);
	}

	RefreshGameStateCounters();
}

void APNGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	APNPlayerController* PNPlayerController = Cast<APNPlayerController>(NewPlayer);
	if (PNPlayerController)
	{
		InitializePlayerState(PNPlayerController);
		PNPlayerController->Client_ShowSystemMessage(FText::FromString(TEXT("Connected to ProjectNova server.")));
	}

	RefreshGameStateCounters();
}

void APNGameModeBase::Logout(AController* Exiting)
{
	if (APNPlayerState* PNPlayerState = Exiting ? Exiting->GetPlayerState<APNPlayerState>() : nullptr)
	{
		PNPlayerState->SetRaidState(EPNRaidState::Disconnected);
	}

	Super::Logout(Exiting);

	RefreshGameStateCounters();
}

void APNGameModeBase::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	APNPlayerController* PNPlayerController = Cast<APNPlayerController>(NewPlayer);
	if (PNPlayerController)
	{
		InitializePlayerState(PNPlayerController);
	}
}

EPNGameModeType APNGameModeBase::GetGameModeType() const
{
	return GameModeType;
}

bool APNGameModeBase::IsPVPEnabled() const
{
	return bIsPVPEnabled;
}

bool APNGameModeBase::IsPVEEnabled() const
{
	return bIsPVEEnabled;
}

bool APNGameModeBase::UsesExtractionRules() const
{
	return bUseExtractionRules;
}

bool APNGameModeBase::CanPlayerRespawn(APNPlayerController* PlayerController) const
{
	if (!PlayerController)
	{
		return false;
	}

	if (!bAllowRespawn)
	{
		return false;
	}

	return true;
}

void APNGameModeBase::RequestPlayerRespawn(APNPlayerController* PlayerController)
{
	if (!PlayerController)
	{
		return;
	}

	if (!CanPlayerRespawn(PlayerController))
	{
		PlayerController->Client_ShowSystemMessage(FText::FromString(TEXT("Respawn is not allowed in this mode.")));
		return;
	}

	RestartPlayer(PlayerController);

	if (APNPlayerState* PNPlayerState = PlayerController->GetPNPlayerState())
	{
		PNPlayerState->SetRaidState(EPNRaidState::Alive);
	}

	RefreshGameStateCounters();
}

void APNGameModeBase::InitializePlayerState(APNPlayerController* PlayerController)
{
	if (!PlayerController)
	{
		return;
	}

	APNPlayerState* PNPlayerState = PlayerController->GetPNPlayerState();
	if (!PNPlayerState)
	{
		return;
	}

	if (PNPlayerState->GetRaidState() == EPNRaidState::None)
	{
		PNPlayerState->SetRaidState(EPNRaidState::Alive);
	}

	if (PNPlayerState->GetNickname().IsEmpty())
	{
		PNPlayerState->SetNickname(TEXT("Player"));
	}
}

void APNGameModeBase::RefreshGameStateCounters()
{
	APNGameStateBase* PNGameState = GetGameState<APNGameStateBase>();
	if (!PNGameState)
	{
		return;
	}

	int32 AliveCount = 0;
	int32 ExtractedCount = 0;
	int32 DeadCount = 0;

	for (APlayerState* RawPlayerState : GameState->PlayerArray)
	{
		APNPlayerState* PNPlayerState = Cast<APNPlayerState>(RawPlayerState);
		if (!PNPlayerState)
		{
			continue;
		}

		switch (PNPlayerState->GetRaidState())
		{
		case EPNRaidState::Alive:
			++AliveCount;
			break;

		case EPNRaidState::Extracted:
			++ExtractedCount;
			break;

		case EPNRaidState::Dead:
			++DeadCount;
			break;

		default:
			break;
		}
	}

	PNGameState->SetPlayerCounters(AliveCount, ExtractedCount, DeadCount);
}