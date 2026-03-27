#pragma once

#include "CoreMinimal.h"
#include "PlacedObjectSaveData.generated.h"

// Necessary struct for data saving/loading on runtime
USTRUCT()
struct FPlacedObjectSaveData
{
	GENERATED_BODY()

	// Matches UPlaceableItemData::PrefabID.
	// Used to find the right DataAsset on load.
	UPROPERTY() FName PrefabID;

	UPROPERTY() FVector  Position = FVector::ZeroVector;
	UPROPERTY() FRotator Rotation = FRotator::ZeroRotator;
	UPROPERTY() FVector  Scale    = FVector::OneVector;
};