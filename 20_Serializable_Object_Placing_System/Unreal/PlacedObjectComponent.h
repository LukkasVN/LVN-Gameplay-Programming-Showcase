#pragma once

#include "CoreMinimal.h"
#include "Mechanics_Test_LVN/SaveableComponent.h" // Include for the SaveableComponent made in a previous section
#include "PlaceableItemData.h"
#include "PlacedObjectComponent.generated.h"

// This Component has to be attached along GUIDComponent (Part of the Save System) in the "Placeable" Actor
UCLASS(ClassGroup = "Placement", Meta = (BlueprintSpawnableComponent))
class MECHANICS_TEST_LVN_API UPlacedObjectComponent : public USaveableComponent
{
	GENERATED_BODY()

public:

	UPlacedObjectComponent();

	UPROPERTY(BlueprintReadOnly, Category = "Placement")
	TObjectPtr<UPlaceableItemData> Definition;

	UFUNCTION(BlueprintCallable, Category = "Placement")
	void Initialize(UPlaceableItemData* InDefinition);

	UFUNCTION(BlueprintCallable, Category = "Placement")
	void ApplyOverrideMaterial(UMaterialInterface* OverrideMaterial);

	UFUNCTION(BlueprintCallable, Category = "Placement")
	void RestoreMaterials();

	UFUNCTION(BlueprintCallable, Category = "Placement")
	void SnapshotForEdit();

	UFUNCTION(BlueprintCallable, Category = "Placement")
	void RevertToSnapshot();

	UFUNCTION(BlueprintCallable, Category = "Placement")
	void MarkForRemoval(bool bMark);

	UFUNCTION(BlueprintPure, Category = "Placement")
	bool IsMarkedForRemoval() const { return bMarkedForRemoval; }

	virtual FString CaptureState() override;
	virtual void    RestoreState(const FString& JsonData) override;
	virtual FString GetSaveDataType() const override { return TEXT("FPlacedObjectSaveData"); }

private:

	TMap<TObjectPtr<UMeshComponent>, TArray<TObjectPtr<UMaterialInterface>>> OriginalMaterials;

	FVector SnapshotPosition;
	FQuat   SnapshotRotation;

	bool bMarkedForRemoval = false;

	void CacheMaterials();
};