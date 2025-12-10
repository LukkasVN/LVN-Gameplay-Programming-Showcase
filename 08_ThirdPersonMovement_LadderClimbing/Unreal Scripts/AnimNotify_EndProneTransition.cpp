// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_EndProneTransition.h"
#include "PlayerCharacter.h"

void UAnimNotify_EndProneTransition::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (APlayerCharacter* Player = Cast<APlayerCharacter>(MeshComp->GetOwner()))
	{
		Player->EndProneTransition();
	}
}

