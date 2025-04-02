#include "RegionSystem.h"

#define LOCTEXT_NAMESPACE "FRegionSystemModule"

DEFINE_LOG_CATEGORY(LogRegions)

void FRegionSystemModule::StartupModule()
{
}

void FRegionSystemModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FRegionSystemModule, RegionSystem)