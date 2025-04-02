#include "RegionFunctionLibrary.h"

#include "RegionTags.h"
#include "Extensions/GameplayTagExtensions.h"

ERegionTypes URegionFunctionLibrary::GetRegionTypeByTag(FGameplayTag RegionTag)
{
	switch (GetRegionDepthByTag(RegionTag))
	{
	case 0:
		return ERegionTypes::Master;
	case 1:
		return ERegionTypes::Section;
	case 2:
		return ERegionTypes::Subsection;
	default:
		return ERegionTypes::Room;
	}
}

int32 URegionFunctionLibrary::GetRegionDepthByTag(FGameplayTag RegionTag)
{
	const int32 TagDepth = UGameplayTagExtensions::GetTagDepth(RegionTag);
	const int32 BaseDepth = UGameplayTagExtensions::GetTagDepth(RegionTags::Areas::Name);
	return FMath::Max(0, TagDepth - BaseDepth);
}