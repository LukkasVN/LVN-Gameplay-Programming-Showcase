#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HitWeaponData.generated.h"

UCLASS(BlueprintType)
class MECHANICS_TEST_LVN_API UHitWeaponData : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Identity")
	FString WeaponName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stats")
	int32 Damage = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stats")
	int32 Tier = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stats")
	float HitCooldown = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stats")
	float HitRange = 200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Feedback")
	bool bUseHitShake = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Feedback")
	TSubclassOf<UCameraShakeBase> HitShakeClass;
};
