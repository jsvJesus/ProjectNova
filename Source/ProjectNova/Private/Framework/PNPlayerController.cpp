#include "Framework/PNPlayerController.h"

#include "Framework/PNGameModeBase.h"
#include "Framework/PNPlayerState.h"

APNPlayerController::APNPlayerController()
{
	bReplicates = true;
}

void APNPlayerController::BeginPlay()
{
	Super::BeginPlay();

	SetGameInputMode();
}

void APNPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
}

APNPlayerState* APNPlayerController::GetPNPlayerState() const
{
	return GetPlayerState<APNPlayerState>();
}

void APNPlayerController::SetGameInputMode()
{
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);

	bShowMouseCursor = false;
}

void APNPlayerController::RequestRespawn()
{
	if (HasAuthority())
	{
		Server_RequestRespawn_Implementation();
		return;
	}

	Server_RequestRespawn();
}

void APNPlayerController::Server_RequestRespawn_Implementation()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	APNGameModeBase* PNGameMode = World->GetAuthGameMode<APNGameModeBase>();
	if (!PNGameMode)
	{
		return;
	}

	PNGameMode->RequestPlayerRespawn(this);
}

void APNPlayerController::Client_ShowSystemMessage_Implementation(const FText& Message)
{
	UE_LOG(LogTemp, Log, TEXT("[ProjectNova] %s"), *Message.ToString());
}