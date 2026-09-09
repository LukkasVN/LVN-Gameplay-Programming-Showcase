#include "FPS_WeaponData.h"

EFPS_FireMode UFPS_WeaponData::ResolveInitialFireMode() const
{
	if (SupportsFireMode(DefaultFireMode))
	{
		return DefaultFireMode;
	}

	const EFPS_FireMode Order[] = { EFPS_FireMode::Semi, EFPS_FireMode::Burst, EFPS_FireMode::Auto };
	for (const EFPS_FireMode Mode : Order)
	{
		if (SupportsFireMode(Mode))
		{
			return Mode;
		}
	}

	return EFPS_FireMode::Semi;
}
