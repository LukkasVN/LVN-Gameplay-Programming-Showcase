#include "ObjectPlacementManager.h"
#include "Camera/CameraComponent.h"
#include "Components/MeshComponent.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Mechanics_Test_LVN/FP_Character.h"
#include "Mechanics_Test_LVN/SaveGameData.h"
#include "Mechanics_Test_LVN/SaveDataEntry.h"
#include "Mechanics_Test_LVN/GUIDComponent.h"
#include "PlacedObjectSaveData.h"
#include "JsonObjectConverter.h"

UObjectPlacementManager::UObjectPlacementManager()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UObjectPlacementManager::BeginPlay()
{
	Super::BeginPlay();
}

void UObjectPlacementManager::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CurrentMode == EPlacementMode::None) return;

	switch (CurrentMode)
	{
		case EPlacementMode::Placing:  TickPlacing();  break;
		case EPlacementMode::Editing:  TickEditing();  break;
		case EPlacementMode::Removing: TickRemoving(); break;
		default: break;
	}
}

void UObjectPlacementManager::EnterPlacingMode(UPlaceableItemData* Item)
{
	if (!Item) return;
	ExitCurrentMode();

	SelectedItem   = Item;
	CurrentMode    = EPlacementMode::Placing;
	RotationOffset = 0.f;

	SpawnPreview(Item);
	SetBobbingEnabled(false);

	UE_LOG(LogTemp, Log, TEXT("[ObjectPlacer] Placing: %s"), *Item->DisplayName.ToString());
}

void UObjectPlacementManager::EnterEditMode()
{
	ExitCurrentMode();

	CurrentMode = EPlacementMode::Editing;
	EditPhase   = EEditPhase::SelectingObject;

	SetBobbingEnabled(false);

	UE_LOG(LogTemp, Log, TEXT("[ObjectPlacer] Edit mode."));
}

void UObjectPlacementManager::EnterRemoveMode()
{
	ExitCurrentMode();

	CurrentMode = EPlacementMode::Removing;

	SetBobbingEnabled(false);

	UE_LOG(LogTemp, Log, TEXT("[ObjectPlacer] Remove mode."));
}

void UObjectPlacementManager::ExitCurrentMode()
{
	if (CurrentMode == EPlacementMode::None) return; 

	DestroyPreview();
	ClearTargetedActor();

	EditPhase      = EEditPhase::SelectingObject;
	RotationOffset = 0.f;
	SelectedItem   = nullptr;
	CurrentMode    = EPlacementMode::None;

	SetBobbingEnabled(true);
}

void UObjectPlacementManager::OnConfirmInput()
{
	switch (CurrentMode)
	{
		case EPlacementMode::Placing:
		{
			FHitResult Hit;
			EPlacementSurfaceType SurfaceType;
			bool bValid = EvaluatePlacement(SelectedItem, Hit, SurfaceType);
			if (bValid) bValid = !IsPreviewOverlapping();
			if (bValid) CommitPlacement();
			break;
		}
		case EPlacementMode::Editing:
		{
			if (EditPhase == EEditPhase::SelectingObject && TargetedActor)
			{
				BeginRepositioning(TargetedActor);
			}
			else if (EditPhase == EEditPhase::RepositioningObject)
			{
				UPlacedObjectComponent* POC = Cast<UPlacedObjectComponent>(
					TargetedActor->GetComponentByClass(UPlacedObjectComponent::StaticClass()));

				FHitResult Hit;
				EPlacementSurfaceType SurfaceType;
				bool bValid = EvaluatePlacement(POC ? POC->Definition : nullptr, Hit, SurfaceType);
				if (bValid) bValid = !IsPreviewOverlapping();
				if (bValid) CommitEdit();
			}
			break;
		}
		case EPlacementMode::Removing:
		{
			if (TargetedActor) StageForRemoval(TargetedActor);
			break;
		}
		default: break;
	}
}

void UObjectPlacementManager::OnCancelInput()
{
	if (CurrentMode == EPlacementMode::Editing
		&& EditPhase == EEditPhase::RepositioningObject)
	{
		CancelEdit();
		return;
	}

	ExitCurrentMode();
}

