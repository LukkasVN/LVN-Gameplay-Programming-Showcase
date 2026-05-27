#pragma once

#include "CoreMinimal.h"
#include "DestructibleData.h"
#include "HitWeaponData.h"
#include "IDestructible.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UDestructible : public UInterface { GENERATED_BODY() };

class MECHANICS_TEST_LVN_API IDestructible
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Destructible")
	bool TakeHit(UHitWeaponData* Weapon, FVector HitLocation, FVector HitDirection);
	// Returns true if the hit was actually applied (passed tier check etc.), so the attacker can play feedback only on successful hits if it wants.

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Destructible")
	void ForceDestroy();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Destructible")
	UDestructibleData* GetDestructibleData() const;
};
