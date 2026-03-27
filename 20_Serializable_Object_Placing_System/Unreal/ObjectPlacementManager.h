#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlaceableItemData.h"
#include "PlacedObjectComponent.h"
#include "Camera/CameraComponent.h"
#include "ObjectPlacementManager.generated.h"

UENUM(BlueprintType)
enum class EPlacementMode : uint8
{
	None      UMETA(DisplayName = "None"),
	Placing   UMETA(DisplayName = "Placing"),
	Editing   UMETA(DisplayName = "Editing"),
	Removing  UMETA(DisplayName = "Removing"),
};

UENUM(BlueprintType)
enum class EEditPhase : uint8
{
	SelectingObject      UMETA(DisplayName = "Selecting Object"),
	RepositioningObject  UMETA(DisplayName = "Repositioning Object"),
};

UCLASS(ClassGroup = "Placement", Meta = (BlueprintSpawnableComponent))
class MECHANICS_TEST_LVN_API UObjectPlacementManager : public UActorComponent
{
	GENERATED_BODY()

public:

	UObjectPlacementManager();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	// Tag required for a surface to be considered valid for placement. Should be added to the trace channel's collision presets.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement|Settings")
	FName RequiredSurfaceTag = FName("PlaceableSurface");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement|Settings")
	float PlacementDistance = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement|Settings")
	TEnumAsByte<ECollisionChannel> PlacementChannel = ECC_WorldStatic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement|Settings")
	TEnumAsByte<ECollisionChannel> PlacedObjectChannel = ECC_GameTraceChannel1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement|Materials")
	TObjectPtr<UMaterialInterface> HoverValidMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement|Materials")
	TObjectPtr<UMaterialInterface> HoverEditMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement|Materials")
	TObjectPtr<UMaterialInterface> HoverInvalidMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement|Registry")
	TArray<TObjectPtr<UPlaceableItemData>> KnownItems;

	UFUNCTION(BlueprintCallable, Category = "Placement")
	void EnterPlacingMode(UPlaceableItemData* Item);

	UFUNCTION(BlueprintCallable, Category = "Placement")
	void EnterEditMode();

	UFUNCTION(BlueprintCallable, Category = "Placement")
	void EnterRemoveMode();

	UFUNCTION(BlueprintCallable, Category = "Placement")
	void ExitCurrentMode();

	UFUNCTION(BlueprintCallable, Category = "Placement|Input")
	void OnConfirmInput();

	UFUNCTION(BlueprintCallable, Category = "Placement|Input")
	void OnCancelInput();

	UFUNCTION(BlueprintCallable, Category = "Placement|Input")
	void OnRotateInput();

	UFUNCTION(BlueprintPure, Category = "Placement")
	EPlacementMode GetCurrentMode() const { return CurrentMode; }

	UFUNCTION(BlueprintPure, Category = "Placement")
	bool IsPlacementActive() const { return CurrentMode != EPlacementMode::None; }

	UFUNCTION(BlueprintCallable, Category = "Placement|Save")
	void PrepareForSave();

	UFUNCTION(BlueprintCallable, Category = "Placement|Save")
	void LoadPlacedObjects(class USaveGameData* SaveData);

	UFUNCTION(BlueprintCallable, Category = "Placement|Save")
	void ClearAllPlacedObjects();

private:

	EPlacementMode CurrentMode  = EPlacementMode::None;
	EEditPhase     EditPhase    = EEditPhase::SelectingObject;

	TObjectPtr<UPlaceableItemData> SelectedItem;
	TObjectPtr<AActor>             PreviewActor;
	TArray<TObjectPtr<UMeshComponent>> PreviewMeshes;

	bool  bLastValidState = false;
	float RotationOffset  = 0.f;

	TObjectPtr<AActor>         TargetedActor;
	TArray<TObjectPtr<AActor>> PlacedActors;

	void SetBobbingEnabled(bool bEnabled);

	void TickPlacing();
	void TickEditing();
	void TickRemoving();
	void TickEditSelection();
	void TickEditRepositioning();

	bool EvaluatePlacement(UPlaceableItemData* Item,
		FHitResult& OutHit, EPlacementSurfaceType& OutSurfaceType);
	EPlacementSurfaceType ClassifySurface(const FVector& Normal) const;
	bool IsPreviewOverlapping() const;

	void SpawnPreview(UPlaceableItemData* Item);
	void DestroyPreview();
	void PositionAndRotatePreview(bool bValidHit, const FHitResult& Hit, UPlaceableItemData* Item);
	void SetPreviewMaterial(UMaterialInterface* Mat);
	void UpdatePreviewMaterial(UMaterialInterface* Mat);

	AActor* LineTraceForPlacedActor() const;
	void    RegisterPlacedActor(AActor* Actor, UPlaceableItemData* Item);
	void    ClearTargetedActor();

	void CommitPlacement();
	void BeginRepositioning(AActor* Actor);
	void CommitEdit();
	void CancelEdit();
	void StageForRemoval(AActor* Actor);

	UPlaceableItemData* FindDefinitionByPrefabID(const FName& PrefabID);

	UCameraComponent* GetPlayerCamera() const;
};