void UObjectPlacementManager::OnRotateInput()
{
	if (CurrentMode == EPlacementMode::None) return;

	UPlaceableItemData* ActiveItem = nullptr;

	if (CurrentMode == EPlacementMode::Placing)
	{
		ActiveItem = SelectedItem;
	}
	else if (CurrentMode == EPlacementMode::Editing
		&& EditPhase == EEditPhase::RepositioningObject && TargetedActor)
	{
		UPlacedObjectComponent* POC = Cast<UPlacedObjectComponent>(
			TargetedActor->GetComponentByClass(UPlacedObjectComponent::StaticClass()));
		if (POC) ActiveItem = POC->Definition;
	}

	if (!ActiveItem) return;

	RotationOffset += ActiveItem->RotationStep;

	FHitResult Hit;
	EPlacementSurfaceType SurfaceType;
	bool bValid = EvaluatePlacement(ActiveItem, Hit, SurfaceType);
	PositionAndRotatePreview(bValid, Hit, ActiveItem);
}

void UObjectPlacementManager::TickPlacing()
{
	FHitResult Hit;
	EPlacementSurfaceType SurfaceType;

	bool bValid = EvaluatePlacement(SelectedItem, Hit, SurfaceType);
	PositionAndRotatePreview(bValid, Hit, SelectedItem);

	if (bValid) bValid = !IsPreviewOverlapping();

	SetPreviewMaterial(bValid ? HoverValidMaterial : HoverInvalidMaterial);
	bLastValidState = bValid;
}

void UObjectPlacementManager::TickEditing()
{
	switch (EditPhase)
	{
		case EEditPhase::SelectingObject:     TickEditSelection();     break;
		case EEditPhase::RepositioningObject: TickEditRepositioning(); break;
	}
}

void UObjectPlacementManager::TickRemoving()
{
	AActor* Hovered = LineTraceForPlacedActor();

	if (Hovered != TargetedActor)
	{
		ClearTargetedActor();
		TargetedActor = Hovered;

		if (TargetedActor)
		{
			UPlacedObjectComponent* POC = Cast<UPlacedObjectComponent>(
				TargetedActor->GetComponentByClass(UPlacedObjectComponent::StaticClass()));
			if (POC) POC->ApplyOverrideMaterial(HoverInvalidMaterial);
		}
	}
}

void UObjectPlacementManager::TickEditSelection()
{
	AActor* Hovered = LineTraceForPlacedActor();

	if (Hovered != TargetedActor)
	{
		ClearTargetedActor();
		TargetedActor = Hovered;

		if (TargetedActor)
		{
			UPlacedObjectComponent* POC = Cast<UPlacedObjectComponent>(
				TargetedActor->GetComponentByClass(UPlacedObjectComponent::StaticClass()));
			if (POC) POC->ApplyOverrideMaterial(HoverEditMaterial);
		}
	}
}

void UObjectPlacementManager::TickEditRepositioning()
{
	UPlacedObjectComponent* POC = TargetedActor
		? Cast<UPlacedObjectComponent>(
			TargetedActor->GetComponentByClass(UPlacedObjectComponent::StaticClass()))
		: nullptr;

	if (!POC || !POC->Definition) return;

	FHitResult Hit;
	EPlacementSurfaceType SurfaceType;

	bool bValid = EvaluatePlacement(POC->Definition, Hit, SurfaceType);
	PositionAndRotatePreview(bValid, Hit, POC->Definition);

	if (bValid) bValid = !IsPreviewOverlapping();

	SetPreviewMaterial(bValid ? HoverEditMaterial : HoverInvalidMaterial);
	bLastValidState = bValid;
}

bool UObjectPlacementManager::EvaluatePlacement(UPlaceableItemData* Item,FHitResult& OutHit, EPlacementSurfaceType& OutSurfaceType)
{
	OutSurfaceType = EPlacementSurfaceType::Floor;

	if (!Item) return false;

	UCameraComponent* Cam = GetPlayerCamera();
	if (!Cam) return false;

	FVector Start = Cam->GetComponentLocation();
	FVector End   = Start + Cam->GetForwardVector() * PlacementDistance;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());
	if (PreviewActor) Params.AddIgnoredActor(PreviewActor);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		OutHit, Start, End, PlacementChannel, Params);

	if (!bHit) return false;

	// If RequiredSurfaceTag is set, the hit actor must have it
	if (!RequiredSurfaceTag.IsNone() && OutHit.GetActor())
	{
		if (!OutHit.GetActor()->ActorHasTag(RequiredSurfaceTag))
			return false;
	}

	OutSurfaceType = ClassifySurface(OutHit.ImpactNormal);
	return Item->IsSurfaceAllowed(OutSurfaceType);
}

