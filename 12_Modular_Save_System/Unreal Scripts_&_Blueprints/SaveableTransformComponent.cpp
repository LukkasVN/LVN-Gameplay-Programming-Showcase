#include "SaveableTransformComponent.h"
#include "JsonObjectConverter.h"

void USaveableTransformComponent::BeginPlay()
{
	Super::BeginPlay();
	DefaultTransform = GetOwner()->GetActorTransform();
}

void USaveableTransformComponent::ResetToDefault() const
{
	AActor* Owner = GetOwner();
	Owner->SetActorTransform(DefaultTransform);
}

FString USaveableTransformComponent::CaptureState()
{
	FTransformSaveData Data = {};
	AActor* Owner = GetOwner();

	if (bSavePosition){ Data.Position = Owner->GetActorLocation(); }
	if (bSaveRotation){ Data.Rotation = Owner->GetActorRotation(); }
	if (bSaveScale){ Data.Scale = Owner->GetActorScale3D(); }

	FString Json;
	FJsonObjectConverter::UStructToJsonObjectString(Data, Json);

	UE_LOG(LogTemp, Warning, TEXT("Saved Transform: %s"), *Json);
	
	return Json;
}

void USaveableTransformComponent::RestoreState(const FString& JsonData)
{
	FTransformSaveData Data = {};
	FJsonObjectConverter::JsonObjectStringToUStruct(JsonData, &Data, 0, 0);

	AActor* Owner = GetOwner();

	if (bSavePosition){ Owner->SetActorLocation(Data.Position); }
	if (bSaveRotation){ Owner->SetActorRotation(Data.Rotation); }
	if (bSaveScale){ Owner->SetActorScale3D(Data.Scale); }

	UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(Owner->GetRootComponent());
	if (Root && Root->IsSimulatingPhysics())
	{
		Root->SetSimulatePhysics(false);

		Root->SetPhysicsLinearVelocity(FVector::ZeroVector);
		Root->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);

		Root->SetSimulatePhysics(true);
	}

	UE_LOG(LogTemp, Warning, TEXT("Restoring Transform: %s"), *JsonData);
}
