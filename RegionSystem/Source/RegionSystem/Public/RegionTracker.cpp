// Copyright Phoenix Dawn Development LLC. All Rights Reserved.

#include "RegionTracker.h"

#include "RegionTags.h"
#include "RegionVolume.h"
#include "GameFramework/Character.h"
#include "DebugFunctionLibrary.h"

DEFINE_LOG_CATEGORY(LogRegions)

bool URegionTracker::IsInRegion(FGameplayTag Tag) const
{
	for (auto Region : GetRegions())
	{
		if (Region.MatchesTag(Tag))
			return true;
	}
	return false;
}

TSet<FGameplayTag> URegionTracker::GetRegions() const
{
	TSet<FGameplayTag> Regions;
	for (auto RegionRef : RegionRefs)
	{
		Regions.Add(RegionRef->GetRegionTag());
	}
	return Regions;
}

FGameplayTag URegionTracker::GetRelevantRegion() const
{
	if (CachedRegionTag.IsValid())
		return CachedRegionTag;

	CachedRegionTag = CalculateRelevantRegion();
	return CachedRegionTag;
}

TSet<ARegionVolume*> URegionTracker::GetRegionRefs() const
{
	return RegionRefs;
}

void URegionTracker::AddRegion(ARegionVolume* Volume)
{
	const FGameplayTag RegionTag = Volume->GetRegionTag();
	bool bShouldTrigger = !IsInRegion(RegionTag);
	
	RegionRefs.Add(Volume);

	if (bShouldTrigger)
	{
		OnRegionEnter.Broadcast(RegionTag);
		DEBUG_SIMPLE(LogRegions, Log, FColor::Green, FString::Printf(TEXT("Entered %s"), *RegionTag.GetTagName().ToString()), RegionTags::Name);
	}
	else
	{
		DEBUG_SIMPLE(LogRegions, Log, FColor::White, FString::Printf(TEXT("Quietly Entered %s"), *RegionTag.GetTagName().ToString()), RegionTags::Name);
	}
}

void URegionTracker::RemoveRegion(ARegionVolume* Volume)
{
	RegionRefs.Remove(Volume);
	
	const FGameplayTag RegionTag = Volume->GetRegionTag();
	if (!IsInRegion(RegionTag))
	{
		OnRegionExit.Broadcast(RegionTag);
		DEBUG_SIMPLE(LogRegions, Log, FColor::Red, FString::Printf(TEXT("Exited %s"), *RegionTag.GetTagName().ToString()), RegionTags::Name);
	}
	else
	{
		DEBUG_SIMPLE(LogRegions, Log, FColor::White, FString::Printf(TEXT("Quietly Exited %s"), *RegionTag.GetTagName().ToString()), RegionTags::Name);
	}
}

FGameplayTag URegionTracker::CalculateRelevantRegion() const
{
	TSet<FGameplayTag> ContainingRegions = GetRegions();

	FGameplayTag MostDetailedTag;
	int32 MaxDetailLevel = 0;

	for (const FGameplayTag& Tag : ContainingRegions)
	{
		TArray<FString> Components;
		Tag.ToString().ParseIntoArray(Components, TEXT("."));
		
		int32 DetailLevel = Components.Num();
		
		if (DetailLevel > MaxDetailLevel)
		{
			MaxDetailLevel = DetailLevel;
			MostDetailedTag = Tag;
		}
	}
	
	return MostDetailedTag;
}
