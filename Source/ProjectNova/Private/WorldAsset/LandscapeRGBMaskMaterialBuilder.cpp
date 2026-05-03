#include "WorldAsset/LandscapeRGBMaskMaterialBuilder.h"

#include "MaterialDomain.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/Engine.h"
#include "Engine/Texture.h"
#include "Factories/MaterialFactoryNew.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionCustom.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionTextureObjectParameter.h"
#include "Materials/MaterialExpressionWorldPosition.h"
#include "Materials/MaterialExpressionConstant.h"
#include "UObject/Package.h"

namespace WarZLandscapeMaterial
{
    template<typename T>
    T* AddNode(UMaterial* Material, int32 X, int32 Y)
    {
        T* Node = NewObject<T>(Material);
        Node->MaterialExpressionEditorX = X;
        Node->MaterialExpressionEditorY = Y;

        Material->GetEditorOnlyData()->ExpressionCollection.Expressions.Add(Node);
        return Node;
    }

    UTexture* LoadTextureSafe(const TCHAR* Path)
    {
        return LoadObject<UTexture>(nullptr, Path);
    }

    UTexture* GetDefaultColorTexture()
    {
        if (UTexture* Tex = LoadTextureSafe(TEXT("/Engine/EngineResources/DefaultTexture.DefaultTexture")))
        {
            return Tex;
        }

        return GEngine ? GEngine->DefaultTexture : nullptr;
    }

    UTexture* GetDefaultNormalTexture()
    {
        if (UTexture* Tex = LoadTextureSafe(TEXT("/Engine/EngineMaterials/DefaultNormal.DefaultNormal")))
        {
            return Tex;
        }

        // В UE5.5 у GEngine нет DefaultNormalTexture.
        // Для компиляции fallback делаем на обычную DefaultTexture.
        return GetDefaultColorTexture();
    }

    UTexture* GetDefaultMaskTexture()
    {
        if (UTexture* Tex = LoadTextureSafe(TEXT("/Engine/EngineResources/Black.Black")))
        {
            return Tex;
        }

        return GEngine ? GEngine->DefaultTexture : nullptr;
    }

    UMaterialExpressionScalarParameter* AddScalar(
        UMaterial* Material,
        const FString& Name,
        float DefaultValue,
        const FName& Group,
        int32 X,
        int32 Y)
    {
        UMaterialExpressionScalarParameter* Node =
            AddNode<UMaterialExpressionScalarParameter>(Material, X, Y);

        Node->ParameterName = FName(*Name);
        Node->DefaultValue = DefaultValue;
        Node->Group = Group;

        return Node;
    }

    UMaterialExpressionTextureObjectParameter* AddTextureObject(
        UMaterial* Material,
        const FString& Name,
        UTexture* DefaultTexture,
        EMaterialSamplerType SamplerType,
        const FName& Group,
        int32 X,
        int32 Y)
    {
        UMaterialExpressionTextureObjectParameter* Node =
            AddNode<UMaterialExpressionTextureObjectParameter>(Material, X, Y);

        Node->ParameterName = FName(*Name);
        Node->Texture = DefaultTexture;
        Node->SamplerType = SamplerType;
        Node->Group = Group;

        return Node;
    }

    void AddCustomInput(
        UMaterialExpressionCustom* Custom,
        const FString& Name,
        UMaterialExpression* Expression,
        int32 OutputIndex = 0)
    {
        FCustomInput Input;
        Input.InputName = FName(*Name);
        Input.Input.Connect(OutputIndex, Expression);
        Custom->Inputs.Add(Input);
    }

    struct FLayerSlot
    {
        FString Prefix;
        FString MaskVar;
        TCHAR Channel;

        float Tile;
        float MultiTile;
        float SpecPow;
        float Weight;

        UMaterialExpressionTextureObjectParameter* BaseColor = nullptr;
        UMaterialExpressionTextureObjectParameter* Normal = nullptr;

        UMaterialExpressionScalarParameter* TileParam = nullptr;
        UMaterialExpressionScalarParameter* MultiTileParam = nullptr;
        UMaterialExpressionScalarParameter* SpecPowParam = nullptr;
        UMaterialExpressionScalarParameter* WeightParam = nullptr;
    };

