#include "Items/PNItemInstance.h"
#include "Items/PNItemDataAsset.h"

void UPNItemInstance::Initialize(UPNItemDataAsset* InItemData, int32 InQuantity)
{
	ItemData = InItemData;
	Quantity = FMath::Max(1, InQuantity);
	bInitialized = ItemData != nullptr;

	ResetRuntimeFromData();
	ClampRuntimeValues();
}

bool UPNItemInstance::IsValidItem() const
{
	return ItemData != nullptr && !IsEmpty();
}

bool UPNItemInstance::IsEmpty() const
{
	return Quantity <= 0;
}

UPNItemDataAsset* UPNItemInstance::GetItemData() const
{
	return ItemData;
}

FName UPNItemInstance::GetItemId() const
{
	return ItemData ? ItemData->GetItemId() : NAME_None;
}

FText UPNItemInstance::GetItemName() const
{
	return ItemData ? ItemData->GetItemName() : FText::GetEmpty();
}

int32 UPNItemInstance::GetMaxStack() const
{
	return ItemData ? ItemData->GetMaxStack() : 1;
}

float UPNItemInstance::GetTotalWeight() const
{
	return ItemData ? ItemData->GetTotalWeightForCount(Quantity) : 0.0f;
}

float UPNItemInstance::GetDurabilityPercent() const
{
	if (!ItemData || !ItemData->UsesDurability())
	{
		return 1.0f;
	}

	const float MaxDurability = FMath::Max(0.0f, ItemData->Durability.MaxDurability);

	if (MaxDurability <= 0.0f)
	{
		return 0.0f;
	}

	return FMath::Clamp(CurrentDurability / MaxDurability, 0.0f, 1.0f);
}

float UPNItemInstance::GetBatteryChargePercent() const
{
	if (!ItemData || !ItemData->UsesBattery())
	{
		return 1.0f;
	}

	const float MaxCharge = FMath::Max(0.0f, ItemData->Battery.MaxCharge);

	if (MaxCharge <= 0.0f)
	{
		return 0.0f;
	}

	return FMath::Clamp(CurrentBatteryCharge / MaxCharge, 0.0f, 1.0f);
}

float UPNItemInstance::GetExpirationPercent() const
{
	if (!ItemData || !ItemData->UsesExpiration())
	{
		return 1.0f;
	}

	const float ShelfLife = FMath::Max(0.0f, ItemData->Expiration.ShelfLifeSeconds);

	if (ShelfLife <= 0.0f)
	{
		return 0.0f;
	}

	return FMath::Clamp(RemainingShelfLifeSeconds / ShelfLife, 0.0f, 1.0f);
}

bool UPNItemInstance::IsBroken() const
{
	if (!ItemData || !ItemData->UsesDurability())
	{
		return false;
	}

	return ItemData->Durability.bBrokenWhenZero && CurrentDurability <= 0.0f;
}

bool UPNItemInstance::IsExpired() const
{
	if (!ItemData || !ItemData->UsesExpiration())
	{
		return false;
	}

	return RemainingShelfLifeSeconds <= 0.0f;
}

bool UPNItemInstance::CanStackWith(const UPNItemInstance* Other) const
{
	if (!IsValidItem() || !Other || !Other->IsValidItem())
	{
		return false;
	}

	if (!ItemData->CanStack())
	{
		return false;
	}

	if (GetItemId() != Other->GetItemId())
	{
		return false;
	}

	if (Quantity >= GetMaxStack())
	{
		return false;
	}

	const FPNItemStackData& StackRules = ItemData->Stack;

	if (StackRules.bStackRequiresSameDurability)
	{
		if (!FMath::IsNearlyEqual(CurrentDurability, Other->CurrentDurability, 0.01f))
		{
			return false;
		}
	}

	if (StackRules.bStackRequiresSameBatteryCharge)
	{
		if (!FMath::IsNearlyEqual(CurrentBatteryCharge, Other->CurrentBatteryCharge, 0.01f))
		{
			return false;
		}
	}

	if (StackRules.bStackRequiresSameExpirationTime)
	{
		if (!FMath::IsNearlyEqual(RemainingShelfLifeSeconds, Other->RemainingShelfLifeSeconds, 0.01f))
		{
			return false;
		}
	}

	if (StackRules.bStackRequiresSameAmmoCount)
	{
		if (AmmoInMagazine != Other->AmmoInMagazine)
		{
			return false;
		}
	}

	return true;
}

