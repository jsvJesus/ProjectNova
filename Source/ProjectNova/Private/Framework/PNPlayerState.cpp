#include "Framework/PNPlayerState.h"

#include "Net/UnrealNetwork.h"

APNPlayerState::APNPlayerState()
{
	bReplicates = true;
}

void APNPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APNPlayerState, AccountId);
	DOREPLIFETIME(APNPlayerState, Nickname);
	DOREPLIFETIME(APNPlayerState, CharacterId);
	DOREPLIFETIME(APNPlayerState, TeamId);
	DOREPLIFETIME(APNPlayerState, SquadId);
	DOREPLIFETIME(APNPlayerState, ClanId);
	DOREPLIFETIME(APNPlayerState, Kills);
	DOREPLIFETIME(APNPlayerState, Deaths);
	DOREPLIFETIME(APNPlayerState, RaidState);
}

const FString& APNPlayerState::GetAccountId() const
{
	return AccountId;
}

const FString& APNPlayerState::GetNickname() const
{
	return Nickname;
}

FName APNPlayerState::GetCharacterId() const
{
	return CharacterId;
}

int32 APNPlayerState::GetTeamId() const
{
	return TeamId;
}

int32 APNPlayerState::GetSquadId() const
{
	return SquadId;
}

int32 APNPlayerState::GetClanId() const
{
	return ClanId;
}

int32 APNPlayerState::GetKills() const
{
	return Kills;
}

int32 APNPlayerState::GetDeaths() const
{
	return Deaths;
}

EPNRaidState APNPlayerState::GetRaidState() const
{
	return RaidState;
}

bool APNPlayerState::IsAliveInRaid() const
{
	return RaidState == EPNRaidState::Alive;
}

bool APNPlayerState::IsSameTeam(const APNPlayerState* OtherPlayerState) const
{
	if (!OtherPlayerState)
	{
		return false;
	}

	if (TeamId == INDEX_NONE || OtherPlayerState->TeamId == INDEX_NONE)
	{
		return false;
	}

	return TeamId == OtherPlayerState->TeamId;
}

void APNPlayerState::SetAccountId(const FString& NewAccountId)
{
	if (!HasAuthority())
	{
		return;
	}

	AccountId = NewAccountId;
}

void APNPlayerState::SetNickname(const FString& NewNickname)
{
	if (!HasAuthority())
	{
		return;
	}

	Nickname = NewNickname.IsEmpty() ? TEXT("Player") : NewNickname;
	SetPlayerName(Nickname);
}

void APNPlayerState::SetCharacterId(FName NewCharacterId)
{
	if (!HasAuthority())
	{
		return;
	}

	CharacterId = NewCharacterId;
}

void APNPlayerState::SetTeamData(int32 NewTeamId, int32 NewSquadId, int32 NewClanId)
{
	if (!HasAuthority())
	{
		return;
	}

	TeamId = NewTeamId;
	SquadId = NewSquadId;
	ClanId = NewClanId;
}

void APNPlayerState::AddKill()
{
	if (!HasAuthority())
	{
		return;
	}

	++Kills;
}

void APNPlayerState::AddDeath()
{
	if (!HasAuthority())
	{
		return;
	}

	++Deaths;
}

void APNPlayerState::SetRaidState(EPNRaidState NewRaidState)
{
	if (!HasAuthority())
	{
		return;
	}

	RaidState = NewRaidState;
	OnRep_RaidState();
}

void APNPlayerState::OnRep_RaidState()
{
}