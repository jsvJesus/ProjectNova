#include "WorldAsset/RoadGeneratorActor.h"

#include "Components/SplineComponent.h"
#include "ProceduralMeshComponent.h"
#include "KismetProceduralMeshLibrary.h"
#include "Engine/World.h"

ARoadGeneratorActor::ARoadGeneratorActor()
{
	PrimaryActorTick.bCanEverTick = false;

	RoadSpline = CreateDefaultSubobject<USplineComponent>(TEXT("RoadSpline"));
	RootComponent = RoadSpline;

	RoadSpline->SetMobility(EComponentMobility::Movable);
	RoadSpline->bDrawDebug = true;

	RoadMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("RoadMesh"));
	RoadMesh->SetupAttachment(RootComponent);
	RoadMesh->SetMobility(EComponentMobility::Movable);
	RoadMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ARoadGeneratorActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	RebuildRoad();
}

#if WITH_EDITOR
void ARoadGeneratorActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	RebuildRoad();
}

void ARoadGeneratorActor::PostEditMove(bool bFinished)
{
	Super::PostEditMove(bFinished);

	if (bFinished)
	{
		RebuildRoad();
	}
}
#endif

bool ARoadGeneratorActor::TraceDownToTerrain(const FVector& WorldPosition, FVector& OutHitPosition) const
{
	if (!bSnapMeshToTerrain)
	{
		OutHitPosition = WorldPosition;
		return false;
	}

	UWorld* World = GetWorld();

	if (!World)
	{
		OutHitPosition = WorldPosition;
		return false;
	}

	const FVector Start = WorldPosition + FVector(0.0f, 0.0f, TraceHeight);
	const FVector End = WorldPosition - FVector(0.0f, 0.0f, TraceDepth);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.bTraceComplex = false;

	const bool bHit = World->LineTraceSingleByChannel(
		Hit,
		Start,
		End,
		TraceChannel,
		Params
	);

	if (bHit)
	{
		OutHitPosition = Hit.Location + FVector(0.0f, 0.0f, ZOffset);
		return true;
	}

	OutHitPosition = WorldPosition;
	return false;
}

FVector ARoadGeneratorActor::GetRightVectorAtDistance(float Distance) const
{
	FVector Tangent = RoadSpline->GetTangentAtDistanceAlongSpline(
		Distance,
		ESplineCoordinateSpace::World
	).GetSafeNormal();

	if (Tangent.IsNearlyZero())
	{
		Tangent = GetActorForwardVector();
	}

	FVector Right = FVector::CrossProduct(FVector::UpVector, Tangent).GetSafeNormal();

	if (Right.IsNearlyZero())
	{
		Right = GetActorRightVector();
	}

	return Right;
}

