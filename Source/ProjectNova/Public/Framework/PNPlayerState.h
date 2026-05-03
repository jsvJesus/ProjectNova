#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Framework/PNGameModeTypes.h"
#include "PNPlayerState.generated.h"

UCLASS()
class PROJECTNOVA_API APNPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	APNPlayerState();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player State")
	FString AccountId;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player State")
	FString Nickname = TEXT("Player");

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player State")
	FName CharacterId = NAME_None;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Team")
	int32 TeamId = INDEX_NONE;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Team")
	int32 SquadId = INDEX_NONE;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Clan")
	int32 ClanId = INDEX_NONE;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Stats")
	int32 Kills = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Stats")
	int32 Deaths = 0;

	UPROPERTY(ReplicatedUsing = OnRep_RaidState, BlueprintReadOnly, Category = "Raid")
	EPNRaidState RaidState = EPNRaidState::None;

public:
	UFUNCTION(BlueprintPure, Category = "Player State")
	const FString& GetAccountId() const;

	UFUNCTION(BlueprintPure, Category = "Player State")
	const FString& GetNickname() const;

	UFUNCTION(BlueprintPure, Category = "Player State")
	FName GetCharacterId() const;

	UFUNCTION(BlueprintPure, Category = "Team")
	int32 GetTeamId() const;

	UFUNCTION(BlueprintPure, Category = "Team")
	int32 GetSquadId() const;

	UFUNCTION(BlueprintPure, Category = "Clan")
	int32 GetClanId() const;

	UFUNCTION(BlueprintPure, Category = "Stats")
	int32 GetKills() const;

	UFUNCTION(BlueprintPure, Category = "Stats")
	int32 GetDeaths() const;

	UFUNCTION(BlueprintPure, Category = "Raid")
	EPNRaidState GetRaidState() const;

	UFUNCTION(BlueprintPure, Category = "Raid")
	bool IsAliveInRaid() const;

	UFUNCTION(BlueprintPure, Category = "Team")
	bool IsSameTeam(const APNPlayerState* OtherPlayerState) const;

	UFUNCTION(BlueprintCallable, Category = "Player State")
	void SetAccountId(const FString& NewAccountId);

	UFUNCTION(BlueprintCallable, Category = "Player State")
	void SetNickname(const FString& NewNickname);

	UFUNCTION(BlueprintCallable, Category = "Player State")
	void SetCharacterId(FName NewCharacterId);

	UFUNCTION(BlueprintCallable, Category = "Team")
	void SetTeamData(int32 NewTeamId, int32 NewSquadId, int32 NewClanId);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void AddKill();

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void AddDeath();

	UFUNCTION(BlueprintCallable, Category = "Raid")
	void SetRaidState(EPNRaidState NewRaidState);

protected:
	UFUNCTION()
	void OnRep_RaidState();
};