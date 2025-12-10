// AnimNotify_TriggerFlip.cpp
#include "AnimNotify_TriggerFlip.h"
#include "PlayerCharacter.h"

void UAnimNotify_TriggerFlip::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (APlayerCharacter* Player = Cast<APlayerCharacter>(MeshComp->GetOwner()))
	{
		Player->TriggerFlip();
	}
}

