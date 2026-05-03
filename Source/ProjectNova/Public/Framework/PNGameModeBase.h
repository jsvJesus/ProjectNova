#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Framework/PNGameModeTypes.h"
#include "PNGameModeBase.generated.h"

class APNPlayerController;
class APNPlayerState;
class APNGameStateBase;

UCLASS()
class PROJECTNOVA_API APNGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	APNGameModeBase();

protected:
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Mode")
	EPNGameModeType GameModeType = EPNGameModeType::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Mode")
	bool bAllowRespawn = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Mode")
	bool bIsPVPEnabled = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Mode")
	bool bIsPVEEnabled = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Mode")
	bool bUseExtractionRules = false;

public:
	UFUNCTION(BlueprintPure, Category = "Game Mode")
	EPNGameModeType GetGameModeType() const;

	UFUNCTION(BlueprintPure, Category = "Game Mode")
	bool IsPVPEnabled() const;

	UFUNCTION(BlueprintPure, Category = "Game Mode")
	bool IsPVEEnabled() const;

	UFUNCTION(BlueprintPure, Category = "Game Mode")
	bool UsesExtractionRules() const;

	UFUNCTION(BlueprintPure, Category = "Game Mode")
	bool CanPlayerRespawn(APNPlayerController* PlayerController) const;

	UFUNCTION(BlueprintCallable, Category = "Game Mode")
	void RequestPlayerRespawn(APNPlayerController* PlayerController);

protected:
	void InitializePlayerState(APNPlayerController* PlayerController);
	void RefreshGameStateCounters();
};