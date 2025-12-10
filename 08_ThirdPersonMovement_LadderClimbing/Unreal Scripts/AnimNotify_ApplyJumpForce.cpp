// AnimNotify_ApplyJumpForce.cpp
#include "AnimNotify_ApplyJumpForce.h"
#include "PlayerCharacter.h"

void UAnimNotify_ApplyJumpForce::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (APlayerCharacter* Player = Cast<APlayerCharacter>(MeshComp->GetOwner()))
	{
		Player->ApplyJumpForce();
	}
}