    FString BuildMaskCode()
    {
        return TEXT(
            "float2 MaskUV = float2(\n"
            "    (WorldPos.x - MaskOriginX) / max(MaskWorldSizeX, 1.0),\n"
            "    (WorldPos.y - MaskOriginY) / max(MaskWorldSizeY, 1.0)\n"
            ");\n"
            "MaskUV.y = lerp(MaskUV.y, 1.0 - MaskUV.y, saturate(MaskFlipY));\n"
            "float3 Mask1 = Texture2DSample(Mask1_RGB, Mask1_RGBSampler, MaskUV).rgb;\n"
            "float3 Mask2 = Texture2DSample(Mask2_RGB, Mask2_RGBSampler, MaskUV).rgb;\n"
            "float3 Mask3 = Texture2DSample(Mask3_RGB, Mask3_RGBSampler, MaskUV).rgb;\n"
            "\n"
            "#define WZ_UV(TileValue, MultiValue) ((WorldPos.xy / max((TileValue) * WorldUnitScale, 1.0)) * max((MultiValue), 0.001))\n"
        );
    }

    FString BuildBaseColorCode(const TArray<FLayerSlot>& Layers)
    {
        FString Code;
        Code += BuildMaskCode();

        Code += TEXT(
            "float3 OutColor = Texture2DSample(\n"
            "    Base_BaseColor,\n"
            "    Base_BaseColorSampler,\n"
            "    WZ_UV(Base_Tile, Base_MultiTile)\n"
            ").rgb;\n"
            "\n"
        );

        for (const FLayerSlot& L : Layers)
        {
            Code += FString::Printf(
                TEXT(
                    "{\n"
                    "    float A = saturate(%s.%c * %s_Weight);\n"
                    "    float3 C = Texture2DSample(\n"
                    "        %s_BaseColor,\n"
                    "        %s_BaseColorSampler,\n"
                    "        WZ_UV(%s_Tile, %s_MultiTile)\n"
                    "    ).rgb;\n"
                    "    OutColor = lerp(OutColor, C, A);\n"
                    "}\n"
                    "\n"
                ),
                *L.MaskVar,
                L.Channel,
                *L.Prefix,
                *L.Prefix,
                *L.Prefix,
                *L.Prefix,
                *L.Prefix
            );
        }

        Code += TEXT(
            "#undef WZ_UV\n"
            "return OutColor;\n"
        );

        return Code;
    }

    FString BuildNormalCode(const TArray<FLayerSlot>& Layers)
    {
        FString Code;
        Code += BuildMaskCode();

        Code += TEXT(
            "float3 PackedBaseN = Texture2DSample(\n"
            "    Base_Normal,\n"
            "    Base_NormalSampler,\n"
            "    WZ_UV(Base_Tile, Base_MultiTile)\n"
            ").xyz;\n"
            "\n"
            "float2 BaseXY = PackedBaseN.xy * 2.0 - 1.0;\n"
            "float3 OutNormal = normalize(float3(BaseXY, sqrt(saturate(1.0 - dot(BaseXY, BaseXY)))));\n"
            "\n"
        );

        for (const FLayerSlot& L : Layers)
        {
            Code += FString::Printf(
                TEXT(
                    "{\n"
                    "    float A = saturate(%s.%c * %s_Weight);\n"
                    "    float3 PackedN = Texture2DSample(\n"
                    "        %s_Normal,\n"
                    "        %s_NormalSampler,\n"
                    "        WZ_UV(%s_Tile, %s_MultiTile)\n"
                    "    ).xyz;\n"
                    "    float2 XY = PackedN.xy * 2.0 - 1.0;\n"
                    "    float3 N = normalize(float3(XY, sqrt(saturate(1.0 - dot(XY, XY)))));\n"
                    "    OutNormal = normalize(lerp(OutNormal, N, A));\n"
                    "}\n"
                    "\n"
                ),
                *L.MaskVar,
                L.Channel,
                *L.Prefix,
                *L.Prefix,
                *L.Prefix,
                *L.Prefix,
                *L.Prefix
            );
        }

        Code += TEXT(
            "#undef WZ_UV\n"
            "return OutNormal;\n"
        );

        return Code;
    }

    FString BuildRoughnessCode(const TArray<FLayerSlot>& Layers)
    {
        FString Code;
        Code += BuildMaskCode();

        Code += TEXT(
            "float FinalSpecPow = Base_SpecPow;\n"
            "\n"
        );

        for (const FLayerSlot& L : Layers)
        {
            Code += FString::Printf(
                TEXT(
                    "{\n"
                    "    float A = saturate(%s.%c * %s_Weight);\n"
                    "    FinalSpecPow = lerp(FinalSpecPow, %s_SpecPow, A);\n"
                    "}\n"
                    "\n"
                ),
                *L.MaskVar,
                L.Channel,
                *L.Prefix,
                *L.Prefix
            );
        }

        Code += TEXT(
            "#undef WZ_UV\n"
            "// Legacy SpecPow -> UE Roughness. SpecPow 1 = rough, bigger SpecPow = sharper.\n"
            "return saturate(1.0 / max(FinalSpecPow, 1.0));\n"
        );

        return Code;
    }

