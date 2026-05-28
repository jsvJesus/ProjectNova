#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Items/PNItemTypes.h"
#include "PNFirstPersonArmsAnimInstance.generated.h"

class APNPlayerCharacter;
class UCharacterMovementComponent;

UCLASS(Blueprintable, BlueprintType)
class PROJECTNOVA_API UPNFirstPersonArmsAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Character")
	TObjectPtr<APNPlayerCharacter> CachedPlayerCharacter = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Character")
	TObjectPtr<UCharacterMovementComponent> CachedMovementComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float Speed = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float Direction = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bIsMoving = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bHasAcceleration = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bIsSprinting = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bIsInAir = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bIsCrouching = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsDead = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
	EPNAnimType CurrentAnimType = EPNAnimType::Unarmed;

public:
	UFUNCTION(BlueprintPure, Category = "Anim")
	float GetSpeed() const;

	UFUNCTION(BlueprintPure, Category = "Anim")
	float GetDirection() const;

	UFUNCTION(BlueprintPure, Category = "Anim")
	bool IsMoving() const;

	UFUNCTION(BlueprintPure, Category = "Anim")
	bool HasAcceleration() const;

	UFUNCTION(BlueprintPure, Category = "Anim")
	bool IsSprinting() const;

	UFUNCTION(BlueprintPure, Category = "Anim")
	bool IsInAir() const;

	UFUNCTION(BlueprintPure, Category = "Anim")
	bool IsCrouching() const;

	UFUNCTION(BlueprintPure, Category = "Anim")
	bool IsDead() const;

	UFUNCTION(BlueprintPure, Category = "Anim")
	EPNAnimType GetCurrentAnimType() const;

	UFUNCTION(BlueprintPure, Category = "Anim", meta = (BlueprintThreadSafe))
	bool IsKnifeAnim() const;

protected:
	void RefreshCachedCharacter();
	void ResetAnimationState();

	float CalculateMovementDirection(const FVector& Velocity, const FRotator& BaseRotation) const;
};