#pragma once

#include "CoreMinimal.h"
#include "EPlacementSurfaceType.generated.h"

// Surface types
UENUM(BlueprintType, Meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EPlacementSurfaceType : uint8
{
	None     = 0          UMETA(Hidden),
	Floor    = 1 << 0     UMETA(DisplayName = "Floor"),
	Wall     = 1 << 1     UMETA(DisplayName = "Wall"),
	Ceiling  = 1 << 2     UMETA(DisplayName = "Ceiling"),
};

ENUM_CLASS_FLAGS(EPlacementSurfaceType)