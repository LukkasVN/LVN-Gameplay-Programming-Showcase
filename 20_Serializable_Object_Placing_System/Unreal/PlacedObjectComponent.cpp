#include "PlacedObjectComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PlacedObjectSaveData.h"
#include "JsonObjectConverter.h"

UPlacedObjectComponent::UPlacedObjectComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UPlacedObjectComponent::Initialize(UPlaceableItemData* InDefinition)
{
	Definition = InDefinition;
	CacheMaterials();
}


void UPlacedObjectComponent::CacheMaterials()
{
	OriginalMaterials.Empty();

	if (!GetOwner()) return;

	TArray<UMeshComponent*> Meshes;
	GetOwner()->GetComponents<UMeshComponent>(Meshes);

	for (UMeshComponent* Mesh : Meshes)
	{
		if (!Mesh) continue;

		TArray<TObjectPtr<UMaterialInterface>> Mats;
		for (int32 i = 0; i < Mesh->GetNumMaterials(); i++)
		{
			Mats.Add(Mesh->GetMaterial(i));
		}

		OriginalMaterials.Add(Mesh, Mats);
	}
}

void UPlacedObjectComponent::ApplyOverrideMaterial(UMaterialInterface* OverrideMaterial)
{
	if (!GetOwner() || !OverrideMaterial) return;

	TArray<UMeshComponent*> Meshes;
	GetOwner()->GetComponents<UMeshComponent>(Meshes);

	for (UMeshComponent* Mesh : Meshes)
	{
		if (!Mesh) continue;

		for (int32 i = 0; i < Mesh->GetNumMaterials(); i++)
		{
			Mesh->SetMaterial(i, OverrideMaterial);
		}
	}
}

void UPlacedObjectComponent::RestoreMaterials()
{
	if (!GetOwner()) return;

	TArray<UMeshComponent*> Meshes;
	GetOwner()->GetComponents<UMeshComponent>(Meshes);

	for (UMeshComponent* Mesh : Meshes)
	{
		if (!Mesh) continue;

		const TArray<TObjectPtr<UMaterialInterface>>* CachedMats = OriginalMaterials.Find(Mesh);
		if (!CachedMats) continue;

		for (int32 i = 0; i < CachedMats->Num(); i++)
		{
			Mesh->SetMaterial(i, (*CachedMats)[i]);
		}
	}
}

void UPlacedObjectComponent::SnapshotForEdit()
{
	if (!GetOwner()) return;

	SnapshotPosition = GetOwner()->GetActorLocation();
	SnapshotRotation = GetOwner()->GetActorQuat();
}

void UPlacedObjectComponent::RevertToSnapshot()
{
	if (!GetOwner()) return;

	GetOwner()->SetActorLocationAndRotation(SnapshotPosition, SnapshotRotation);
}

void UPlacedObjectComponent::MarkForRemoval(bool bMark)
{
	bMarkedForRemoval = bMark;
}

FString UPlacedObjectComponent::CaptureState()
{
	FPlacedObjectSaveData Data;

	Data.PrefabID = Definition ? Definition->PrefabID : NAME_None;

	if (GetOwner())
	{
		Data.Position = GetOwner()->GetActorLocation();
		Data.Rotation = GetOwner()->GetActorRotation();
		Data.Scale    = GetOwner()->GetActorScale3D();
	}

	FString JsonString;
	FJsonObjectConverter::UStructToJsonObjectString(Data, JsonString);
	return JsonString;
}

void UPlacedObjectComponent::RestoreState(const FString& JsonData)
{
	// Placed actors are spawned by LoadPlacedObjects() in the manager,
	// not restored in-place, so this way RestoreState just applies the transform
	// to the already-spawned actor. The manager calls Initialize() first.
	FPlacedObjectSaveData Data;
	if (!FJsonObjectConverter::JsonObjectStringToUStruct(JsonData, &Data)) return;

	if (GetOwner())
	{
		GetOwner()->SetActorLocation(Data.Position);
		GetOwner()->SetActorRotation(Data.Rotation);
		GetOwner()->SetActorScale3D(Data.Scale);
	}
}