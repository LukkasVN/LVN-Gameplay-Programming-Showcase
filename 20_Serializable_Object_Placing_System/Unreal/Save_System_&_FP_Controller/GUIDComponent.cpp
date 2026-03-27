#include "GUIDComponent.h"

UGUIDComponent::UGUIDComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UGUIDComponent::OnRegister()
{
	Super::OnRegister();

	if (!GUID.IsValid())
	{
		GUID = FGuid::NewGuid();
	}
}
