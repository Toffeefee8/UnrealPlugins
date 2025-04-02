#include "Region.h"

#include "RegionFunctionLibrary.h"
#include "RegionTracker.h"
#include "Extensions/GameplayTagExtensions.h"
#include "Modules/RegionModule.h"

void URegion::StartRegion()
{
	RefreshRegion();

	for (auto RegionModule : RegionModules)
	{
		RegionModule->StartModule(this);
	}
}

void URegion::RefreshRegion()
{
	if (URegionSubsystem* RegionSubsystem = URegionSubsystem::Get(this))
	{
		for (auto Volume : Volumes)
		{
			TArray<UObject*> InterfaceObjects = Volume.Key->GetOverlappingInterfaceObjects<URegionObject>();
			for (auto Object : InterfaceObjects)
			{
				RegionSubsystem->ReevaluateRegionObject(Object);
			}
		}
	}

	for (auto RegionModule : RegionModules)
	{
		RegionModule->RefreshModule();
	}
}

void URegion::EndRegion()
{
	for (auto RegionModule : RegionModules)
	{
		RegionModule->EndModule();
	}

	Volumes.Empty();
	RegionModules.Empty();
}

void URegion::OnNewChild(URegion* NewChild)
{
	for (auto RegionModule : RegionModules)
	{
		RegionModule->NewChild(NewChild);
	}
}

void URegion::OnNewParent(URegion* OldParent, URegion* NewParent)
{
	for (auto RegionModule : RegionModules)
	{
		RegionModule->NewParent(OldParent, NewParent);
	}
}

FGameplayTag URegion::GetRegionTag() const
{
	return RegionTag;
}

FGameplayTag URegion::GetRegionTagOfType(ERegionTypes RegionType) const
{
	if (const URegion* Region = GetRegionOfType(RegionType))
	{
		return Region->GetRegionTag();
	}
	return FGameplayTag();
}

int8 URegion::GetRegionDepth() const
{
	return UGameplayTagExtensions::CreatePartsFromTag(RegionTag).Num();
}

ERegionTypes URegion::GetRegionType() const
{
	return URegionFunctionLibrary::GetRegionTypeByTag(GetRegionTag());
}

bool URegion::IsParentRegion(FGameplayTag InRegionTag) const
{
	return InRegionTag.MatchesTag(RegionTag);
}

bool URegion::Contains(FVector Location) const
{
	for (auto VolumePair : Volumes)
	{
		check(VolumePair.Key);
		if (VolumePair.Key->Contains(Location))
			return true;
	}
	return false;
}

bool URegion::ContainsFully(const FVector Location, const FVector BoxExtents) const
{
	//TODO: Returns false when box is divided up on multiple volumes which needs to be fixed
	for (auto VolumePair : Volumes)
	{
		check(VolumePair.Key);
		if (VolumePair.Key->ContainsFully(Location, BoxExtents))
			return true;
	}
	return false;
}

bool URegion::IsChildRegion(FGameplayTag InRegionTag) const
{
	return RegionTag.MatchesTag(InRegionTag);
}

bool URegion::IsRegionExact(FGameplayTag InRegionTag) const
{
	return RegionTag.MatchesTagExact(InRegionTag);
}

FRegionPOIData URegion::GetRandomPOI() const
{
	TArray<FRegionPOIData> POIData = GetAllPOIs();
	int32 RandomNum = FMath::RandRange(0, POIData.Num() - 1);
	if (POIData.IsValidIndex(RandomNum))
		return POIData[RandomNum];
	
	return FRegionPOIData();
}

FRegionPOIData URegion::GetClosestPOI(FVector Vector) const
{
	TArray<FRegionPOIData> POIData = GetAllPOIs();
	float ClosestDist = FLT_MAX;
	FRegionPOIData Data;
	for (auto POI : POIData)
	{
		float CurrentDist = FVector::Distance(Vector, POI.Location);
		if (CurrentDist < ClosestDist)
		{
			ClosestDist = CurrentDist;
			Data = POI;
		}
	}
	return Data;
}

TArray<ARegionVolume*> URegion::GetRegionVolumes() const
{
	TArray<TObjectPtr<ARegionVolume>> OutVolumes {};
	Volumes.GetKeys(OutVolumes);
	return OutVolumes;
}

const URegion* URegion::GetRegionOfType(ERegionTypes RegionType, bool AllowWrongIfNotFound) const
{
	const URegion* CheckRegion = this;

	while (CheckRegion)
	{
		if (CheckRegion->GetRegionType() == RegionType)
			return CheckRegion;
		
		switch (CheckRegion->GetRegionType()) {
		case ERegionTypes::Master:
			break;
		case ERegionTypes::Section:
			if (CheckRegion->GetRegionType() == ERegionTypes::Master)
				return AllowWrongIfNotFound ? CheckRegion : nullptr;
			break;
		case ERegionTypes::Subsection:
			if (CheckRegion->GetRegionType() == ERegionTypes::Section ||
				CheckRegion->GetRegionType() == ERegionTypes::Master)
				return AllowWrongIfNotFound ? CheckRegion : nullptr;
			break;
		case ERegionTypes::Room:
			if (CheckRegion->GetRegionType() == ERegionTypes::Section ||
				CheckRegion->GetRegionType() == ERegionTypes::Subsection ||
				CheckRegion->GetRegionType() == ERegionTypes::Master)
				return AllowWrongIfNotFound ? CheckRegion : nullptr;
		}
	}

	return AllowWrongIfNotFound ? this : nullptr;
}

