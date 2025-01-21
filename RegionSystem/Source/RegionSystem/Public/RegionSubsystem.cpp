// Copyright Phoenix Dawn Development LLC. All Rights Reserved.


#include "RegionSubsystem.h"
#include "EngineUtils.h"
#include "NavigationSystem.h"
#include "RegionVolume.h"
#include "Components/BoxComponent.h"

void URegionSubsystem::BuildCache() const
{
	for (auto Volume : CachedRegionVolumes)
	{
		Volume->bRegistered = false;
	}
	
	CachedRegionVolumes.Empty();
	
	for (TActorIterator<ARegionVolume> It(GetWorld()); It; ++It)
	{
		if (ARegionVolume* Volume = *It)
		{
			CachedRegionVolumes.Add(Volume);
			Volume->bRegistered = true;
		}
	}
}

void URegionSubsystem::ForceResetCache() const
{
	CachedRegionVolumes.Empty();
}

FVector URegionSubsystem::ProjectPointToNavigationInRegion(const FGameplayTag RegionTag)
{
	TArray<ARegionVolume*>  Volumes =  GetVolumesByTag(RegionTag).Array();

	if (Volumes.Num()<=0)
	{
		UE_LOG(LogTemp, Error, TEXT("Region Not Found"))
		return FVector::ZeroVector;
	}

	//TODO: handle edges case where multiply regions have the same tag
	const ARegionVolume* FoundVolume = Volumes[0];
	if (!IsValid(FoundVolume))
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid Volume found"))
		return FVector::ZeroVector;
	}
	
	FVector BoxExtent =  FoundVolume->GetBoxExtent();
	FVector RegionCenter = FoundVolume->GetCenterPoint();
	UNavigationSystemV1* NavSys = 	UNavigationSystemV1::GetCurrent(this);
	if (!IsValid(NavSys))
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid NavigationSystem"))
		return FVector::ZeroVector;
	}

	FNavLocation NavLocation;
	if (!NavSys->ProjectPointToNavigation(RegionCenter, NavLocation, BoxExtent, nullptr))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to project point to navigation"))
		return FVector::ZeroVector;
	}
	
	return NavLocation.Location;
}

FVector URegionSubsystem::GetRegionCenterPoint(FGameplayTag RegionTag)
{
	TArray<ARegionVolume*>  Volumes =  GetVolumesByTag(RegionTag).Array();

	if (Volumes.Num()<=0)
	{
		UE_LOG(LogTemp, Error, TEXT("Region Not Found"))
		return FVector::ZeroVector;
	}

	//TODO: handle edges case where multiply regions have the same tag
	const ARegionVolume* FoundVolume = Volumes[0];

	return FoundVolume->GetCenterPoint();
}

FVector URegionSubsystem::GetRegionExtents(FGameplayTag RegionTag)
{
	TArray<ARegionVolume*>  Volumes =  GetVolumesByTag(RegionTag).Array();

	if (Volumes.Num()<=0)
	{
		UE_LOG(LogTemp, Error, TEXT("Region Not Found"))
		return FVector::ZeroVector;
	}

	//TODO: handle edges case where multiply regions have the same tag
	const ARegionVolume* FoundVolume = Volumes[0];

	return FoundVolume->GetBoxExtent();
}

FTransform URegionSubsystem::GetRegionTransform(FGameplayTag RegionTag)
{
	TArray<ARegionVolume*> Volumes = GetVolumesByTag(RegionTag).Array();

	if (Volumes.Num()<=0)
	{
		UE_LOG(LogTemp, Error, TEXT("Region Not Found"))
		return FTransform();
	}

	//TODO: handle edges case where multiply regions have the same tag
	const ARegionVolume* FoundVolume = Volumes[0];

	return FoundVolume->GetTransform();
}

void URegionSubsystem::RegisterVolume(ARegionVolume* Volume)
{
	if (CachedRegionVolumes.Contains(Volume))
		return;

	CachedRegionVolumes.Add(Volume);
	Volume->bRegistered = true;
}

void URegionSubsystem::DeregisterVolume(ARegionVolume* Volume)
{
	if (!CachedRegionVolumes.Contains(Volume))
		return;

	CachedRegionVolumes.Remove(Volume);
	Volume->bRegistered = false;
}

URegionSubsystem* URegionSubsystem::Get(const UObject* WorldContextObject)
{
	if (WorldContextObject)
	{
		if (UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject))
		{
			return World->GetSubsystem<URegionSubsystem>();
		}
	}
	return nullptr;
}

FGameplayTag URegionSubsystem::CreateTagFromParts(const TArray<FString>& Parts, int32 NumDetail)
{
	NumDetail = FMath::Clamp(NumDetail, 0, Parts.Num());

	FString TagString;
	for (int32 i = 0; i < NumDetail; ++i)
	{
		if (i > 0)
			TagString.Append(TEXT(".")); 
		TagString.Append(Parts[i]);
	}
	
	return FGameplayTag::RequestGameplayTag(FName(*TagString));
}

TArray<FString> URegionSubsystem::CreatePartsFromTag(FGameplayTag Tag)
{
	TArray<FString> Parts;
	Tag.ToString().ParseIntoArray(Parts, TEXT("."));
	return Parts;
}