EPlacementSurfaceType UObjectPlacementManager::ClassifySurface(const FVector& Normal) const
{
	float Dot = FVector::DotProduct(Normal, FVector::UpVector);

	if (Dot >  0.7f) return EPlacementSurfaceType::Floor;
	if (Dot < -0.7f) return EPlacementSurfaceType::Ceiling;
	return EPlacementSurfaceType::Wall;
}

bool UObjectPlacementManager::IsPreviewOverlapping() const
{
    if (!PreviewActor || !GetWorld()) return false;

    FBox LocalCombinedBox(EForceInit::ForceInitToZero);
    bool bHasAny = false;

    const FTransform ActorWorldInverse = PreviewActor->GetActorTransform().Inverse();

    for (UMeshComponent* Mesh : PreviewMeshes)
    {
        if (!Mesh) continue;

        const FBox ComponentLocalBox = Mesh->CalcLocalBounds().GetBox();

        const FTransform ComponentToActorLocal =
            Mesh->GetComponentTransform() * ActorWorldInverse;

        LocalCombinedBox += ComponentLocalBox.TransformBy(ComponentToActorLocal);
        bHasAny = true;
    }

    if (!bHasAny)
    {
        LocalCombinedBox = FBox::BuildAABB(FVector::ZeroVector, FVector(50.f));
    }

    const FVector WorldCenter =
        PreviewActor->GetActorTransform().TransformPosition(LocalCombinedBox.GetCenter());

    const FVector HalfExt = LocalCombinedBox.GetExtent() * 0.85f;

    FCollisionShape Box = FCollisionShape::MakeBox(HalfExt);

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetOwner());
    Params.AddIgnoredActor(PreviewActor);

    TArray<FOverlapResult> Overlaps;

    return GetWorld()->OverlapMultiByChannel(
        Overlaps,
        WorldCenter,
        PreviewActor->GetActorQuat(),
        PlacedObjectChannel,
        Box, Params);
}

void UObjectPlacementManager::SpawnPreview(UPlaceableItemData* Item)
{
	DestroyPreview();
	if (!Item || !GetWorld()) return;

	UClass* ActorClass = Item->ActorClass.LoadSynchronous();
	if (!ActorClass) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FVector HiddenLocation = FVector(0.f, 0.f, -100000.f); // below the map
	PreviewActor = GetWorld()->SpawnActor<AActor>(
		ActorClass, HiddenLocation, FRotator::ZeroRotator, Params);

	if (!PreviewActor) return;

	TArray<UPrimitiveComponent*> Primitives;
	PreviewActor->GetComponents<UPrimitiveComponent>(Primitives);
	for (UPrimitiveComponent* Prim : Primitives)
		if (Prim) Prim->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PreviewMeshes.Empty();
	TArray<UMeshComponent*> Meshes;
	PreviewActor->GetComponents<UMeshComponent>(Meshes);
	for (UMeshComponent* Mesh : Meshes)
		if (Mesh) PreviewMeshes.Add(Mesh);

	bLastValidState = false;
	SetPreviewMaterial(HoverInvalidMaterial);
}

void UObjectPlacementManager::DestroyPreview()
{
	if (PreviewActor)
	{
		PreviewActor->Destroy();
		PreviewActor = nullptr;
	}
	PreviewMeshes.Empty();
}