URegion* URegion::GetParentRegion() const
{
	URegionSubsystem* RegionSubsystem = URegionSubsystem::Get(this);
	if (!RegionSubsystem)
		return nullptr;
	
	FGameplayTag ParentTag = UGameplayTagExtensions::RemoveTagDepth(RegionTag);
	return RegionSubsystem->GetRegionByTag(ParentTag);
}

FGameplayTag URegion::GetParentRegionTag() const
{
	if (URegion* ParentRegion = GetParentRegion())
	{
		return ParentRegion->GetRegionTag();
	}
	return FGameplayTag();
}

TSet<URegion*> URegion::GetChildRegions() const
{
	URegionSubsystem* RegionSubsystem = URegionSubsystem::Get(this);
	if (!RegionSubsystem)
		return {};

	TSet<URegion*> ChildRegions {};
	for (auto Region : RegionSubsystem->GetAllRegions())
	{
		if (!Region || Region == this)
			continue;
		
		if (Region->GetRegionTag().MatchesTag(GetRegionTag()))
		{
			ChildRegions.Add(Region);
		}
	}
	return ChildRegions;
}

FGameplayTagContainer URegion::GetChildRegionTags() const
{
	FGameplayTagContainer ChildRegionTags {};
	for (auto ChildRegion : GetChildRegions())
	{
		if (!ChildRegion)
			continue;

		ChildRegionTags.AddTag(ChildRegion->GetRegionTag());
	}
	return ChildRegionTags;
}

URegionModule* URegion::GetRegionModule(TSubclassOf<URegionModule> ModuleClass, bool AllowParentSearch) const
{
	const URegion* RegionToSearch = this;
	do
	{
		for (auto RegionModule : RegionModules)
		{
			if (RegionModule.IsA(ModuleClass))
			{
				return RegionModule;
			}
		}
		RegionToSearch = RegionToSearch->GetParentRegion();
	}
	while (RegionToSearch && AllowParentSearch);
	
	return nullptr;
}

bool URegion::TryEnterRegion(URegionTracker* Tracker, const ARegionVolume* Volume)
{
	FContainedTrackers* FoundTrackers = Volumes.Find(Volume);
	if (!FoundTrackers)
	{
		UE_LOG(LogTemp, Error, TEXT("Volume not found! Not registered Correctly!"))
		return false;
	}

	if (FoundTrackers->ContainedTrackers.Contains(Tracker))
	{
		UE_LOG(LogTemp, Warning, TEXT("Volume already contains tracker!"))
		return true;
	}

	bool bContainedByOthers = false;
	for (auto Pair : Volumes)
	{
		if (Pair.Value.ContainedTrackers.Contains(Tracker))
		{
			bContainedByOthers = true;
			break;
		}
	}

	FoundTrackers->ContainedTrackers.Add(Tracker);

	//Only fire enter region if not contained previously
	if (!bContainedByOthers)
	{
		OnEnterRegion(Tracker);
	}
	return true;
}

bool URegion::TryExitRegion(URegionTracker* Tracker, const ARegionVolume* Volume)
{
	FContainedTrackers* FoundTrackers = Volumes.Find(Volume);
	if (!FoundTrackers)
	{
		UE_LOG(LogTemp, Error, TEXT("Volume not found! Not registered Correctly!"))
		return false;
	}

	if (!FoundTrackers->ContainedTrackers.Contains(Tracker))
	{
		UE_LOG(LogTemp, Warning, TEXT("Volume does not contain tracker!"))
		return true;
	}

	FoundTrackers->ContainedTrackers.Remove(Tracker);

	bool bContainedByOthers = false;
	for (auto Pair : Volumes)
	{
		if (Pair.Value.ContainedTrackers.Contains(Tracker))
		{
			bContainedByOthers = true;
			break;
		}
	}

	//Only fire enter region if not contained previously
	if (!bContainedByOthers)
	{
		OnExitRegion(Tracker);
	}
	return true;
}

void URegion::OnEnterRegion(URegionTracker* Tracker)
{
	Tracker->AddRegion(this);
}

void URegion::OnExitRegion(URegionTracker* Tracker)
{
	Tracker->RemoveRegion(this);
}

void URegion::AddVolume(ARegionVolume* Volume)
{
	if (Volumes.Contains(Volume))
		return;

	Volumes.Add(Volume);
}

void URegion::RemoveVolume(ARegionVolume* Volume)
{
	if (!Volumes.Contains(Volume))
		return;

	//Handle objects in volume here

	Volumes.Remove(Volume);
}

TArray<FRegionPOIData> URegion::GetAllPOIs() const
{
	TArray<FRegionPOIData> OutPOIData {};
	for (auto Volume : Volumes)
	{
		OutPOIData.Append(Volume.Key->GetPOIData());
	}
	return OutPOIData;
}