bool UPNItemInstance::CanAddQuantity(int32 AddQuantity) const
{
	if (!IsValidItem())
	{
		return false;
	}

	if (AddQuantity <= 0)
	{
		return false;
	}

	if (!ItemData->CanStack())
	{
		return false;
	}

	return Quantity + AddQuantity <= GetMaxStack();
}

bool UPNItemInstance::AddQuantity(int32 AddQuantity)
{
	if (!CanAddQuantity(AddQuantity))
	{
		return false;
	}

	Quantity += AddQuantity;
	ClampRuntimeValues();

	return true;
}

int32 UPNItemInstance::RemoveQuantity(int32 RemoveQuantityValue)
{
	if (RemoveQuantityValue <= 0 || IsEmpty())
	{
		return 0;
	}

	const int32 Removed = FMath::Min(Quantity, RemoveQuantityValue);
	Quantity -= Removed;

	return Removed;
}

void UPNItemInstance::SetQuantityClamped(int32 NewQuantity)
{
	if (!ItemData)
	{
		Quantity = FMath::Max(0, NewQuantity);
		return;
	}

	Quantity = FMath::Clamp(NewQuantity, 0, GetMaxStack());
}

void UPNItemInstance::SetDurability(float NewDurability)
{
	if (!ItemData || !ItemData->UsesDurability())
	{
		CurrentDurability = 0.0f;
		return;
	}

	CurrentDurability = FMath::Clamp(NewDurability, 0.0f, ItemData->Durability.MaxDurability);
}

void UPNItemInstance::AddDurability(float DeltaDurability)
{
	SetDurability(CurrentDurability + DeltaDurability);
}

void UPNItemInstance::SetBatteryCharge(float NewBatteryCharge)
{
	if (!ItemData || !ItemData->UsesBattery())
	{
		CurrentBatteryCharge = 0.0f;
		return;
	}

	CurrentBatteryCharge = FMath::Clamp(NewBatteryCharge, 0.0f, ItemData->Battery.MaxCharge);
}

void UPNItemInstance::AddBatteryCharge(float DeltaBatteryCharge)
{
	SetBatteryCharge(CurrentBatteryCharge + DeltaBatteryCharge);
}

void UPNItemInstance::SetRemainingShelfLife(float NewRemainingShelfLifeSeconds)
{
	if (!ItemData || !ItemData->UsesExpiration())
	{
		RemainingShelfLifeSeconds = 0.0f;
		return;
	}

	RemainingShelfLifeSeconds = FMath::Clamp(NewRemainingShelfLifeSeconds, 0.0f, ItemData->Expiration.ShelfLifeSeconds);
}

void UPNItemInstance::SetAmmoInMagazine(int32 NewAmmoInMagazine)
{
	if (!ItemData)
	{
		AmmoInMagazine = 0;
		return;
	}

	const int32 MaxAmmo = FMath::Max(0, ItemData->MagazineStats.ClipSizeMax);

	if (MaxAmmo <= 0)
	{
		AmmoInMagazine = FMath::Max(0, NewAmmoInMagazine);
		return;
	}

	AmmoInMagazine = FMath::Clamp(NewAmmoInMagazine, 0, MaxAmmo);
}

