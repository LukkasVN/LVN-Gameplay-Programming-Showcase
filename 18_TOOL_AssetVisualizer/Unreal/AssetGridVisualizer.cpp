#include "AssetGridVisualizer.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h"

bool FAssetGrid::HasConfigChanged() const
{
	int32 TotalAssets = 0;
	for (const FAssetRow& Row : Rows)
	{
		TotalAssets += Row.Assets.Num();
	}
	return TotalAssets != InstantiatedAssets.Num();
}

AAssetGridVisualizer::AAssetGridVisualizer()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AAssetGridVisualizer::BeginPlay()
{
	Super::BeginPlay();
}

void AAssetGridVisualizer::BeginDestroy()
{
	CleanupGridContainers();
	Super::BeginDestroy();
}

#if WITH_EDITOR
void AAssetGridVisualizer::Destroyed()
{
	CleanupGridContainers();
	Super::Destroyed();
}
#endif

void AAssetGridVisualizer::RefreshLayout()
{
	RecalculateLayout();
}

void AAssetGridVisualizer::ResetScript()
{
	CleanupGridContainers();
	Grids.Empty();
}

void AAssetGridVisualizer::RecalculateLayout()
{
	CleanupGridContainers();

	if (Grids.Num() == 0)
		return;

	FVector ActorWorldPosition = GetActorLocation();
	
	float CurrentYOffset = 0.0f;

	for (int32 i = 0; i < Grids.Num(); ++i)
	{
		InstantiateGrid(i, ActorWorldPosition, CurrentYOffset);
		CurrentYOffset += GetGridWidth(i) + GridSeparation;
	}
}

void AAssetGridVisualizer::InstantiateGrid(int32 GridIndex, FVector ActorWorldPosition, float YOffset)
{
	if (!Grids.IsValidIndex(GridIndex))
		return;

	FAssetGrid& Grid = Grids[GridIndex];
	Grid.InstantiatedAssets.Empty();

	float CurrentX = 0.0f;

	for (int32 RowIndex = 0; RowIndex < Grid.Rows.Num(); ++RowIndex)
	{
		FAssetRow& Row = Grid.Rows[RowIndex];
		float RowWidth = 0.0f;
		float CurrentY = YOffset;

		for (int32 ColIndex = 0; ColIndex < Row.Assets.Num(); ++ColIndex)
		{
			FVisualAssetData& AssetData = Row.Assets[ColIndex];

			if (!AssetData.PrefabReference)
				continue;

			FVector WorldPosition = ActorWorldPosition + FVector(CurrentX, CurrentY, 0.0f) + AssetData.PositionOffset;
			AActor* SpawnedActor = SpawnFromReference(AssetData.PrefabReference, WorldPosition, AssetData.Rotation);

			if (SpawnedActor)
			{
				SpawnedActor->SetActorLocation(WorldPosition);
				SpawnedActor->SetActorRotation(AssetData.Rotation);
				SpawnedActor->SetActorScale3D(FVector(AssetData.ScaleMultiplier));

				Grid.InstantiatedAssets.Add(SpawnedActor);

				FVector Bounds = GetAssetBounds(SpawnedActor);
				RowWidth = FMath::Max(RowWidth, Bounds.Y);
				
				CurrentY += Bounds.Y + Grid.HorizontalSeparation;
			}
		}

		CurrentX -= RowWidth + Grid.VerticalSeparation;
	}
}

AActor* AAssetGridVisualizer::SpawnFromReference(UObject* Reference, const FVector& Location, const FRotator& Rotation)
{
	if (!Reference)
		return nullptr;

	// Only accept Static Mesh for this version
	if (UStaticMesh* Mesh = Cast<UStaticMesh>(Reference))
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.bNoFail = true;

		AStaticMeshActor* MeshActor = GetWorld()->SpawnActor<AStaticMeshActor>(Location, Rotation, SpawnParams);
		if (MeshActor)
		{
			MeshActor->GetStaticMeshComponent()->SetStaticMesh(Mesh);
			return MeshActor;
		}
	}

	return nullptr;
}

float AAssetGridVisualizer::GetGridWidth(int32 GridIndex) const
{
	if (!Grids.IsValidIndex(GridIndex))
		return 0.0f;

	const FAssetGrid& Grid = Grids[GridIndex];
	float MaxWidth = 0.0f;

	for (const FAssetRow& Row : Grid.Rows)
	{
		float RowWidth = 0.0f;
		for (const FVisualAssetData& AssetData : Row.Assets)
		{
			if (!AssetData.PrefabReference)
				continue;

			FVector Bounds = GetPrefabBounds(AssetData.PrefabReference);
			RowWidth += Bounds.Y * AssetData.ScaleMultiplier + Grid.HorizontalSeparation;
		}
		MaxWidth = FMath::Max(MaxWidth, RowWidth);
	}

	return MaxWidth;
}

void AAssetGridVisualizer::CleanupGridContainers()
{
	for (FAssetGrid& Grid : Grids)
	{
		for (AActor* Asset : Grid.InstantiatedAssets)
		{
			if (Asset && !Asset->IsActorBeingDestroyed())
			{
#if WITH_EDITOR
				if (GIsEditor && GetWorld() && !GetWorld()->IsGameWorld())
				{
					Asset->SetFlags(RF_Transient);
				}
#endif
				Asset->Destroy();
			}
		}
		Grid.InstantiatedAssets.Empty();
	}
}

FVector AAssetGridVisualizer::GetAssetBounds(AActor* Asset) const
{
	if (!Asset)
		return FVector(0.1f, 0.1f, 0.1f);

	FVector Origin, BoxExtent;
	Asset->GetActorBounds(false, Origin, BoxExtent);
	return BoxExtent * 2.0f;
}

FVector AAssetGridVisualizer::GetPrefabBounds(UObject* PrefabReference) const
{
	if (!PrefabReference)
		return FVector(0.1f, 0.1f, 0.1f);

	// Only accept Static Mesh for this version
	if (UStaticMesh* Mesh = Cast<UStaticMesh>(PrefabReference))
	{
		FBox MeshBounds = Mesh->GetBoundingBox();
		return MeshBounds.GetSize();
	}

	return FVector(0.1f, 0.1f, 0.1f);
}