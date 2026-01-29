#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SaveableComponent.generated.h"

UCLASS(Abstract, Blueprintable, ClassGroup=(Save), meta=(BlueprintSpawnableComponent))
class MECHANICS_TEST_LVN_API USaveableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	virtual FString CaptureState() PURE_VIRTUAL(USaveableComponent::CaptureState, return "";);
	virtual void RestoreState(const FString& JsonData) PURE_VIRTUAL(USaveableComponent::RestoreState, );
	virtual FString GetSaveDataType() const PURE_VIRTUAL(USaveableComponent::GetSaveDataType, return "";);
};
