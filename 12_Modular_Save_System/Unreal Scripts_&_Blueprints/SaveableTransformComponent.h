#pragma once

#include "CoreMinimal.h"
#include "SaveableComponent.h"
#include "SaveableTransformComponent.generated.h"

USTRUCT()
struct FTransformSaveData
{
	GENERATED_BODY()

	UPROPERTY() FVector Position;
	UPROPERTY() FRotator Rotation;
	UPROPERTY() FVector Scale;
};

UCLASS(ClassGroup=(Save), meta=(BlueprintSpawnableComponent))
class MECHANICS_TEST_LVN_API USaveableTransformComponent : public USaveableComponent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Saveable Transform")
	bool bSavePosition = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Saveable Transform")
	bool bSaveRotation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Saveable Transform")
	bool bSaveScale = false;

	
	virtual FString CaptureState() override;
	virtual void RestoreState(const FString& JsonData) override;
	virtual FString GetSaveDataType() const override { return "FTransformSaveData"; }

	// Custom method to reset the transform to it's initial values
	void ResetToDefault() const;

private:
	FTransform DefaultTransform;
};