    UMaterialExpressionCustom* AddCustomNode(
        UMaterial* Material,
        const FString& Description,
        const FString& Code,
        ECustomMaterialOutputType OutputType,
        int32 X,
        int32 Y)
    {
        UMaterialExpressionCustom* Node =
            AddNode<UMaterialExpressionCustom>(Material, X, Y);

        Node->Description = Description;
        Node->Code = Code;
        Node->OutputType = OutputType;

        return Node;
    }

    void AddSharedInputs(
        UMaterialExpressionCustom* Custom,
        UMaterialExpressionWorldPosition* WorldPos,

        UMaterialExpressionScalarParameter* WorldUnitScale,
        UMaterialExpressionScalarParameter* MaskOriginX,
        UMaterialExpressionScalarParameter* MaskOriginY,
        UMaterialExpressionScalarParameter* MaskWorldSizeX,
        UMaterialExpressionScalarParameter* MaskWorldSizeY,
        UMaterialExpressionScalarParameter* MaskFlipY,

        UMaterialExpressionTextureObjectParameter* Mask1,
        UMaterialExpressionTextureObjectParameter* Mask2,
        UMaterialExpressionTextureObjectParameter* Mask3,

        UMaterialExpressionTextureObjectParameter* BaseColor,
        UMaterialExpressionTextureObjectParameter* BaseNormal,
        UMaterialExpressionScalarParameter* BaseTile,
        UMaterialExpressionScalarParameter* BaseMultiTile,
        UMaterialExpressionScalarParameter* BaseSpecPow,

        const TArray<FLayerSlot>& Layers)
    {
        AddCustomInput(Custom, TEXT("WorldPos"), WorldPos);

        AddCustomInput(Custom, TEXT("WorldUnitScale"), WorldUnitScale);
        AddCustomInput(Custom, TEXT("MaskOriginX"), MaskOriginX);
        AddCustomInput(Custom, TEXT("MaskOriginY"), MaskOriginY);
        AddCustomInput(Custom, TEXT("MaskWorldSizeX"), MaskWorldSizeX);
        AddCustomInput(Custom, TEXT("MaskWorldSizeY"), MaskWorldSizeY);
        AddCustomInput(Custom, TEXT("MaskFlipY"), MaskFlipY);

        AddCustomInput(Custom, TEXT("Mask1_RGB"), Mask1);
        AddCustomInput(Custom, TEXT("Mask2_RGB"), Mask2);
        AddCustomInput(Custom, TEXT("Mask3_RGB"), Mask3);

        AddCustomInput(Custom, TEXT("Base_BaseColor"), BaseColor);
        AddCustomInput(Custom, TEXT("Base_Normal"), BaseNormal);
        AddCustomInput(Custom, TEXT("Base_Tile"), BaseTile);
        AddCustomInput(Custom, TEXT("Base_MultiTile"), BaseMultiTile);
        AddCustomInput(Custom, TEXT("Base_SpecPow"), BaseSpecPow);

        for (const FLayerSlot& L : Layers)
        {
            AddCustomInput(Custom, L.Prefix + TEXT("_BaseColor"), L.BaseColor);
            AddCustomInput(Custom, L.Prefix + TEXT("_Normal"), L.Normal);
            AddCustomInput(Custom, L.Prefix + TEXT("_Tile"), L.TileParam);
            AddCustomInput(Custom, L.Prefix + TEXT("_MultiTile"), L.MultiTileParam);
            AddCustomInput(Custom, L.Prefix + TEXT("_SpecPow"), L.SpecPowParam);
            AddCustomInput(Custom, L.Prefix + TEXT("_Weight"), L.WeightParam);
        }
    }
}

