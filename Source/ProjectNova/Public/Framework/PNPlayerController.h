#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PNPlayerController.generated.h"

class APNPlayerState;

UCLASS()
class PROJECTNOVA_API APNPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	APNPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

public:
	UFUNCTION(BlueprintPure, Category = "Player Controller")
	APNPlayerState* GetPNPlayerState() const;

	UFUNCTION(BlueprintCallable, Category = "Player Controller")
	void SetGameInputMode();

	UFUNCTION(BlueprintCallable, Category = "Player Controller")
	void RequestRespawn();

	UFUNCTION(Server, Reliable)
	void Server_RequestRespawn();

	UFUNCTION(Client, Reliable)
	void Client_ShowSystemMessage(const FText& Message);
};