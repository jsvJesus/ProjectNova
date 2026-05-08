#include "Framework/PNPlayerController.h"

#include "Characters/PNBaseCharacter.h"
#include "Framework/PNGameModeBase.h"
#include "Framework/PNPlayerState.h"
#include "InputCoreTypes.h"
#include "UI/PNPlayerHUDComponent.h"
#include "UI/PNPlayerHUDWidget.h"
#include "UI/Inventory/PNInventoryHUDWidget.h"

APNPlayerController::APNPlayerController()
{
	bReplicates = true;

	PlayerHUDWidgetClass = UPNInventoryHUDWidget::StaticClass();
}

void APNPlayerController::BeginPlay()
{
	Super::BeginPlay();

	SetGameInputMode();

	if (IsLocalController())
	{
		CreatePlayerHUD();
	}
}

void APNPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (!InputComponent)
	{
		return;
	}

	FInputKeyBinding ToggleInventoryKeyBinding(EKeys::I, IE_Pressed);
	ToggleInventoryKeyBinding.KeyDelegate.GetDelegateForManualSet().BindUObject(
		this,
		&APNPlayerController::HandleToggleInventoryHUDInput
	);
	ToggleInventoryKeyBinding.bConsumeInput = true;

	InputComponent->KeyBindings.Add(ToggleInventoryKeyBinding);
}

void APNPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (IsLocalController())
	{
		InitializeHUDWidgetForCurrentPawn();
	}
}

void APNPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();

	if (IsLocalController())
	{
		InitializeHUDWidgetForCurrentPawn();
	}
}

APNPlayerState* APNPlayerController::GetPNPlayerState() const
{
	return GetPlayerState<APNPlayerState>();
}

APNBaseCharacter* APNPlayerController::GetPNBaseCharacter() const
{
	return Cast<APNBaseCharacter>(GetPawn());
}

void APNPlayerController::SetGameInputMode()
{
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);

	bShowMouseCursor = false;
}

void APNPlayerController::CreatePlayerHUD()
{
	if (!IsLocalController())
	{
		return;
	}

	if (PlayerHUDWidget)
	{
		InitializeHUDWidgetForCurrentPawn();
		return;
	}

	TSubclassOf<UPNPlayerHUDWidget> WidgetClassToUse = PlayerHUDWidgetClass;
	if (!WidgetClassToUse)
	{
		WidgetClassToUse = UPNPlayerHUDWidget::StaticClass();
	}

	PlayerHUDWidget = CreateWidget<UPNPlayerHUDWidget>(this, WidgetClassToUse);
	if (!PlayerHUDWidget)
	{
		return;
	}

	PlayerHUDWidget->AddToViewport(PlayerHUDZOrder);

	InitializeHUDWidgetForCurrentPawn();
}

void APNPlayerController::RemovePlayerHUD()
{
	if (!PlayerHUDWidget)
	{
		return;
	}

	PlayerHUDWidget->RemoveFromParent();
	PlayerHUDWidget = nullptr;

	SetGameInputMode();
}

void APNPlayerController::RefreshPlayerHUD()
{
	if (!PlayerHUDWidget)
	{
		CreatePlayerHUD();
		return;
	}

	PlayerHUDWidget->RefreshFromHUDComponent();
}

void APNPlayerController::SetInventoryHUDVisible(bool bVisible)
{
	if (!IsLocalController())
	{
		return;
	}

	if (!PlayerHUDWidget)
	{
		CreatePlayerHUD();
	}

	if (!PlayerHUDWidget)
	{
		return;
	}

	PlayerHUDWidget->SetInventoryVisible(bVisible);

	if (bVisible)
	{
		PlayerHUDWidget->RefreshFromHUDComponent();
		SetGameAndUIInputMode();

		PlayerHUDWidget->SetUserFocus(this);
		PlayerHUDWidget->SetKeyboardFocus();
	}
	else
	{
		SetGameInputMode();
	}
}

void APNPlayerController::ToggleInventoryHUD()
{
	if (!IsLocalController())
	{
		return;
	}

	if (!PlayerHUDWidget)
	{
		CreatePlayerHUD();
	}

	if (!PlayerHUDWidget)
	{
		return;
	}

	SetInventoryHUDVisible(!PlayerHUDWidget->IsInventoryVisible());
}

UPNPlayerHUDWidget* APNPlayerController::GetPlayerHUDWidget() const
{
	return PlayerHUDWidget;
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

void APNPlayerController::InitializeHUDWidgetForCurrentPawn()
{
	if (!PlayerHUDWidget)
	{
		return;
	}
	
	APNBaseCharacter* PNCharacter = GetPNBaseCharacter();
	UPNPlayerHUDComponent* PlayerHUDComponent = PNCharacter
		? PNCharacter->GetPlayerHUDComponent()
		: nullptr;

	PlayerHUDWidget->InitializeWithHUDComponent(PlayerHUDComponent);
}

void APNPlayerController::SetGameAndUIInputMode()
{
	FInputModeGameAndUI InputMode;

	if (PlayerHUDWidget)
	{
		InputMode.SetWidgetToFocus(PlayerHUDWidget->TakeWidget());
	}

	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);

	SetInputMode(InputMode);

	bShowMouseCursor = true;
}

void APNPlayerController::HandleToggleInventoryHUDInput()
{
	ToggleInventoryHUD();
}