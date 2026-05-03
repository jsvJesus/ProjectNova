#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PNItemInstance.generated.h"

class UPNItemDataAsset;

UCLASS(BlueprintType)
class PROJECTNOVA_API UPNItemInstance : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Instance")
	TObjectPtr<UPNItemDataAsset> ItemData = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Instance", meta = (ClampMin = "0"))
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Runtime", meta = (ClampMin = "0.0"))
	float CurrentDurability = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Runtime", meta = (ClampMin = "0.0"))
	float CurrentBatteryCharge = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Runtime", meta = (ClampMin = "0.0"))
	float RemainingShelfLifeSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Runtime", meta = (ClampMin = "0"))
	int32 AmmoInMagazine = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Runtime")
	bool bInitialized = false;

public:
	UFUNCTION(BlueprintCallable, Category = "ProjectNova|Item Instance")
	void Initialize(UPNItemDataAsset* InItemData, int32 InQuantity = 1);

	UFUNCTION(BlueprintPure, Category = "ProjectNova|Item Instance")
	bool IsValidItem() const;

	UFUNCTION(BlueprintPure, Category = "ProjectNova|Item Instance")
	bool IsEmpty() const;

	UFUNCTION(BlueprintPure, Category = "ProjectNova|Item Instance")
	UPNItemDataAsset* GetItemData() const;

	UFUNCTION(BlueprintPure, Category = "ProjectNova|Item Instance")
	FName GetItemId() const;

	UFUNCTION(BlueprintPure, Category = "ProjectNova|Item Instance")
	FText GetItemName() const;

	UFUNCTION(BlueprintPure, Category = "ProjectNova|Item Instance")
	int32 GetMaxStack() const;

	UFUNCTION(BlueprintPure, Category = "ProjectNova|Item Instance")
	float GetTotalWeight() const;

	UFUNCTION(BlueprintPure, Category = "ProjectNova|Item Instance")
	float GetDurabilityPercent() const;

	UFUNCTION(BlueprintPure, Category = "ProjectNova|Item Instance")
	float GetBatteryChargePercent() const;

	UFUNCTION(BlueprintPure, Category = "ProjectNova|Item Instance")
	float GetExpirationPercent() const;

	UFUNCTION(BlueprintPure, Category = "ProjectNova|Item Instance")
	bool IsBroken() const;

	UFUNCTION(BlueprintPure, Category = "ProjectNova|Item Instance")
	bool IsExpired() const;

	UFUNCTION(BlueprintPure, Category = "ProjectNova|Item Instance")
	bool CanStackWith(const UPNItemInstance* Other) const;

	UFUNCTION(BlueprintPure, Category = "ProjectNova|Item Instance")
	bool CanAddQuantity(int32 AddQuantity) const;

	UFUNCTION(BlueprintCallable, Category = "ProjectNova|Item Instance")
	bool AddQuantity(int32 AddQuantity);

	UFUNCTION(BlueprintCallable, Category = "ProjectNova|Item Instance")
	int32 RemoveQuantity(int32 RemoveQuantity);

	UFUNCTION(BlueprintCallable, Category = "ProjectNova|Item Instance")
	void SetQuantityClamped(int32 NewQuantity);

	UFUNCTION(BlueprintCallable, Category = "ProjectNova|Item Instance")
	void SetDurability(float NewDurability);

	UFUNCTION(BlueprintCallable, Category = "ProjectNova|Item Instance")
	void AddDurability(float DeltaDurability);

	UFUNCTION(BlueprintCallable, Category = "ProjectNova|Item Instance")
	void SetBatteryCharge(float NewBatteryCharge);

	UFUNCTION(BlueprintCallable, Category = "ProjectNova|Item Instance")
	void AddBatteryCharge(float DeltaBatteryCharge);

	UFUNCTION(BlueprintCallable, Category = "ProjectNova|Item Instance")
	void SetRemainingShelfLife(float NewRemainingShelfLifeSeconds);

	UFUNCTION(BlueprintCallable, Category = "ProjectNova|Item Instance")
	void SetAmmoInMagazine(int32 NewAmmoInMagazine);

	UFUNCTION(BlueprintCallable, Category = "ProjectNova|Item Instance")
	bool ConsumeAmmo(int32 AmmoCount);

	UFUNCTION(BlueprintCallable, Category = "ProjectNova|Item Instance")
	void RefillAmmo();

	UFUNCTION(BlueprintCallable, Category = "ProjectNova|Item Instance")
	bool TickExpiration(float DeltaSeconds);

protected:
	void ResetRuntimeFromData();
	void ClampRuntimeValues();
};