void UObjectPlacementManager::PositionAndRotatePreview(bool bValidHit,
	const FHitResult& Hit, UPlaceableItemData* Item)
{
	if (!PreviewActor || !Item) return;

	UCameraComponent* Cam = GetPlayerCamera();
	if (!Cam) return;

	FHitResult DisplayHit = Hit;
	bool bHasDisplayHit   = bValidHit;

	if (!bValidHit)
	{
		FVector Start = Cam->GetComponentLocation();
		FVector End   = Start + Cam->GetForwardVector() * PlacementDistance;

		FCollisionQueryParams Params;
		Params.AddIgnoredActor(GetOwner());
		if (PreviewActor) Params.AddIgnoredActor(PreviewActor);

		bHasDisplayHit = GetWorld()->LineTraceSingleByChannel(
			DisplayHit, Start, End, PlacementChannel, Params);
	}

	if (!bHasDisplayHit)
	{
		PreviewActor->SetActorLocation(FVector(0.f, 0.f, -100000.f));
		return;
	}

	FQuat NewRotation;

	if (Item->bSnapRotationToSurface)
	{
		FVector Normal = DisplayHit.ImpactNormal;

		FVector WorldRef = (FMath::Abs(FVector::DotProduct(Normal, FVector::UpVector)) < 0.9f)
			? FVector::UpVector
			: FVector::ForwardVector;

		FQuat BaseRotation = FRotationMatrix::MakeFromZX(Normal, WorldRef).ToQuat();
		FQuat OffsetSpin   = FQuat(Normal, FMath::DegreesToRadians(RotationOffset));
		NewRotation        = OffsetSpin * BaseRotation;
	}
	else
	{
		NewRotation = FRotator(0.f, RotationOffset, 0.f).Quaternion();
	}

	PreviewActor->SetActorLocationAndRotation(DisplayHit.ImpactPoint, NewRotation);
	PreviewActor->MarkComponentsRenderStateDirty();

	FBox Bounds(EForceInit::ForceInitToZero);
	bool bHasAny = false;

	for (UMeshComponent* Mesh : PreviewMeshes)
	{
		if (!Mesh) continue;
		Bounds += Mesh->CalcBounds(Mesh->GetComponentTransform()).GetBox();
		bHasAny = true;
	}

	if (!bHasAny) return;

	FVector BoundsCenter = Bounds.GetCenter();
	FVector BoundsExtent = Bounds.GetExtent();
	FVector Normal       = DisplayHit.ImpactNormal;

	float ExtentAlongNormal =
		FMath::Abs(BoundsExtent.X * Normal.X) +
		FMath::Abs(BoundsExtent.Y * Normal.Y) +
		FMath::Abs(BoundsExtent.Z * Normal.Z);

	float PivotToCenterAlongNormal = FVector::DotProduct(BoundsCenter - DisplayHit.ImpactPoint, Normal);

	float FinalOffset = ExtentAlongNormal - PivotToCenterAlongNormal;
	PreviewActor->SetActorLocation(DisplayHit.ImpactPoint + Normal * FinalOffset);
}

void UObjectPlacementManager::SetPreviewMaterial(UMaterialInterface* Mat)
{
	if (!Mat) return;

	for (UMeshComponent* Mesh : PreviewMeshes)
	{
		if (!Mesh) continue;
		for (int32 i = 0; i < Mesh->GetNumMaterials(); i++)
			Mesh->SetMaterial(i, Mat);
	}
}

void UObjectPlacementManager::UpdatePreviewMaterial(UMaterialInterface* Mat)
{
	bool bNowValid = (Mat == HoverValidMaterial || Mat == HoverEditMaterial);
	if (bNowValid == bLastValidState) return;

	SetPreviewMaterial(Mat);
	bLastValidState = bNowValid;
}

AActor* UObjectPlacementManager::LineTraceForPlacedActor() const
{
	UCameraComponent* Cam = GetPlayerCamera();
	if (!Cam) return nullptr;

	FVector Start = Cam->GetComponentLocation();
	FVector End   = Start + Cam->GetForwardVector() * PlacementDistance;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());

	if (!GetWorld()->LineTraceSingleByChannel(Hit, Start, End, PlacedObjectChannel, Params))
		return nullptr;

	AActor* HitActor = Hit.GetActor();
	if (!HitActor) return nullptr;

	return HitActor->GetComponentByClass(UPlacedObjectComponent::StaticClass())
		? HitActor : nullptr;
}

void UObjectPlacementManager::RegisterPlacedActor(AActor* Actor, UPlaceableItemData* Item)
{
	if (!Actor || !Item) return;

	UPlacedObjectComponent* POC = Cast<UPlacedObjectComponent>(
		Actor->GetComponentByClass(UPlacedObjectComponent::StaticClass()));

	if (!POC)
	{
		POC = NewObject<UPlacedObjectComponent>(Actor);
		POC->RegisterComponent();
	}

	POC->Initialize(Item);

	if (!Actor->FindComponentByClass<UGUIDComponent>())
	{
		UGUIDComponent* GUIDComp = NewObject<UGUIDComponent>(Actor);
		GUIDComp->RegisterComponent();
	}

	TArray<UPrimitiveComponent*> Primitives;
	Actor->GetComponents<UPrimitiveComponent>(Primitives);
	for (UPrimitiveComponent* Prim : Primitives)
		if (Prim) Prim->SetCollisionResponseToChannel(PlacedObjectChannel, ECR_Block);

	PlacedActors.Add(Actor);
}

void UObjectPlacementManager::ClearTargetedActor()
{
	if (TargetedActor)
	{
		UPlacedObjectComponent* POC = Cast<UPlacedObjectComponent>(
			TargetedActor->GetComponentByClass(UPlacedObjectComponent::StaticClass()));
		if (POC) POC->RestoreMaterials();
	}
	TargetedActor = nullptr;
}

