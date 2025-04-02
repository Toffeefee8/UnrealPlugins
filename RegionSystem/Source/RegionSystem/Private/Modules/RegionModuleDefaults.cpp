#include "Modules/RegionModuleDefaults.h"

#include "Modules/RegionModule.h"

bool FRegionModuleDefaults::Contains(const TSubclassOf<URegionModule>& Class) const
{
	for (auto Module : RegionModules)
	{
		if (Module.IsA(Class))
		{
			return true;
		}
	}
	return false;
}

URegionModule* FRegionModuleDefaults::GetRegionModule(const TSubclassOf<URegionModule>& Class) const
{
	for (auto Module : RegionModules)
	{
		if (Module.IsA(Class))
		{
			return Module;
		}
	}
	return nullptr;
}
