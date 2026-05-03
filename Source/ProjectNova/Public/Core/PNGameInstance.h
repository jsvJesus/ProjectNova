#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Framework/PNGameModeTypes.h"
#include "PNGameInstance.generated.h"

UCLASS()
class PROJECTNOVA_API UPNGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	virtual void Shutdown() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	FString AccountId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	FString PlayerNickname = TEXT("Player");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	FName SelectedCharacterId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game")
	EPNGameModeType SelectedGameModeType = EPNGameModeType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game")
	FName SelectedMapName = NAME_None;

public:
	UFUNCTION(BlueprintCallable, Category = "GameInstance")
	void SetAccountId(const FString& NewAccountId);

	UFUNCTION(BlueprintCallable, Category = "GameInstance")
	void SetPlayerNickname(const FString& NewNickname);

	UFUNCTION(BlueprintCallable, Category = "GameInstance")
	void SetSelectedCharacterId(FName NewCharacterId);

	UFUNCTION(BlueprintCallable, Category = "GameInstance")
	void SetSelectedGameModeType(EPNGameModeType NewGameModeType);

	UFUNCTION(BlueprintCallable, Category = "GameInstance")
	void SetSelectedMapName(FName NewMapName);

	UFUNCTION(BlueprintPure, Category = "GameInstance")
	bool HasSelectedCharacter() const;

	UFUNCTION(BlueprintPure, Category = "GameInstance")
	bool IsRaidModeSelected() const;

	UFUNCTION(BlueprintPure, Category = "GameInstance")
	bool IsTeamModeSelected() const;
};