void UObjectPlacementManager::CommitPlacement()
{
	if (!SelectedItem || !PreviewActor) return;

	UClass* ActorClass = SelectedItem->ActorClass.LoadSynchronous();
	if (!ActorClass) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* Placed = GetWorld()->SpawnActor<AActor>(
		ActorClass,
		PreviewActor->GetActorLocation(),
		PreviewActor->GetActorRotation(),
		Params);

	if (!Placed) return;

	RegisterPlacedActor(Placed, SelectedItem);

	UE_LOG(LogTemp, Log, TEXT("[ObjectPlacer] Placed: %s"),
		*SelectedItem->DisplayName.ToString());

	RotationOffset  = 0.f;
	bLastValidState = false;
}

void UObjectPlacementManager::BeginRepositioning(AActor* Actor)
{
	if (!Actor) return;

	UPlacedObjectComponent* POC = Cast<UPlacedObjectComponent>(
		Actor->GetComponentByClass(UPlacedObjectComponent::StaticClass()));

	if (!POC || !POC->Definition)
	{
		UE_LOG(LogTemp, Error,
			TEXT("[ObjectPlacer] Cannot edit '%s' — Definition is null."), *Actor->GetName());
		ExitCurrentMode();
		return;
	}

	POC->SnapshotForEdit();
	RotationOffset = Actor->GetActorRotation().Yaw;

	SpawnPreview(POC->Definition);
	if (PreviewActor)
	{
		PreviewActor->SetActorLocationAndRotation(
			Actor->GetActorLocation(), Actor->GetActorRotation());
		SetPreviewMaterial(HoverEditMaterial);
	}

	bLastValidState = true;
	TargetedActor   = Actor;
	Actor->SetActorHiddenInGame(true);

	EditPhase = EEditPhase::RepositioningObject;

	UE_LOG(LogTemp, Log, TEXT("[ObjectPlacer] Repositioning: %s"),
		*POC->Definition->DisplayName.ToString());
}

void UObjectPlacementManager::CommitEdit()
{
	if (!TargetedActor || !PreviewActor) return;

	TargetedActor->SetActorHiddenInGame(false);
	TargetedActor->SetActorLocationAndRotation(
		PreviewActor->GetActorLocation(), PreviewActor->GetActorRotation());

	UPlacedObjectComponent* POC = Cast<UPlacedObjectComponent>(
		TargetedActor->GetComponentByClass(UPlacedObjectComponent::StaticClass()));
	if (POC) POC->RestoreMaterials();

	DestroyPreview();
	RotationOffset = 0.f;
	TargetedActor  = nullptr;
	EditPhase      = EEditPhase::SelectingObject;

	UE_LOG(LogTemp, Log, TEXT("[ObjectPlacer] Edit committed."));
}

void UObjectPlacementManager::CancelEdit()
{
	if (!TargetedActor) return;

	TargetedActor->SetActorHiddenInGame(false);

	UPlacedObjectComponent* POC = Cast<UPlacedObjectComponent>(
		TargetedActor->GetComponentByClass(UPlacedObjectComponent::StaticClass()));
	if (POC)
	{
		POC->RevertToSnapshot();
		POC->RestoreMaterials();
	}

	DestroyPreview();
	RotationOffset = 0.f;
	TargetedActor  = nullptr;
	EditPhase      = EEditPhase::SelectingObject;

	UE_LOG(LogTemp, Log, TEXT("[ObjectPlacer] Edit cancelled — reverted to snapshot."));
}

void UObjectPlacementManager::StageForRemoval(AActor* Actor)
{
	if (!Actor) return;

	UPlacedObjectComponent* POC = Cast<UPlacedObjectComponent>(
		Actor->GetComponentByClass(UPlacedObjectComponent::StaticClass()));
	if (POC) POC->MarkForRemoval(true);

	Actor->SetActorHiddenInGame(true);
	TargetedActor = nullptr;

	FString Name = POC && POC->Definition
		? POC->Definition->DisplayName.ToString()
		: Actor->GetName();

	UE_LOG(LogTemp, Log,
		TEXT("[ObjectPlacer] Staged for removal: %s. Save to commit."), *Name);
}

