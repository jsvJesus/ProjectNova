#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PNPlayerController.generated.h"

class APNBaseCharacter;
class APNPlayerState;
class UPNPlayerHUDWidget;
class UPNInventoryHUDWidget;

UCLASS()
class PROJECTNOVA_API APNPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	APNPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnRep_Pawn() override;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player HUD")
	TSubclassOf<UPNPlayerHUDWidget> PlayerHUDWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player HUD")
	TSubclassOf<UPNInventoryHUDWidget> InventoryHUDWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player HUD")
	int32 PlayerHUDZOrder = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player HUD")
	int32 InventoryHUDZOrder = 10;

protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Player HUD")
	TObjectPtr<UPNPlayerHUDWidget> PlayerHUDWidget = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Player HUD")
	TObjectPtr<UPNInventoryHUDWidget> InventoryHUDWidget = nullptr;

public:
	UFUNCTION(BlueprintPure, Category = "Player Controller")
	APNPlayerState* GetPNPlayerState() const;

	UFUNCTION(BlueprintPure, Category = "Player Controller")
	APNBaseCharacter* GetPNBaseCharacter() const;

	UFUNCTION(BlueprintCallable, Category = "Player Controller")
	void SetGameInputMode();

	UFUNCTION(BlueprintCallable, Category = "Player HUD")
	void CreatePlayerHUD();

	UFUNCTION(BlueprintCallable, Category = "Player HUD")
	void CreateInventoryHUD();

	UFUNCTION(BlueprintCallable, Category = "Player HUD")
	void RemovePlayerHUD();

	UFUNCTION(BlueprintCallable, Category = "Player HUD")
	void RefreshPlayerHUD();

	UFUNCTION(BlueprintCallable, Category = "Player HUD")
	void SetInventoryHUDVisible(bool bVisible);

	UFUNCTION(BlueprintCallable, Category = "Player HUD")
	void ToggleInventoryHUD();

	UFUNCTION(BlueprintPure, Category = "Player HUD")
	UPNPlayerHUDWidget* GetPlayerHUDWidget() const;

	UFUNCTION(BlueprintPure, Category = "Player HUD")
	UPNInventoryHUDWidget* GetInventoryHUDWidget() const;

	UFUNCTION(BlueprintCallable, Category = "Player Controller")
	void RequestRespawn();

	UFUNCTION(Server, Reliable)
	void Server_RequestRespawn();

	UFUNCTION(Client, Reliable)
	void Client_ShowSystemMessage(const FText& Message);

protected:
	void InitializeHUDWidgetsForCurrentPawn();
	void InitializePlayerHUDWidgetForCurrentPawn();
	void InitializeInventoryHUDWidgetForCurrentPawn();

	// Старое имя оставляем как wrapper, чтобы не ловить ошибки при частичной замене файлов.
	void InitializeHUDWidgetForCurrentPawn();

	void SetGameAndUIInputMode();
	void HandleToggleInventoryHUDInput();
};