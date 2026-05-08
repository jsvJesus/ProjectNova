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

	PlayerHUDWidgetClass = UPNPlayerHUDWidget::StaticClass();
	InventoryHUDWidgetClass = UPNInventoryHUDWidget::StaticClass();

	PlayerHUDZOrder = 0;
	InventoryHUDZOrder = 10;
}

void APNPlayerController::BeginPlay()
{
	Super::BeginPlay();

	SetGameInputMode();

	if (IsLocalController())
	{
		CreatePlayerHUD();
		CreateInventoryHUD();
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
		InitializeHUDWidgetsForCurrentPawn();
	}
}

void APNPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();

	if (IsLocalController())
	{
		InitializeHUDWidgetsForCurrentPawn();
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
		InitializePlayerHUDWidgetForCurrentPawn();
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

	InitializePlayerHUDWidgetForCurrentPawn();
}

void APNPlayerController::CreateInventoryHUD()
{
	if (!IsLocalController())
	{
		return;
	}

	if (InventoryHUDWidget)
	{
		InitializeInventoryHUDWidgetForCurrentPawn();
		return;
	}

	TSubclassOf<UPNInventoryHUDWidget> WidgetClassToUse = InventoryHUDWidgetClass;
	if (!WidgetClassToUse)
	{
		WidgetClassToUse = UPNInventoryHUDWidget::StaticClass();
	}

	InventoryHUDWidget = CreateWidget<UPNInventoryHUDWidget>(this, WidgetClassToUse);
	if (!InventoryHUDWidget)
	{
		return;
	}

	InventoryHUDWidget->AddToViewport(InventoryHUDZOrder);
	InventoryHUDWidget->SetInventoryVisible(false);
	InventoryHUDWidget->SetVisibility(ESlateVisibility::Collapsed);

	InitializeInventoryHUDWidgetForCurrentPawn();
}

void APNPlayerController::RemovePlayerHUD()
{
	if (PlayerHUDWidget)
	{
		PlayerHUDWidget->RemoveFromParent();
		PlayerHUDWidget = nullptr;
	}

	if (InventoryHUDWidget)
	{
		InventoryHUDWidget->RemoveFromParent();
		InventoryHUDWidget = nullptr;
	}

	SetGameInputMode();
}

void APNPlayerController::RefreshPlayerHUD()
{
	if (!PlayerHUDWidget)
	{
		CreatePlayerHUD();
	}

	if (!InventoryHUDWidget)
	{
		CreateInventoryHUD();
	}

	if (PlayerHUDWidget)
	{
		PlayerHUDWidget->RefreshFromHUDComponent();
	}

	if (InventoryHUDWidget)
	{
		InventoryHUDWidget->RefreshFromHUDComponent();
	}
}

void APNPlayerController::SetInventoryHUDVisible(bool bVisible)
{
	if (!IsLocalController())
	{
		return;
	}

	if (!InventoryHUDWidget)
	{
		CreateInventoryHUD();
	}

	if (!InventoryHUDWidget)
	{
		return;
	}

	InventoryHUDWidget->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	InventoryHUDWidget->SetInventoryVisible(bVisible);

	if (bVisible)
	{
		InventoryHUDWidget->RefreshFromHUDComponent();

		SetGameAndUIInputMode();

		InventoryHUDWidget->SetUserFocus(this);
		InventoryHUDWidget->SetKeyboardFocus();
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

	if (!InventoryHUDWidget)
	{
		CreateInventoryHUD();
	}

	if (!InventoryHUDWidget)
	{
		return;
	}

	SetInventoryHUDVisible(!InventoryHUDWidget->IsInventoryVisible());
}

UPNPlayerHUDWidget* APNPlayerController::GetPlayerHUDWidget() const
{
	return PlayerHUDWidget;
}

UPNInventoryHUDWidget* APNPlayerController::GetInventoryHUDWidget() const
{
	return InventoryHUDWidget;
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

void APNPlayerController::InitializeHUDWidgetsForCurrentPawn()
{
	InitializePlayerHUDWidgetForCurrentPawn();
	InitializeInventoryHUDWidgetForCurrentPawn();
}

void APNPlayerController::InitializePlayerHUDWidgetForCurrentPawn()
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

void APNPlayerController::InitializeInventoryHUDWidgetForCurrentPawn()
{
	if (!InventoryHUDWidget)
	{
		return;
	}

	APNBaseCharacter* PNCharacter = GetPNBaseCharacter();
	UPNPlayerHUDComponent* PlayerHUDComponent = PNCharacter
		? PNCharacter->GetPlayerHUDComponent()
		: nullptr;

	InventoryHUDWidget->InitializeWithHUDComponent(PlayerHUDComponent);
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

	if (InventoryHUDWidget)
	{
		InputMode.SetWidgetToFocus(InventoryHUDWidget->TakeWidget());
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