// GUIDComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Misc/Guid.h"
#include "GUIDComponent.generated.h"

UCLASS(ClassGroup=(Save), meta=(BlueprintSpawnableComponent))
class MECHANICS_TEST_LVN_API UGUIDComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGuid GUID;

	UGUIDComponent();

protected:
	virtual void OnRegister() override;
};

