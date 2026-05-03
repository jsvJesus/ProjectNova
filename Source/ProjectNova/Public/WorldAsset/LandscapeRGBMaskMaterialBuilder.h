#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LandscapeRGBMaskMaterialBuilder.generated.h"

class UMaterial;

UCLASS()
class UWarZLandscapeRGBMaskMaterialBuilder : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "WarZ|Landscape Material")
    static UMaterial* CreateRGBMaskLandscapeMaterial(
        const FString& PackagePath = TEXT("/Game/WarZ/Materials"),
        const FString& MaterialName = TEXT("M_ROTB_LMS_RGBMasks")
    );
    
    UFUNCTION(BlueprintCallable, Category = "WarZ|Landscape Material")
        static UMaterial* CreateShatteredSkiesRGBMaskLandscapeMaterial(
            const FString& PackagePath = TEXT("/Game/WarZ/Materials"),
            const FString& MaterialName = TEXT("M_ShatteredSkies_RGBMasks")
        );
};