bool UPNItemInstance::ConsumeAmmo(int32 AmmoCount)
{
	if (AmmoCount <= 0)
	{
		return false;
	}

	if (AmmoInMagazine < AmmoCount)
	{
		return false;
	}

	AmmoInMagazine -= AmmoCount;
	return true;
}

void UPNItemInstance::RefillAmmo()
{
	if (!ItemData)
	{
		AmmoInMagazine = 0;
		return;
	}

	if (ItemData->MagazineStats.ClipSizeMax > 0)
	{
		AmmoInMagazine = ItemData->MagazineStats.ClipSizeMax;
		return;
	}

	AmmoInMagazine = ItemData->MagazineStats.ClipSize;
}

bool UPNItemInstance::TickExpiration(float DeltaSeconds)
{
	if (!ItemData || !ItemData->UsesExpiration())
	{
		return false;
	}

	if (DeltaSeconds <= 0.0f)
	{
		return IsExpired();
	}

	RemainingShelfLifeSeconds = FMath::Max(0.0f, RemainingShelfLifeSeconds - DeltaSeconds);

	return IsExpired();
}

void UPNItemInstance::ResetRuntimeFromData()
{
	if (!ItemData)
	{
		CurrentDurability = 0.0f;
		CurrentBatteryCharge = 0.0f;
		RemainingShelfLifeSeconds = 0.0f;
		AmmoInMagazine = 0;
		return;
	}

	if (ItemData->UsesDurability())
	{
		CurrentDurability = ItemData->Durability.DefaultDurability;
	}
	else
	{
		CurrentDurability = 0.0f;
	}

	if (ItemData->UsesBattery())
	{
		CurrentBatteryCharge = ItemData->Battery.DefaultCharge;
	}
	else
	{
		CurrentBatteryCharge = 0.0f;
	}

	if (ItemData->UsesExpiration())
	{
		RemainingShelfLifeSeconds = ItemData->Expiration.ShelfLifeSeconds;
	}
	else
	{
		RemainingShelfLifeSeconds = 0.0f;
	}

	if (ItemData->ItemCategory == EPNItemCategory::Mag)
	{
		AmmoInMagazine = ItemData->MagazineStats.ClipSize;
	}
	else
	{
		AmmoInMagazine = 0;
	}
}

void UPNItemInstance::ClampRuntimeValues()
{
	if (!ItemData)
	{
		Quantity = FMath::Max(0, Quantity);
		CurrentDurability = FMath::Max(0.0f, CurrentDurability);
		CurrentBatteryCharge = FMath::Max(0.0f, CurrentBatteryCharge);
		RemainingShelfLifeSeconds = FMath::Max(0.0f, RemainingShelfLifeSeconds);
		AmmoInMagazine = FMath::Max(0, AmmoInMagazine);
		return;
	}

	Quantity = FMath::Clamp(Quantity, 0, GetMaxStack());

	if (ItemData->UsesDurability())
	{
		CurrentDurability = FMath::Clamp(CurrentDurability, 0.0f, ItemData->Durability.MaxDurability);
	}
	else
	{
		CurrentDurability = 0.0f;
	}

	if (ItemData->UsesBattery())
	{
		CurrentBatteryCharge = FMath::Clamp(CurrentBatteryCharge, 0.0f, ItemData->Battery.MaxCharge);
	}
	else
	{
		CurrentBatteryCharge = 0.0f;
	}

	if (ItemData->UsesExpiration())
	{
		RemainingShelfLifeSeconds = FMath::Clamp(RemainingShelfLifeSeconds, 0.0f, ItemData->Expiration.ShelfLifeSeconds);
	}
	else
	{
		RemainingShelfLifeSeconds = 0.0f;
	}

	if (ItemData->MagazineStats.ClipSizeMax > 0)
	{
		AmmoInMagazine = FMath::Clamp(AmmoInMagazine, 0, ItemData->MagazineStats.ClipSizeMax);
	}
	else
	{
		AmmoInMagazine = FMath::Max(0, AmmoInMagazine);
	}
}