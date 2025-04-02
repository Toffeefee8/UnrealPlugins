#include "RegionTags.h"

#include "Engine/EngineTypes.h"

namespace RegionTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Name, "Regions", "All Region related tags should be defined here.");

	namespace Areas
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Name, "Regions.Areas", "All Regions should be defined here.");
	}

	namespace Modules
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Name, "Regions.Modules", "All Regions Module related tags should be defined here.");

		namespace Electricity
		{
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Name, "Regions.Modules.Electricity", "All Electricity Region Module related tags should be defined here.");

			namespace ConsumerTypes
			{
				UE_DEFINE_GAMEPLAY_TAG_COMMENT(Name, "Regions.Modules.Electricity.ConsumerTypes", "All Electricity Consumer Types tags should be defined here. (e.g. Lights, Security, Life Support etc.)");
			}
		}
	}

	namespace Messages
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Name, "Regions.Messages", "Parent Category for region Messages.");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Refresh, "Regions.Messages.Refresh", "Region Refresh Message.");
	}
}

