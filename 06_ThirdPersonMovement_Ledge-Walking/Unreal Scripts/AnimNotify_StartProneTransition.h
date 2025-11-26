#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_StartProneTransition.generated.h"

/**
 * 
 */
UCLASS()
class MECHANICS_TEST_LVN_API UAnimNotify_StartProneTransition : public UAnimNotify
{
	GENERATED_BODY()
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};
