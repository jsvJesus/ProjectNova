#include "Framework/PNGameStateBase.h"

#include "Net/UnrealNetwork.h"

APNGameStateBase::APNGameStateBase()
{
	bReplicates = true;
}

void APNGameStateBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		ServerWorldTimeAtStart = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	}
}

void APNGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APNGameStateBase, CurrentGameModeType);
	DOREPLIFETIME(APNGameStateBase, MatchPhase);
	DOREPLIFETIME(APNGameStateBase, ServerWorldTimeAtStart);
	DOREPLIFETIME(APNGameStateBase, AlivePlayers);
	DOREPLIFETIME(APNGameStateBase, ExtractedPlayers);
	DOREPLIFETIME(APNGameStateBase, DeadPlayers);
}

EPNGameModeType APNGameStateBase::GetCurrentGameModeType() const
{
	return CurrentGameModeType;
}

EPNMatchPhase APNGameStateBase::GetMatchPhase() const
{
	return MatchPhase;
}

float APNGameStateBase::GetServerWorldTimeAtStart() const
{
	return ServerWorldTimeAtStart;
}

int32 APNGameStateBase::GetAlivePlayers() const
{
	return AlivePlayers;
}

int32 APNGameStateBase::GetExtractedPlayers() const
{
	return ExtractedPlayers;
}

int32 APNGameStateBase::GetDeadPlayers() const
{
	return DeadPlayers;
}

void APNGameStateBase::SetCurrentGameModeType(EPNGameModeType NewGameModeType)
{
	if (!HasAuthority())
	{
		return;
	}

	CurrentGameModeType = NewGameModeType;
	OnRep_GameModeType();
}

void APNGameStateBase::SetMatchPhase(EPNMatchPhase NewMatchPhase)
{
	if (!HasAuthority())
	{
		return;
	}

	MatchPhase = NewMatchPhase;
	OnRep_MatchPhase();
}

void APNGameStateBase::SetPlayerCounters(int32 NewAlivePlayers, int32 NewExtractedPlayers, int32 NewDeadPlayers)
{
	if (!HasAuthority())
	{
		return;
	}

	AlivePlayers = FMath::Max(0, NewAlivePlayers);
	ExtractedPlayers = FMath::Max(0, NewExtractedPlayers);
	DeadPlayers = FMath::Max(0, NewDeadPlayers);
}

void APNGameStateBase::OnRep_GameModeType()
{
}

void APNGameStateBase::OnRep_MatchPhase()
{
}