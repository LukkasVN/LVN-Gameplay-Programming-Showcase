#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AssetGridVisualizer.generated.h"

USTRUCT(BlueprintType)
struct FVisualAssetData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Asset", meta = (AllowedClasses = "StaticMesh"))
	UObject* PrefabReference;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Asset", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float ScaleMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Asset")
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Asset")
	FVector PositionOffset = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct FAssetRow
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	TArray<FVisualAssetData> Assets;
};

USTRUCT(BlueprintType)
struct FAssetGrid
{
	GENERATED_BODY()

	// Horizontal Separation between 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	float HorizontalSeparation = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	float VerticalSeparation = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	TArray<FAssetRow> Rows;

	UPROPERTY(VisibleAnywhere, Category = "Grid")
	TArray<AActor*> InstantiatedAssets;

	bool HasConfigChanged() const;
};

UCLASS()
class MECHANICS_TEST_LVN_API AAssetGridVisualizer : public AActor
{
	GENERATED_BODY()

public:
	AAssetGridVisualizer();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	bool bEnabledInEditor = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	float GridSeparation = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	TArray<FAssetGrid> Grids;

protected:
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;

#if WITH_EDITOR
	virtual void Destroyed() override;
#endif

public:
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Grid")
	void RefreshLayout();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Grid")
	void ResetScript();

	UFUNCTION(BlueprintCallable, Category = "Grid")
	float GetGridWidth(int32 GridIndex) const;

private:
	void RecalculateLayout();
	void CleanupGridContainers();
	void InstantiateGrid(int32 GridIndex, FVector ActorWorldPosition, float YOffset);
	FVector GetAssetBounds(AActor* Asset) const;
	FVector GetPrefabBounds(UObject* PrefabReference) const;
	AActor* SpawnFromReference(UObject* Reference, const FVector& Location, const FRotator& Rotation);
};