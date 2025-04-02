#pragma once

#include "NativeGameplayTags.h"

namespace RegionTags
{
	REGIONSYSTEM_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Name);

	namespace Areas
	{
		REGIONSYSTEM_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Name);
	}

	namespace Modules
	{
		REGIONSYSTEM_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Name);

		namespace Electricity
		{
			REGIONSYSTEM_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Name);
			namespace ConsumerTypes
			{
				REGIONSYSTEM_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Name);
			}
		}
	}

	namespace Messages
	{
		REGIONSYSTEM_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Name);
		REGIONSYSTEM_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Refresh);
	}
};
