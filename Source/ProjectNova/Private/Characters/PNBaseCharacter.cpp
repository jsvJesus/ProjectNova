#include "Characters/PNBaseCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Items/PNInventoryComponent.h"
#include "Net/UnrealNetwork.h"

APNBaseCharacter::APNBaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicateMovement(true);

	InventoryComponent = CreateDefaultSubobject<UPNInventoryComponent>(TEXT("InventoryComponent"));
	if (InventoryComponent)
	{
		InventoryComponent->SetIsReplicated(true);
	}

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
}

void APNBaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	ApplyMovementSpeed();
}

void APNBaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void APNBaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APNBaseCharacter, bIsSprinting);
	DOREPLIFETIME(APNBaseCharacter, bIsDead);
}

float APNBaseCharacter::TakeDamage(
	float DamageAmount,
	FDamageEvent const& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser
)
{
	const float AppliedDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (!HasAuthority())
	{
		return AppliedDamage;
	}

	if (bIsDead)
	{
		return AppliedDamage;
	}

	if (AppliedDamage <= 0.0f)
	{
		return AppliedDamage;
	}

	// Временно. Нормальное здоровье будет в отдельном HealthComponent.
	if (AppliedDamage >= 99999.0f)
	{
		Die(EventInstigator);
	}

	return AppliedDamage;
}

UPNInventoryComponent* APNBaseCharacter::GetInventoryComponent() const
{
	return InventoryComponent;
}

bool APNBaseCharacter::IsSprinting() const
{
	return bIsSprinting;
}

bool APNBaseCharacter::IsDead() const
{
	return bIsDead;
}

void APNBaseCharacter::StartSprint()
{
	if (bIsDead)
	{
		return;
	}

	if (HasAuthority())
	{
		SetSprintingInternal(true);
		return;
	}

	Server_SetSprinting(true);
}

void APNBaseCharacter::StopSprint()
{
	if (HasAuthority())
	{
		SetSprintingInternal(false);
		return;
	}

	Server_SetSprinting(false);
}

void APNBaseCharacter::Die(AController* KillerController)
{
	if (!HasAuthority())
	{
		return;
	}

	if (bIsDead)
	{
		return;
	}

	bIsDead = true;
	bIsSprinting = false;

	ApplyMovementSpeed();

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->DisableMovement();
	}

	OnRep_IsDead();
}

void APNBaseCharacter::Revive()
{
	if (!HasAuthority())
	{
		return;
	}

	bIsDead = false;
	bIsSprinting = false;

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}

	ApplyMovementSpeed();

	OnRep_IsDead();
}

void APNBaseCharacter::Server_SetSprinting_Implementation(bool bNewSprinting)
{
	SetSprintingInternal(bNewSprinting);
}

void APNBaseCharacter::SetSprintingInternal(bool bNewSprinting)
{
	if (bIsDead)
	{
		bNewSprinting = false;
	}

	if (bIsSprinting == bNewSprinting)
	{
		return;
	}

	bIsSprinting = bNewSprinting;
	ApplyMovementSpeed();

	OnRep_IsSprinting();
}

void APNBaseCharacter::ApplyMovementSpeed()
{
	if (!GetCharacterMovement())
	{
		return;
	}

	if (bIsDead)
	{
		GetCharacterMovement()->MaxWalkSpeed = 0.0f;
		return;
	}

	if (bIsSprinting)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
		return;
	}

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
}

void APNBaseCharacter::OnRep_IsSprinting()
{
	ApplyMovementSpeed();
}

void APNBaseCharacter::OnRep_IsDead()
{
	ApplyMovementSpeed();
}