void ARoadGeneratorActor::RebuildRoad()
{
	if (!RoadSpline || !RoadMesh)
    {
        return;
    }

    RoadSpline->SetClosedLoop(bClosedLoop);

    RoadMesh->ClearAllMeshSections();

    if (RoadMaterial)
    {
        RoadMesh->SetMaterial(0, RoadMaterial);
    }

    RoadMesh->SetCollisionEnabled(
        bCreateCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision
    );

    const float SplineLength = RoadSpline->GetSplineLength();

    if (SplineLength <= 10.0f)
    {
        return;
    }

    const float SafeSampleDistance = FMath::Max(25.0f, SampleDistance);
    const int32 LengthSegments = FMath::Max(1, FMath::CeilToInt(SplineLength / SafeSampleDistance));
    const int32 LengthPoints = LengthSegments + 1;

    const int32 SafeWidthSegments = FMath::Clamp(WidthSegments, 1, 64);
    const int32 WidthPoints = SafeWidthSegments + 1;

    const float HalfWidth = RoadWidth * 0.5f;

    TArray<FVector> Vertices;
    TArray<int32> Triangles;
    TArray<FVector2D> UVs;
    TArray<FLinearColor> VertexColors;

    Vertices.Reserve(LengthPoints * WidthPoints);
    UVs.Reserve(LengthPoints * WidthPoints);
    VertexColors.Reserve(LengthPoints * WidthPoints);
    Triangles.Reserve(LengthSegments * SafeWidthSegments * 6);

    const FTransform MeshTransform = RoadMesh->GetComponentTransform();

    for (int32 Y = 0; Y < LengthPoints; ++Y)
    {
        const float LengthAlpha = static_cast<float>(Y) / static_cast<float>(LengthPoints - 1);
        const float Distance = SplineLength * LengthAlpha;

        const FVector CenterWorld = RoadSpline->GetLocationAtDistanceAlongSpline(
            Distance,
            ESplineCoordinateSpace::World
        );

        const FVector RightWorld = GetRightVectorAtDistance(Distance);

        for (int32 X = 0; X < WidthPoints; ++X)
        {
            const float WidthAlpha = static_cast<float>(X) / static_cast<float>(WidthPoints - 1);
            const float WidthOffset = FMath::Lerp(-HalfWidth, HalfWidth, WidthAlpha);

            FVector WorldPos = CenterWorld + RightWorld * WidthOffset;

            FVector HitPos;
            TraceDownToTerrain(WorldPos, HitPos);

            const FVector LocalPos = MeshTransform.InverseTransformPosition(HitPos);

            Vertices.Add(LocalPos);

            const float U = Distance / FMath::Max(1.0f, UVLengthTiling);
            const float V = (WidthOffset + HalfWidth) / FMath::Max(1.0f, UVWidthTiling);

            UVs.Add(FVector2D(U, V));

            const float EdgeFade = FMath::Min(WidthAlpha, 1.0f - WidthAlpha) * 2.0f;
            VertexColors.Add(FLinearColor(1.0f, 1.0f, 1.0f, EdgeFade));
        }
    }

    for (int32 Y = 0; Y < LengthSegments; ++Y)
    {
        for (int32 X = 0; X < SafeWidthSegments; ++X)
        {
            const int32 A = Y * WidthPoints + X;
            const int32 B = Y * WidthPoints + X + 1;
            const int32 C = (Y + 1) * WidthPoints + X;
            const int32 D = (Y + 1) * WidthPoints + X + 1;

            Triangles.Add(A);
            Triangles.Add(C);
            Triangles.Add(B);

            Triangles.Add(B);
            Triangles.Add(C);
            Triangles.Add(D);
        }
    }

    TArray<FVector> Normals;
    TArray<FProcMeshTangent> Tangents;

    UKismetProceduralMeshLibrary::CalculateTangentsForMesh(
        Vertices,
        Triangles,
        UVs,
        Normals,
        Tangents
    );

    RoadMesh->CreateMeshSection_LinearColor(
        0,
        Vertices,
        Triangles,
        Normals,
        UVs,
        VertexColors,
        Tangents,
        bCreateCollision
    );
}

void ARoadGeneratorActor::SnapSplinePointsToTerrain()
{
	if (!RoadSpline)
	{
		return;
	}

	const int32 PointCount = RoadSpline->GetNumberOfSplinePoints();

	for (int32 i = 0; i < PointCount; ++i)
	{
		const FVector PointWorld = RoadSpline->GetLocationAtSplinePoint(
			i,
			ESplineCoordinateSpace::World
		);

		FVector HitPos;

		if (TraceDownToTerrain(PointWorld, HitPos))
		{
			const FVector LocalPos = RoadSpline->GetComponentTransform().InverseTransformPosition(HitPos);

			RoadSpline->SetLocationAtSplinePoint(
				i,
				LocalPos,
				ESplineCoordinateSpace::Local,
				false
			);
		}
	}

	RoadSpline->UpdateSpline();

	RebuildRoad();
}

void ARoadGeneratorActor::ClearRoadMesh()
{
	if (RoadMesh)
	{
		RoadMesh->ClearAllMeshSections();
	}
}