// For ROTB Map
UMaterial* UWarZLandscapeRGBMaskMaterialBuilder::CreateRGBMaskLandscapeMaterial(
    const FString& PackagePath,
    const FString& MaterialName)
{
#if !WITH_EDITOR
    return nullptr;
#else
    using namespace WarZLandscapeMaterial;

    FString CleanPackagePath = PackagePath;
    CleanPackagePath.RemoveFromEnd(TEXT("/"));

    const FString FullPackageName = CleanPackagePath / MaterialName;

    UPackage* Package = CreatePackage(*FullPackageName);
    if (!Package)
    {
        UE_LOG(LogTemp, Error, TEXT("WarZ RGB Material: failed to create package: %s"), *FullPackageName);
        return nullptr;
    }

    Package->FullyLoad();

    UMaterial* Material = FindObject<UMaterial>(Package, *MaterialName);

    if (!Material)
    {
        UMaterialFactoryNew* Factory = NewObject<UMaterialFactoryNew>();

        Material = Cast<UMaterial>(Factory->FactoryCreateNew(
            UMaterial::StaticClass(),
            Package,
            FName(*MaterialName),
            RF_Public | RF_Standalone,
            nullptr,
            GWarn
        ));

        if (!Material)
        {
            UE_LOG(LogTemp, Error, TEXT("WarZ RGB Material: failed to create material: %s"), *FullPackageName);
            return nullptr;
        }

        FAssetRegistryModule::AssetCreated(Material);
    }
    else
    {
        Material->Modify();

        UMaterialEditorOnlyData* EditorData = Material->GetEditorOnlyData();
        EditorData->ExpressionCollection.Expressions.Empty();
        EditorData->ExpressionCollection.EditorComments.Empty();
    }

    Material->MaterialDomain = EMaterialDomain::MD_Surface;
    Material->BlendMode = EBlendMode::BLEND_Opaque;
    Material->TwoSided = false;
    Material->SetShadingModel(EMaterialShadingModel::MSM_DefaultLit);

    UTexture* DefaultColor = GetDefaultColorTexture();
    UTexture* DefaultNormal = GetDefaultNormalTexture();
    UTexture* DefaultMask = GetDefaultMaskTexture();

    const FName GroupGlobal(TEXT("00_Global"));
    const FName GroupMasks(TEXT("01_RGB_Masks"));
    const FName GroupBase(TEXT("02_Base_Layer"));
    const FName GroupLayers(TEXT("03_Layers"));

    UMaterialExpressionWorldPosition* WorldPos =
        AddNode<UMaterialExpressionWorldPosition>(Material, -2400, -800);

    UMaterialExpressionScalarParameter* WorldUnitScale =
        AddScalar(Material, TEXT("WorldUnitScale"), 100.0f, GroupGlobal, -2400, -620);

    UMaterialExpressionScalarParameter* MaskOriginX =
        AddScalar(Material, TEXT("MaskOriginX"), 0.0f, GroupGlobal, -2400, -460);

    UMaterialExpressionScalarParameter* MaskOriginY =
        AddScalar(Material, TEXT("MaskOriginY"), 0.0f, GroupGlobal, -2400, -300);

    UMaterialExpressionScalarParameter* MaskWorldSizeX =
        AddScalar(Material, TEXT("MaskWorldSizeX"), 409600.0f, GroupGlobal, -2400, -140);

    UMaterialExpressionScalarParameter* MaskWorldSizeY =
        AddScalar(Material, TEXT("MaskWorldSizeY"), 409600.0f, GroupGlobal, -2400, 20);

    UMaterialExpressionScalarParameter* MaskFlipY =
        AddScalar(Material, TEXT("MaskFlipY"), 0.0f, GroupGlobal, -2400, 180);

    UMaterialExpressionTextureObjectParameter* Mask1 =
        AddTextureObject(
            Material,
            TEXT("Mask1_RGB_layers_1_3"),
            DefaultMask,
            SAMPLERTYPE_Masks,
            GroupMasks,
            -2100,
            -800
        );

    UMaterialExpressionTextureObjectParameter* Mask2 =
        AddTextureObject(
            Material,
            TEXT("Mask2_RGB_layers_4_6"),
            DefaultMask,
            SAMPLERTYPE_Masks,
            GroupMasks,
            -2100,
            -620
        );

    UMaterialExpressionTextureObjectParameter* Mask3 =
        AddTextureObject(
            Material,
            TEXT("Mask3_RGB_layers_7_9"),
            DefaultMask,
            SAMPLERTYPE_Masks,
            GroupMasks,
            -2100,
            -440
        );

    UMaterialExpressionTextureObjectParameter* BaseColor =
        AddTextureObject(
            Material,
            TEXT("Base_BaseColor"),
            DefaultColor,
            SAMPLERTYPE_Color,
            GroupBase,
            -1800,
            -800
        );

    UMaterialExpressionTextureObjectParameter* BaseNormal =
        AddTextureObject(
            Material,
            TEXT("Base_Normal"),
            DefaultNormal,
            SAMPLERTYPE_Normal,
            GroupBase,
            -1800,
            -620
        );

    UMaterialExpressionScalarParameter* BaseTile =
        AddScalar(Material, TEXT("Base_Tile"), 1000.0f, GroupBase, -1800, -440);

    UMaterialExpressionScalarParameter* BaseMultiTile =
        AddScalar(Material, TEXT("Base_MultiTile"), 1.0f, GroupBase, -1800, -280);

    UMaterialExpressionScalarParameter* BaseSpecPow =
        AddScalar(Material, TEXT("Base_SpecPow"), 1.0f, GroupBase, -1800, -120);

    TArray<FLayerSlot> Layers;

    auto AddLayer = [&](const TCHAR* Prefix, const TCHAR* MaskVar, TCHAR Channel, float Tile, float MultiTile, float SpecPow, float Weight)
        {
            FLayerSlot L;
            L.Prefix = Prefix;
            L.MaskVar = MaskVar;
            L.Channel = Channel;
            L.Tile = Tile;
            L.MultiTile = MultiTile;
            L.SpecPow = SpecPow;
            L.Weight = Weight;

            const int32 Index = Layers.Num();
            const int32 X = -1500 + (Index % 4) * 360;
            const int32 Y = -800 + (Index / 4) * 700;

            L.BaseColor =
                AddTextureObject(
                    Material,
                    L.Prefix + TEXT("_BaseColor"),
                    DefaultColor,
                    SAMPLERTYPE_Color,
                    GroupLayers,
                    X,
                    Y
                );

            L.Normal =
                AddTextureObject(
                    Material,
                    L.Prefix + TEXT("_Normal"),
                    DefaultNormal,
                    SAMPLERTYPE_Normal,
                    GroupLayers,
                    X,
                    Y + 160
                );

            L.TileParam =
                AddScalar(
                    Material,
                    L.Prefix + TEXT("_Tile"),
                    Tile,
                    GroupLayers,
                    X,
                    Y + 320
                );

            L.MultiTileParam =
                AddScalar(
                    Material,
                    L.Prefix + TEXT("_MultiTile"),
                    MultiTile,
                    GroupLayers,
                    X,
                    Y + 480
                );

            L.SpecPowParam =
                AddScalar(
                    Material,
                    L.Prefix + TEXT("_SpecPow"),
                    SpecPow,
                    GroupLayers,
                    X,
                    Y + 640
                );

            L.WeightParam =
                AddScalar(
                    Material,
                    L.Prefix + TEXT("_Weight"),
                    Weight,
                    GroupLayers,
                    X,
                    Y + 800
                );

            Layers.Add(L);
        };

    AddLayer(TEXT("L01_GrassDry"), TEXT("Mask1"), TEXT('r'), 1000.0f, 1.0f, 1.0f, 1.0f);
    AddLayer(TEXT("L02_GrassGravel"), TEXT("Mask1"), TEXT('g'), 1000.0f, 1.0f, 1.0f, 1.0f);
    AddLayer(TEXT("L03_RocksGravel"), TEXT("Mask1"), TEXT('b'), 500.0f, 1.0f, 1.0f, 1.0f);

    AddLayer(TEXT("L04_DesertDunesSand"), TEXT("Mask2"), TEXT('r'), 400.0f, 1.0f, 1.0f, 1.0f);
    AddLayer(TEXT("L05_DirtWithGravel"), TEXT("Mask2"), TEXT('g'), 700.0f, 1.0f, 1.0f, 1.0f);
    AddLayer(TEXT("L06_RocksMountain"), TEXT("Mask2"), TEXT('b'), 400.0f, 1.0f, 1.0f, 1.0f);

    AddLayer(TEXT("L07_Asphalt"), TEXT("Mask3"), TEXT('r'), 400.0f, 1.0f, 1.0f, 1.0f);

    // Spare slot �� ������, ���� ������ RGB-����� ����� ������������ G-�����.
    // �� ��������� �������� ����� Weight = 0.
    AddLayer(TEXT("L08_Mask3_G_Spare"), TEXT("Mask3"), TEXT('g'), 400.0f, 1.0f, 1.0f, 0.0f);

    UMaterialExpressionCustom* BaseColorCustom =
        AddCustomNode(
            Material,
            TEXT("WarZ RGB Mask BaseColor"),
            BuildBaseColorCode(Layers),
            CMOT_Float3,
            200,
            -700
        );

    UMaterialExpressionCustom* NormalCustom =
        AddCustomNode(
            Material,
            TEXT("WarZ RGB Mask Normal"),
            BuildNormalCode(Layers),
            CMOT_Float3,
            200,
            -350
        );

    UMaterialExpressionCustom* RoughnessCustom =
        AddCustomNode(
            Material,
            TEXT("WarZ Legacy SpecPow To Roughness"),
            BuildRoughnessCode(Layers),
            CMOT_Float1,
            200,
            0
        );

    AddSharedInputs(
        BaseColorCustom,
        WorldPos,
        WorldUnitScale,
        MaskOriginX,
        MaskOriginY,
        MaskWorldSizeX,
        MaskWorldSizeY,
        MaskFlipY,
        Mask1,
        Mask2,
        Mask3,
        BaseColor,
        BaseNormal,
        BaseTile,
        BaseMultiTile,
        BaseSpecPow,
        Layers
    );

    AddSharedInputs(
        NormalCustom,
        WorldPos,
        WorldUnitScale,
        MaskOriginX,
        MaskOriginY,
        MaskWorldSizeX,
        MaskWorldSizeY,
        MaskFlipY,
        Mask1,
        Mask2,
        Mask3,
        BaseColor,
        BaseNormal,
        BaseTile,
        BaseMultiTile,
        BaseSpecPow,
        Layers
    );

    AddSharedInputs(
        RoughnessCustom,
        WorldPos,
        WorldUnitScale,
        MaskOriginX,
        MaskOriginY,
        MaskWorldSizeX,
        MaskWorldSizeY,
        MaskFlipY,
        Mask1,
        Mask2,
        Mask3,
        BaseColor,
        BaseNormal,
        BaseTile,
        BaseMultiTile,
        BaseSpecPow,
        Layers
    );

    UMaterialExpressionConstant* SpecularConst =
        AddNode<UMaterialExpressionConstant>(Material, 520, 180);

    SpecularConst->R = 0.5f;

    Material->GetExpressionInputForProperty(MP_BaseColor)->Connect(0, BaseColorCustom);
    Material->GetExpressionInputForProperty(MP_Normal)->Connect(0, NormalCustom);
    Material->GetExpressionInputForProperty(MP_Roughness)->Connect(0, RoughnessCustom);
    Material->GetExpressionInputForProperty(MP_Specular)->Connect(0, SpecularConst);

    Material->PreEditChange(nullptr);
    Material->PostEditChange();

    Material->MarkPackageDirty();
    Package->MarkPackageDirty();

    UE_LOG(LogTemp, Warning, TEXT("WarZ RGB Landscape Material created: %s"), *FullPackageName);

    return Material;
#endif
}

