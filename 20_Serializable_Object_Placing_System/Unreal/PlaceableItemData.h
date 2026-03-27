#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EPlacementSurfaceType.h"
#include "PlaceableItemData.generated.h"

// It is mandatory to have a Data Asset per "Placeable" due to it containing the spawnable actor, and it's placement characteristics.
UCLASS(BlueprintType)
class MECHANICS_TEST_LVN_API UPlaceableItemData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	// Static naming for prefab lookup. Should be unique for each added prefab.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FName PrefabID;

	// Display name shown in the UI. Does not have to be unique.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
	FText DisplayName;

	// Optional MenuIcon
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
	TSoftObjectPtr<UTexture2D> MenuIcon;


	// Actor that will be spawned/loaded on demand.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Actor")
	TSoftClassPtr<AActor> ActorClass;

	// Allowed surfaces to be placed
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement",
		Meta = (Bitmask, BitmaskEnum = "/Script/MECHANICS_TEST_LVN.EPlacementSurfaceType")) //You must make sure that the path (In my case "MECHANICS_TEST_LVN") has your project's name.
	int32 AllowedSurfaces = 0b111; // Floor | Wall | Ceiling (Everything) by default

	UFUNCTION(BlueprintPure, Category = "Placement")
	bool IsSurfaceAllowed(EPlacementSurfaceType SurfaceType) const
	{
		return (AllowedSurfaces & static_cast<int32>(SurfaceType)) != 0;
	}


	// Rotation degrees applied on "Rotation key" pressed
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotation",
		Meta = (ClampMin = "1.0", ClampMax = "90.0"))
	float RotationStep = 45.f;

	// If true the object will rotate to be adapted to the surface normal on placement, instead of keeping its original rotation.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotation")
	bool bSnapRotationToSurface = false;
};