TSet<ARegionVolume*> URegionSubsystem::GetChildRegionVolumesByTag(const UObject* WorldContext, FGameplayTag RegionTag)
{
	if (!WorldContext || !WorldContext->GetWorld())
		return TSet<ARegionVolume*>();
	
	TSet<ARegionVolume*> RegionVolumes;
	for (TActorIterator<ARegionVolume> It(WorldContext->GetWorld()); It; ++It)
	{
		ARegionVolume* Volume = *It;
		if (!Volume)
			continue;

		if (!Volume->IsChildRegion(RegionTag))
			continue;
		
		RegionVolumes.Add(*It);
	}
	return RegionVolumes;
}

TSet<ARegionVolume*> URegionSubsystem::GetParentRegionVolumesByTag(const UObject* WorldContext, FGameplayTag RegionTag)
{
	if (!WorldContext || !WorldContext->GetWorld())
		return TSet<ARegionVolume*>();
	
	TSet<ARegionVolume*> RegionVolumes;
	for (TActorIterator<ARegionVolume> It(WorldContext->GetWorld()); It; ++It)
	{
		ARegionVolume* Volume = *It;
		if (!Volume)
			continue;

		if (!Volume->IsParentRegion(RegionTag))
			continue;
		
		RegionVolumes.Add(*It);
	}
	return RegionVolumes;
}

TSet<ARegionVolume*> URegionSubsystem::GetAllRegionVolumes(const UObject* WorldContext)
{
	if (!WorldContext || !WorldContext->GetWorld())
		return TSet<ARegionVolume*>();
	
	TSet<ARegionVolume*> RegionVolumes;
	for (TActorIterator<ARegionVolume> It(WorldContext->GetWorld()); It; ++It)
	{
		if (!*It)
			continue;
		
		RegionVolumes.Add(*It);
	}
	return RegionVolumes;
}

TSet<ARegionVolume*> URegionSubsystem::GetContainingVolumes(FVector Location) const
{
	TSet<ARegionVolume*> Regions;

	//Recache if empty
	if (CachedRegionVolumes.Num() == 0)
		BuildCache();
	
	for (TWeakObjectPtr<ARegionVolume> WeakVolume : CachedRegionVolumes)
	{
		if (ARegionVolume* Volume = WeakVolume.Get())
		{
			if (Volume->Contains(Location))
			{
				Regions.Add(Volume);
			}
		}
	}

	return Regions;
}

TSet<FGameplayTag> URegionSubsystem::GetContainingRegions(FVector Location) const
{
	TSet<FGameplayTag> Regions;

	for (ARegionVolume* Volume : GetContainingVolumes(Location))
	{
		if (Volume)
		{
			Regions.Add(Volume->GetRegionTag());
		}
	}

	return Regions;
}

FGameplayTag URegionSubsystem::GetRelevantRegion(FVector Location) const
{
	TSet<FGameplayTag> ContainingRegions = GetContainingRegions(Location);

	FGameplayTag MostDetailedTag;
	int32 MaxDetailLevel = 0;

	for (const FGameplayTag& Tag : ContainingRegions)
	{
		TArray<FString> Components = CreatePartsFromTag(Tag);
		
		int32 DetailLevel = Components.Num();
		
		if (DetailLevel > MaxDetailLevel)
		{
			MaxDetailLevel = DetailLevel;
			MostDetailedTag = Tag;
		}
	}

	return MostDetailedTag;
}


TSet<ARegionVolume*> URegionSubsystem::GetVolumesByTag(const FGameplayTag& Tag) const
{
	TSet<ARegionVolume*> Regions;

	//Recache if empty
	if (CachedRegionVolumes.Num() == 0)
		BuildCache();
	
	for (TWeakObjectPtr<ARegionVolume> WeakVolume : CachedRegionVolumes)
	{
		if (ARegionVolume* Volume = WeakVolume.Get())
		{
			if (Volume->GetRegionTag().MatchesTagExact(Tag))
			{
				Regions.Add(Volume);
			}
		}
	}

	return Regions;
}

#if WITH_EDITOR
void URegionSubsystem::ResetAllHighlights(UObject* WorldContext)
{
	if (!WorldContext || !WorldContext->GetWorld())
		return;
	
	for (auto Volume : GetAllRegionVolumes(WorldContext))
	{
		Volume->RegionBox->SetLineThickness(0);
		Volume->RegionBox->ShapeColor = FColor::Cyan;
	}
}

void URegionSubsystem::HighlightHierarchy(UObject* WorldContext, FGameplayTag RegionTag)
{
	ResetAllHighlights(WorldContext);

	TArray<FString> Parts = CreatePartsFromTag(RegionTag);
	TArray<ARegionVolume*> RegionVolumes = GetChildRegionVolumesByTag(WorldContext, RegionTag).Array();
	for (ARegionVolume* Volume : GetParentRegionVolumesByTag(WorldContext, RegionTag))
	{
		if (!Volume)
			continue;

		RegionVolumes.AddUnique(Volume);
	}
	RegionVolumes.Sort();

	FLinearColor StartColor = FLinearColor::Red;
	FLinearColor EndColor   = FLinearColor::Green;
	int32 Count = RegionVolumes.Num();
	
	for (int i = 0; i < Count; ++i)
	{
		ARegionVolume* Volume = RegionVolumes[i];

		Volume->RegionBox->SetLineThickness(2);
		const float t = (Count > 1) ? static_cast<float>(i) / (Count - 1) : 0.0f;
		FLinearColor InterpColor = FLinearColor::LerpUsingHSV(StartColor, EndColor, t);

		Volume->RegionBox->ShapeColor = InterpColor.ToFColor(true);
	}
}
#endif