void UObjectPlacementManager::PrepareForSave()
{
	for (int32 i = PlacedActors.Num() - 1; i >= 0; i--)
	{
		AActor* Actor = PlacedActors[i];
		if (!Actor) { PlacedActors.RemoveAt(i); continue; }

		UPlacedObjectComponent* POC = Cast<UPlacedObjectComponent>(
			Actor->GetComponentByClass(UPlacedObjectComponent::StaticClass()));

		if (POC && POC->IsMarkedForRemoval())
		{
			Actor->Destroy();
			PlacedActors.RemoveAt(i);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[ObjectPlacer] Prepared for save — %d active placed actors."),
		PlacedActors.Num());
}

UPlaceableItemData* UObjectPlacementManager::FindDefinitionByPrefabID(const FName& PrefabID)
{
	for (UPlaceableItemData* Item : KnownItems)
	{
		if (Item && Item->PrefabID == PrefabID)
			return Item;
	}

	// Scan all UPlaceableItemData assets in the project
	FAssetRegistryModule& AssetRegistry =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	TArray<FAssetData> AssetList;
	AssetRegistry.Get().GetAssetsByClass(
		UPlaceableItemData::StaticClass()->GetClassPathName(), AssetList);

	for (const FAssetData& AssetData : AssetList)
	{
		UPlaceableItemData* Item = Cast<UPlaceableItemData>(AssetData.GetAsset());
		if (Item && Item->PrefabID == PrefabID)
		{
			// Cache it so next lookup is instant
			KnownItems.AddUnique(Item);
			return Item;
		}
	}

	return nullptr;
}

void UObjectPlacementManager::LoadPlacedObjects(USaveGameData* SaveData)
{
	UE_LOG(LogTemp, Warning, TEXT("[ObjectPlacer] LoadPlacedObjects called. Entries: %d"),
		SaveData ? SaveData->Entries.Num() : -1);

	if (!SaveData || !GetWorld()) return;

	if (PlacedActors.Num() > 0)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[ObjectPlacer] LoadPlacedObjects called with existing actors — clearing first."));
		ClearAllPlacedObjects();
	}

	int32 Loaded = 0;

	for (const FSaveDataEntry& Entry : SaveData->Entries)
	{
		if (Entry.Type != TEXT("FPlacedObjectSaveData")) continue;

		FPlacedObjectSaveData Data;
		if (!FJsonObjectConverter::JsonObjectStringToUStruct(Entry.JsonData, &Data)) continue;
		if (Data.PrefabID.IsNone()) continue;

		// Use asset registry fallback so KnownItems being empty at load time is not an issue
		UPlaceableItemData* Definition = FindDefinitionByPrefabID(Data.PrefabID);

		if (!Definition)
		{
			UE_LOG(LogTemp, Warning,
				TEXT("[ObjectPlacer] No DataAsset found for PrefabID '%s'. Skipped."),
				*Data.PrefabID.ToString());
			continue;
		}

		UClass* ActorClass = Definition->ActorClass.LoadSynchronous();
		if (!ActorClass) continue;

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AActor* Spawned = GetWorld()->SpawnActor<AActor>(
			ActorClass, Data.Position, Data.Rotation, Params);

		if (!Spawned) continue;

		Spawned->SetActorScale3D(Data.Scale);

		// Restore the GUID so the SaveManager sweep matches it on next save
		UGUIDComponent* GUIDComp = Spawned->FindComponentByClass<UGUIDComponent>();
		if (!GUIDComp)
		{
			GUIDComp = NewObject<UGUIDComponent>(Spawned);
			GUIDComp->RegisterComponent();
		}
		GUIDComp->GUID = Entry.GUID;

		RegisterPlacedActor(Spawned, Definition);
		Loaded++;
	}

	UE_LOG(LogTemp, Log, TEXT("[ObjectPlacer] Loaded %d placed actors."), Loaded);
}

void UObjectPlacementManager::ClearAllPlacedObjects()
{
	ExitCurrentMode();

	for (int32 i = PlacedActors.Num() - 1; i >= 0; i--)
		if (PlacedActors[i]) PlacedActors[i]->Destroy();

	PlacedActors.Empty();

	UE_LOG(LogTemp, Log, TEXT("[ObjectPlacer] All placed actors cleared."));
}

// FP_Controller hard-coded methods to ensure raycasting and camera movements not affecting the system
void UObjectPlacementManager::SetBobbingEnabled(bool bEnabled)
{
	AFP_Character* Character = Cast<AFP_Character>(GetOwner());
	if (!Character) return;

	Character->SetCanBob(bEnabled);
}

UCameraComponent* UObjectPlacementManager::GetPlayerCamera() const
{
	if (!GetOwner()) return nullptr;
	return GetOwner()->FindComponentByClass<UCameraComponent>();
}