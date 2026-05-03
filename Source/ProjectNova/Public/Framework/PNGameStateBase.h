#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Framework/PNGameModeTypes.h"
#include "PNGameStateBase.generated.h"

UCLASS()
class PROJECTNOVA_API APNGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	APNGameStateBase();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_GameModeType, BlueprintReadOnly, Category = "Game State")
	EPNGameModeType CurrentGameModeType = EPNGameModeType::None;

	UPROPERTY(ReplicatedUsing = OnRep_MatchPhase, BlueprintReadOnly, Category = "Game State")
	EPNMatchPhase MatchPhase = EPNMatchPhase::None;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game State")
	float ServerWorldTimeAtStart = 0.0f;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game State")
	int32 AlivePlayers = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game State")
	int32 ExtractedPlayers = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game State")
	int32 DeadPlayers = 0;

public:
	UFUNCTION(BlueprintPure, Category = "Game State")
	EPNGameModeType GetCurrentGameModeType() const;

	UFUNCTION(BlueprintPure, Category = "Game State")
	EPNMatchPhase GetMatchPhase() const;

	UFUNCTION(BlueprintPure, Category = "Game State")
	float GetServerWorldTimeAtStart() const;

	UFUNCTION(BlueprintPure, Category = "Game State")
	int32 GetAlivePlayers() const;

	UFUNCTION(BlueprintPure, Category = "Game State")
	int32 GetExtractedPlayers() const;

	UFUNCTION(BlueprintPure, Category = "Game State")
	int32 GetDeadPlayers() const;

	UFUNCTION(BlueprintCallable, Category = "Game State")
	void SetCurrentGameModeType(EPNGameModeType NewGameModeType);

	UFUNCTION(BlueprintCallable, Category = "Game State")
	void SetMatchPhase(EPNMatchPhase NewMatchPhase);

	UFUNCTION(BlueprintCallable, Category = "Game State")
	void SetPlayerCounters(int32 NewAlivePlayers, int32 NewExtractedPlayers, int32 NewDeadPlayers);

protected:
	UFUNCTION()
	void OnRep_GameModeType();

	UFUNCTION()
	void OnRep_MatchPhase();
};