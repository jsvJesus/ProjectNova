#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RoadGeneratorActor.generated.h"

class USplineComponent;
class UProceduralMeshComponent;
class UMaterialInterface;

UCLASS()
class PROJECTNOVA_API ARoadGeneratorActor : public AActor
{
    GENERATED_BODY()

public:
    ARoadGeneratorActor();
    virtual void OnConstruction(const FTransform& Transform) override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
    virtual void PostEditMove(bool bFinished) override;
#endif

    UFUNCTION(CallInEditor, Category = "Road Generator")
    void RebuildRoad();

    UFUNCTION(CallInEditor, Category = "Road Generator")
    void SnapSplinePointsToTerrain();

    UFUNCTION(CallInEditor, Category = "Road Generator")
    void ClearRoadMesh();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Road Generator")
    USplineComponent* RoadSpline;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Road Generator")
    UProceduralMeshComponent* RoadMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road Generator|Shape", meta = (ClampMin = "10.0"))
    float RoadWidth = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road Generator|Shape", meta = (ClampMin = "1", ClampMax = "64"))
    int32 WidthSegments = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road Generator|Shape", meta = (ClampMin = "25.0"))
    float SampleDistance = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road Generator|Shape")
    bool bClosedLoop = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road Generator|Terrain Snap")
    bool bSnapMeshToTerrain = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road Generator|Terrain Snap")
    float ZOffset = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road Generator|Terrain Snap")
    float TraceHeight = 50000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road Generator|Terrain Snap")
    float TraceDepth = 50000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road Generator|Terrain Snap")
    TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road Generator|Material")
    UMaterialInterface* RoadMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road Generator|Material", meta = (ClampMin = "1.0"))
    float UVLengthTiling = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road Generator|Material", meta = (ClampMin = "1.0"))
    float UVWidthTiling = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road Generator|Collision")
    bool bCreateCollision = false;

private:
    bool TraceDownToTerrain(const FVector& WorldPosition, FVector& OutHitPosition) const;
    FVector GetRightVectorAtDistance(float Distance) const;
};