// For ShatteredSkies Map
UMaterial* UWarZLandscapeRGBMaskMaterialBuilder::CreateShatteredSkiesRGBMaskLandscapeMaterial(
    const FString& PackagePath,
    const FString& MaterialName)
{
#if !WITH_EDITOR
    return nullptr;
#else
    using namespace WarZLandscapeMaterial;

    FString CleanPackagePath = PackagePath;
    CleanPackagePath.RemoveFromEnd(TEXT("/"));

    const FString FullPackageName = CleanPackagePath / MaterialName;

    UPackage* Package = CreatePackage(*FullPackageName);
    if (!Package)
    {
        UE_LOG(LogTemp, Error, TEXT("ShatteredSkies RGB Material: failed to create package: %s"), *FullPackageName);
        return nullptr;
    }

    Package->FullyLoad();

    UMaterial* Material = FindObject<UMaterial>(Package, *MaterialName);

    if (!Material)
    {
        UMaterialFactoryNew* Factory = NewObject<UMaterialFactoryNew>();

        Material = Cast<UMaterial>(Factory->FactoryCreateNew(
            UMaterial::StaticClass(),
            Package,
            FName(*MaterialName),
            RF_Public | RF_Standalone,
            nullptr,
            GWarn
        ));

        if (!Material)
        {
            UE_LOG(LogTemp, Error, TEXT("ShatteredSkies RGB Material: failed to create material: %s"), *FullPackageName);
            return nullptr;
        }

        FAssetRegistryModule::AssetCreated(Material);
    }
    else
    {
        Material->Modify();

        UMaterialEditorOnlyData* EditorData = Material->GetEditorOnlyData();
        EditorData->ExpressionCollection.Expressions.Empty();
        EditorData->ExpressionCollection.EditorComments.Empty();
    }

    Material->MaterialDomain = EMaterialDomain::MD_Surface;
    Material->BlendMode = EBlendMode::BLEND_Opaque;
    Material->TwoSided = false;
    Material->SetShadingModel(EMaterialShadingModel::MSM_DefaultLit);

    UTexture* DefaultColor = GetDefaultColorTexture();
    UTexture* DefaultNormal = GetDefaultNormalTexture();
    UTexture* DefaultMask = GetDefaultMaskTexture();

    const FName GroupGlobal(TEXT("00_Global"));
    const FName GroupMasks(TEXT("01_RGB_Masks"));
    const FName GroupBase(TEXT("02_Base_Layer"));
    const FName GroupLayers(TEXT("03_ShatteredSkies_Layers"));

    UMaterialExpressionWorldPosition* WorldPos =
        AddNode<UMaterialExpressionWorldPosition>(Material, -2400, -800);

    UMaterialExpressionScalarParameter* WorldUnitScale =
        AddScalar(Material, TEXT("WorldUnitScale"), 100.0f, GroupGlobal, -2400, -620);

    UMaterialExpressionScalarParameter* MaskOriginX =
        AddScalar(Material, TEXT("MaskOriginX"), 0.0f, GroupGlobal, -2400, -460);

    UMaterialExpressionScalarParameter* MaskOriginY =
        AddScalar(Material, TEXT("MaskOriginY"), 0.0f, GroupGlobal, -2400, -300);

    UMaterialExpressionScalarParameter* MaskWorldSizeX =
        AddScalar(Material, TEXT("MaskWorldSizeX"), 409600.0f, GroupGlobal, -2400, -140);

    UMaterialExpressionScalarParameter* MaskWorldSizeY =
        AddScalar(Material, TEXT("MaskWorldSizeY"), 409600.0f, GroupGlobal, -2400, 20);

    UMaterialExpressionScalarParameter* MaskFlipY =
        AddScalar(Material, TEXT("MaskFlipY"), 0.0f, GroupGlobal, -2400, 180);

    UMaterialExpressionTextureObjectParameter* Mask1 =
        AddTextureObject(
            Material,
            TEXT("ShatteredSkies_Mask1_RGB_layers_1_3"),
            DefaultMask,
            SAMPLERTYPE_Masks,
            GroupMasks,
            -2100,
            -800
        );

    UMaterialExpressionTextureObjectParameter* Mask2 =
        AddTextureObject(
            Material,
            TEXT("ShatteredSkies_Mask2_RGB_layers_4_6"),
            DefaultMask,
            SAMPLERTYPE_Masks,
            GroupMasks,
            -2100,
            -620
        );

    UMaterialExpressionTextureObjectParameter* Mask3 =
        AddTextureObject(
            Material,
            TEXT("ShatteredSkies_Mask3_RGB_layers_7_9"),
            DefaultMask,
            SAMPLERTYPE_Masks,
            GroupMasks,
            -2100,
            -440
        );

    UMaterialExpressionTextureObjectParameter* BaseColor =
        AddTextureObject(
            Material,
            TEXT("Base_BaseColor"),
            DefaultColor,
            SAMPLERTYPE_Color,
            GroupBase,
            -1800,
            -800
        );

    UMaterialExpressionTextureObjectParameter* BaseNormal =
        AddTextureObject(
            Material,
            TEXT("Base_Normal"),
            DefaultNormal,
            SAMPLERTYPE_Normal,
            GroupBase,
            -1800,
            -620
        );

    UMaterialExpressionScalarParameter* BaseTile =
        AddScalar(Material, TEXT("Base_Tile"), 1024.0f, GroupBase, -1800, -440);

    UMaterialExpressionScalarParameter* BaseMultiTile =
        AddScalar(Material, TEXT("Base_MultiTile"), 1.0f, GroupBase, -1800, -280);

    UMaterialExpressionScalarParameter* BaseSpecPow =
        AddScalar(Material, TEXT("Base_SpecPow"), 1.0f, GroupBase, -1800, -120);

    TArray<FLayerSlot> Layers;

    auto AddLayer = [&](const TCHAR* Prefix, const TCHAR* MaskVar, TCHAR Channel, float Tile, float MultiTile, float SpecPow, float Weight)
    {
        FLayerSlot L;
        L.Prefix = Prefix;
        L.MaskVar = MaskVar;
        L.Channel = Channel;
        L.Tile = Tile;
        L.MultiTile = MultiTile;
        L.SpecPow = SpecPow;
        L.Weight = Weight;

        const int32 Index = Layers.Num();
        const int32 X = -1500 + (Index % 4) * 360;
        const int32 Y = -800 + (Index / 4) * 700;

        L.BaseColor =
            AddTextureObject(
                Material,
                L.Prefix + TEXT("_BaseColor"),
                DefaultColor,
                SAMPLERTYPE_Color,
                GroupLayers,
                X,
                Y
            );

        L.Normal =
            AddTextureObject(
                Material,
                L.Prefix + TEXT("_Normal"),
                DefaultNormal,
                SAMPLERTYPE_Normal,
                GroupLayers,
                X,
                Y + 160
            );

        L.TileParam =
            AddScalar(
                Material,
                L.Prefix + TEXT("_Tile"),
                Tile,
                GroupLayers,
                X,
                Y + 320
            );

        L.MultiTileParam =
            AddScalar(
                Material,
                L.Prefix + TEXT("_MultiTile"),
                MultiTile,
                GroupLayers,
                X,
                Y + 480
            );

        L.SpecPowParam =
            AddScalar(
                Material,
                L.Prefix + TEXT("_SpecPow"),
                SpecPow,
                GroupLayers,
                X,
                Y + 640
            );

        L.WeightParam =
            AddScalar(
                Material,
                L.Prefix + TEXT("_Weight"),
                Weight,
                GroupLayers,
                X,
                Y + 800
            );

        Layers.Add(L);
    };

    // shatteredskies_mask_layers_1-3_rgba
    // R -> Grass Boulders
    // G -> Sand Dirty Under Road
    // B -> Vertical Cliffs Lo
    AddLayer(TEXT("L01_GrassBoulders"),        TEXT("Mask1"), TEXT('r'), 1000.0f, 1.0f, 1.0f, 1.0f);
    AddLayer(TEXT("L02_SandDirtyUnderRoad"),   TEXT("Mask1"), TEXT('g'), 1024.0f, 1.0f, 1.0f, 1.0f);
    AddLayer(TEXT("L03_VerticalCliffsLo"),     TEXT("Mask1"), TEXT('b'),  150.0f, 1.0f, 1.0f, 1.0f);

    // shatteredskies_mask_layers_4-6_rgba
    // R -> Vertical Cliffs Hi
    // G -> Road Cut Gravel
    // B -> Sediment Gravel
    AddLayer(TEXT("L04_VerticalCliffsHi"),     TEXT("Mask2"), TEXT('r'),  150.0f, 1.0f, 1.0f, 1.0f);
    AddLayer(TEXT("L05_RoadCutGravel"),        TEXT("Mask2"), TEXT('g'), 1024.0f, 1.0f, 1.0f, 1.0f);
    AddLayer(TEXT("L06_SedimentGravel"),       TEXT("Mask2"), TEXT('b'), 1024.0f, 1.0f, 1.0f, 1.0f);

    // shatteredskies_mask_layers_7-9_rgba
    // R -> Snow
    // G -> Asphalt
    // B -> Asphalt Cracked Heavy
    AddLayer(TEXT("L07_Snow"),                 TEXT("Mask3"), TEXT('r'), 1024.0f, 0.5f, 1.0f, 1.0f);
    AddLayer(TEXT("L08_Asphalt"),              TEXT("Mask3"), TEXT('g'),  800.0f, 1.0f, 1.0f, 1.0f);
    AddLayer(TEXT("L09_AsphaltCrackedHeavy"),  TEXT("Mask3"), TEXT('b'),  600.0f, 1.0f, 0.13f, 1.0f);

    UMaterialExpressionCustom* BaseColorCustom =
        AddCustomNode(
            Material,
            TEXT("ShatteredSkies RGB Mask BaseColor"),
            BuildBaseColorCode(Layers),
            CMOT_Float3,
            200,
            -700
        );

    UMaterialExpressionCustom* NormalCustom =
        AddCustomNode(
            Material,
            TEXT("ShatteredSkies RGB Mask Normal"),
            BuildNormalCode(Layers),
            CMOT_Float3,
            200,
            -350
        );

    UMaterialExpressionCustom* RoughnessCustom =
        AddCustomNode(
            Material,
            TEXT("ShatteredSkies Legacy SpecPow To Roughness"),
            BuildRoughnessCode(Layers),
            CMOT_Float1,
            200,
            0
        );

    AddSharedInputs(
        BaseColorCustom,
        WorldPos,
        WorldUnitScale,
        MaskOriginX,
        MaskOriginY,
        MaskWorldSizeX,
        MaskWorldSizeY,
        MaskFlipY,
        Mask1,
        Mask2,
        Mask3,
        BaseColor,
        BaseNormal,
        BaseTile,
        BaseMultiTile,
        BaseSpecPow,
        Layers
    );

    AddSharedInputs(
        NormalCustom,
        WorldPos,
        WorldUnitScale,
        MaskOriginX,
        MaskOriginY,
        MaskWorldSizeX,
        MaskWorldSizeY,
        MaskFlipY,
        Mask1,
        Mask2,
        Mask3,
        BaseColor,
        BaseNormal,
        BaseTile,
        BaseMultiTile,
        BaseSpecPow,
        Layers
    );

    AddSharedInputs(
        RoughnessCustom,
        WorldPos,
        WorldUnitScale,
        MaskOriginX,
        MaskOriginY,
        MaskWorldSizeX,
        MaskWorldSizeY,
        MaskFlipY,
        Mask1,
        Mask2,
        Mask3,
        BaseColor,
        BaseNormal,
        BaseTile,
        BaseMultiTile,
        BaseSpecPow,
        Layers
    );

    UMaterialExpressionConstant* SpecularConst =
        AddNode<UMaterialExpressionConstant>(Material, 520, 180);

    SpecularConst->R = 0.5f;

    Material->GetExpressionInputForProperty(MP_BaseColor)->Connect(0, BaseColorCustom);
    Material->GetExpressionInputForProperty(MP_Normal)->Connect(0, NormalCustom);
    Material->GetExpressionInputForProperty(MP_Roughness)->Connect(0, RoughnessCustom);
    Material->GetExpressionInputForProperty(MP_Specular)->Connect(0, SpecularConst);

    Material->PreEditChange(nullptr);
    Material->PostEditChange();

    Material->MarkPackageDirty();
    Package->MarkPackageDirty();

    UE_LOG(LogTemp, Warning, TEXT("ShatteredSkies RGB Landscape Material created: %s"), *FullPackageName);

    return Material;
#endif
}