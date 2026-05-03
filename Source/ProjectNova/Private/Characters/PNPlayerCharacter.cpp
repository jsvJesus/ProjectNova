#include "Characters/PNPlayerCharacter.h"

#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

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

	FirstPersonArmsMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonArmsMeshComponent"));
	FirstPersonArmsMeshComponent->SetupAttachment(FirstPersonCameraComponent);
	FirstPersonArmsMeshComponent->SetRelativeLocation(FirstPersonArmsRelativeLocation);
	FirstPersonArmsMeshComponent->SetRelativeRotation(FirstPersonArmsRelativeRotation);
	FirstPersonArmsMeshComponent->SetOnlyOwnerSee(true);
	FirstPersonArmsMeshComponent->SetOwnerNoSee(false);
	FirstPersonArmsMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FirstPersonArmsMeshComponent->CastShadow = false;
	FirstPersonArmsMeshComponent->bCastDynamicShadow = false;
}

void APNPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	RefreshFirstPersonVisibility();
	ApplyFirstPersonArmsMesh();
	ApplyFirstPersonArmsAnimClass();
}

void APNPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (!PlayerInputComponent)
	{
		return;
	}

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &APNPlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &APNPlayerCharacter::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &APNPlayerCharacter::Turn);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &APNPlayerCharacter::LookUp);

	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &APNPlayerCharacter::StartJump);
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Released, this, &APNPlayerCharacter::StopJump);

	PlayerInputComponent->BindAction(TEXT("Sprint"), IE_Pressed, this, &APNPlayerCharacter::StartSprint);
	PlayerInputComponent->BindAction(TEXT("Sprint"), IE_Released, this, &APNPlayerCharacter::StopSprint);

	PlayerInputComponent->BindAction(TEXT("Crouch"), IE_Pressed, this, &APNPlayerCharacter::StartCrouchInput);
	PlayerInputComponent->BindAction(TEXT("Crouch"), IE_Released, this, &APNPlayerCharacter::StopCrouchInput);
}

void APNPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APNPlayerCharacter, FirstPersonArmsMeshAsset);
	DOREPLIFETIME(APNPlayerCharacter, FirstPersonAnimType);
}

UCameraComponent* APNPlayerCharacter::GetFirstPersonCameraComponent() const
{
	return FirstPersonCameraComponent;
}

USkeletalMeshComponent* APNPlayerCharacter::GetFirstPersonArmsMeshComponent() const
{
	return FirstPersonArmsMeshComponent;
}

EPNAnimType APNPlayerCharacter::GetFirstPersonAnimType() const
{
	return FirstPersonAnimType;
}

void APNPlayerCharacter::SetFirstPersonArmsMesh(USkeletalMesh* NewArmsMesh)
{
	if (!HasAuthority())
	{
		return;
	}

	FirstPersonArmsMeshAsset = NewArmsMesh;
	ApplyFirstPersonArmsMesh();
}

void APNPlayerCharacter::SetFirstPersonAnimType(EPNAnimType NewAnimType)
{
	if (HasAuthority())
	{
		FirstPersonAnimType = NewAnimType;
		OnRep_FirstPersonAnimType();
		return;
	}

	Server_SetFirstPersonAnimType(NewAnimType);
}

void APNPlayerCharacter::ApplyFirstPersonArmsMesh()
{
	if (!FirstPersonArmsMeshComponent)
	{
		return;
	}

	FirstPersonArmsMeshComponent->SetSkeletalMesh(FirstPersonArmsMeshAsset);
	FirstPersonArmsMeshComponent->SetOnlyOwnerSee(true);
	FirstPersonArmsMeshComponent->SetOwnerNoSee(false);
	FirstPersonArmsMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FirstPersonArmsMeshComponent->SetRelativeLocation(FirstPersonArmsRelativeLocation);
	FirstPersonArmsMeshComponent->SetRelativeRotation(FirstPersonArmsRelativeRotation);

	ApplyFirstPersonArmsAnimClass();
}

void APNPlayerCharacter::ApplyFirstPersonArmsAnimClass()
{
	if (!FirstPersonArmsMeshComponent)
	{
		return;
	}

	if (!FirstPersonArmsAnimClass)
	{
		return;
	}

	FirstPersonArmsMeshComponent->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	FirstPersonArmsMeshComponent->SetAnimInstanceClass(FirstPersonArmsAnimClass);
}

void APNPlayerCharacter::Server_SetFirstPersonAnimType_Implementation(EPNAnimType NewAnimType)
{
	SetFirstPersonAnimType(NewAnimType);
}

void APNPlayerCharacter::MoveForward(float Value)
{
	if (FMath::IsNearlyZero(Value))
	{
		return;
	}

	if (IsDead() || !Controller)
	{
		return;
	}

	const FRotator ControlRotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);
	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	AddMovementInput(Direction, Value);
}

void APNPlayerCharacter::MoveRight(float Value)
{
	if (FMath::IsNearlyZero(Value))
	{
		return;
	}

	if (IsDead() || !Controller)
	{
		return;
	}

	const FRotator ControlRotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);
	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(Direction, Value);
}

void APNPlayerCharacter::Turn(float Value)
{
	if (FMath::IsNearlyZero(Value))
	{
		return;
	}

	AddControllerYawInput(Value);
}

void APNPlayerCharacter::LookUp(float Value)
{
	if (FMath::IsNearlyZero(Value))
	{
		return;
	}

	AddControllerPitchInput(Value);
}

void APNPlayerCharacter::StartJump()
{
	if (IsDead())
	{
		return;
	}

	Jump();
}

void APNPlayerCharacter::StopJump()
{
	StopJumping();
}

void APNPlayerCharacter::StartCrouchInput()
{
	if (IsDead())
	{
		return;
	}

	StopSprint();
	Crouch();
}

void APNPlayerCharacter::StopCrouchInput()
{
	UnCrouch();
}

void APNPlayerCharacter::RefreshFirstPersonVisibility()
{
	if (GetMesh())
	{
		GetMesh()->SetOwnerNoSee(true);
		GetMesh()->SetOnlyOwnerSee(false);
	}

	if (GetThirdPersonHeadMeshComponent())
	{
		GetThirdPersonHeadMeshComponent()->SetOwnerNoSee(true);
		GetThirdPersonHeadMeshComponent()->SetOnlyOwnerSee(false);
	}

	if (GetThirdPersonLegsMeshComponent())
	{
		GetThirdPersonLegsMeshComponent()->SetOwnerNoSee(true);
		GetThirdPersonLegsMeshComponent()->SetOnlyOwnerSee(false);
	}

	if (GetThirdPersonHandsMeshComponent())
	{
		GetThirdPersonHandsMeshComponent()->SetOwnerNoSee(true);
		GetThirdPersonHandsMeshComponent()->SetOnlyOwnerSee(false);
	}

	if (FirstPersonArmsMeshComponent)
	{
		FirstPersonArmsMeshComponent->SetOnlyOwnerSee(true);
		FirstPersonArmsMeshComponent->SetOwnerNoSee(false);
	}
}

void APNPlayerCharacter::OnRep_FirstPersonArmsMesh()
{
	ApplyFirstPersonArmsMesh();
}

void APNPlayerCharacter::OnRep_FirstPersonAnimType()
{
}