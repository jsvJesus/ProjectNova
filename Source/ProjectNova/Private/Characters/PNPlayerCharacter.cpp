#include "Characters/PNPlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

APNPlayerCharacter::APNPlayerCharacter()
{
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;

	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCameraComponent"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 64.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
}

void APNPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void APNPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Enhanced Input подключим позже отдельным этапом.
	// Сейчас здесь пусто специально: сначала Core Framework, потом Interaction System.
}

UCameraComponent* APNPlayerCharacter::GetFirstPersonCameraComponent() const
{
	return FirstPersonCameraComponent;
}