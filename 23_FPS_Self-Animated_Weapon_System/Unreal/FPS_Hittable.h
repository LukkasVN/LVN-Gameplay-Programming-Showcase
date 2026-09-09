#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FPS_Hittable.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UFPS_Hittable : public UInterface
{
	GENERATED_BODY()
};

class MECHANICS_TEST_LVN_API IFPS_Hittable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FPS|Hittable")
	void TakeHit(float Damage, FVector HitPoint, FVector HitDirection);
};
