#include "Settings/RegionSettings.h"

const URegionSettings* URegionSettings::Get()
{
	return GetDefault<URegionSettings>();
}

bool URegionSettings::GetDefaultModuleFuzeState()
{
	if (const URegionSettings* Defaults = Get())
		return Defaults->bDefaultModuleFuzeState;
	return true;
}

bool URegionSettings::IsTypeEnabledByDefault(EElectricityConsumerType ElectricityConsumer)
{
	if (const URegionSettings* Defaults = Get())
	{
		const bool* FoundState = Defaults->TypeDefaults.Find(ElectricityConsumer);
		if (FoundState != nullptr)
			return *FoundState;

		return Defaults->bDefaultConsumerTypeState;
	}
	return false;
}
