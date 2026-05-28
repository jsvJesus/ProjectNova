#include "Animation/PNFirstPersonArmsAnimInstance.h"

#include "Characters/PNPlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UPNFirstPersonArmsAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	RefreshCachedCharacter();
}

void UPNFirstPersonArmsAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!CachedPlayerCharacter)
	{
		RefreshCachedCharacter();
	}

	if (!CachedPlayerCharacter)
	{
		ResetAnimationState();
		return;
	}

	if (!CachedMovementComponent)
	{
		CachedMovementComponent = CachedPlayerCharacter->GetCharacterMovement();
	}

	const FVector CharacterVelocity = CachedPlayerCharacter->GetVelocity();
	const FVector GroundVelocity(CharacterVelocity.X, CharacterVelocity.Y, 0.0f);

	Speed = GroundVelocity.Size();
	Direction = CalculateMovementDirection(GroundVelocity, CachedPlayerCharacter->GetActorRotation());

	bIsMoving = Speed > 3.0f;

	if (CachedMovementComponent)
	{
		const FVector Acceleration = CachedMovementComponent->GetCurrentAcceleration();

		bHasAcceleration = Acceleration.SizeSquared2D() > 1.0f;
		bIsInAir = CachedMovementComponent->IsFalling();
		bIsCrouching = CachedPlayerCharacter->bIsCrouched;
	}
	else
	{
		bHasAcceleration = false;
		bIsInAir = false;
		bIsCrouching = false;
	}

	bIsSprinting = CachedPlayerCharacter->IsSprinting();
	bIsDead = CachedPlayerCharacter->IsDead();
	CurrentAnimType = CachedPlayerCharacter->GetFirstPersonAnimType();
}

float UPNFirstPersonArmsAnimInstance::GetSpeed() const
{
	return Speed;
}

float UPNFirstPersonArmsAnimInstance::GetDirection() const
{
	return Direction;
}

bool UPNFirstPersonArmsAnimInstance::IsMoving() const
{
	return bIsMoving;
}

bool UPNFirstPersonArmsAnimInstance::HasAcceleration() const
{
	return bHasAcceleration;
}

bool UPNFirstPersonArmsAnimInstance::IsSprinting() const
{
	return bIsSprinting;
}

bool UPNFirstPersonArmsAnimInstance::IsInAir() const
{
	return bIsInAir;
}

bool UPNFirstPersonArmsAnimInstance::IsCrouching() const
{
	return bIsCrouching;
}

bool UPNFirstPersonArmsAnimInstance::IsDead() const
{
	return bIsDead;
}

EPNAnimType UPNFirstPersonArmsAnimInstance::GetCurrentAnimType() const
{
	return CurrentAnimType;
}

bool UPNFirstPersonArmsAnimInstance::IsKnifeAnim() const
{
	return CurrentAnimType == EPNAnimType::Knife;
}

void UPNFirstPersonArmsAnimInstance::RefreshCachedCharacter()
{
	CachedPlayerCharacter = Cast<APNPlayerCharacter>(TryGetPawnOwner());

	if (CachedPlayerCharacter)
	{
		CachedMovementComponent = CachedPlayerCharacter->GetCharacterMovement();
	}
	else
	{
		CachedMovementComponent = nullptr;
	}
}

void UPNFirstPersonArmsAnimInstance::ResetAnimationState()
{
	Speed = 0.0f;
	Direction = 0.0f;

	bIsMoving = false;
	bHasAcceleration = false;
	bIsSprinting = false;
	bIsInAir = false;
	bIsCrouching = false;
	bIsDead = false;

	CurrentAnimType = EPNAnimType::Unarmed;
}

float UPNFirstPersonArmsAnimInstance::CalculateMovementDirection(const FVector& Velocity, const FRotator& BaseRotation) const
{
	if (Velocity.IsNearlyZero())
	{
		return 0.0f;
	}

	const FVector LocalVelocity = BaseRotation.UnrotateVector(Velocity);
	const float DirectionRadians = FMath::Atan2(LocalVelocity.Y, LocalVelocity.X);

	return FMath::RadiansToDegrees(DirectionRadians);
}