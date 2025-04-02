#include "RegionSubsystem.h"
#include "EngineUtils.h"
#include "RegionVolume.h"
#include "Region.h"
#include "RegionSystem.h"
#include "RegionTags.h"
#include "RegionTracker.h"
#include "Extensions/GameplayTagExtensions.h"
#include "Modules/RegionModule.h"
#include "Modules/RegionModuleDefaults.h"
#include "Settings/RegionSettings.h"

#if	WITH_EDITOR
#include "Components/BoxComponent.h"
#endif

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

TSet<ARegionVolume*> URegionSubsystem::FindAllRegionVolumes() const
{
	if (!GetWorld())
		return TSet<ARegionVolume*>();
	
	TSet<ARegionVolume*> RegionVolumes;
	for (TActorIterator<ARegionVolume> It(GetWorld()); It; ++It)
	{
		if (!*It)
			continue;
		
		RegionVolumes.Add(*It);
	}
	return RegionVolumes;
}

void URegionSubsystem::RefreshRegions()
{
	for (auto RegionVolume : FindAllRegionVolumes())
	{
		if (!RegionVolume->bRegistered)
		{
			RegionVolume->RegisterSelfWithSubsystem();
		}
	}

	FGameplayTagContainer InvalidRegions;
	for (auto Pair : RegionMap)
	{
		checkf(Pair.Value, TEXT("RegionMap had invalid Region Pointer!"));

		//Refresh Region
		Pair.Value->RefreshRegion();

		//Add To invalid regions
		if (Pair.Value->GetRegionVolumes().Num() <= 0)
			InvalidRegions.AddTag(Pair.Key);
	}

	//Cleanup Regions with no Volumes
	for (auto InvalidRegion : InvalidRegions)
	{
		DestroyRegion(InvalidRegion);
	}

	OnRegionRefresh.Broadcast();
}

TArray<UObject*> URegionSubsystem::GetAllRegionObjects() const
{
	TArray<UObject*> RegionObjects;
	UWorld* World = GetWorld();
	if (!World)
		return RegionObjects;
	
	for (TActorIterator<AActor> ActorIt(World); ActorIt; ++ActorIt)
	{
		AActor* Actor = *ActorIt;
		if (!Actor) continue;
		
		if (Actor->Implements<URegionObject>())
		{
			RegionObjects.Add(Actor);
		}
		
		for (TComponentIterator<UActorComponent> CompIt(Actor); CompIt; ++CompIt)
		{
			UActorComponent* Component = *CompIt;
			if (Component && Component->Implements<URegionObject>())
			{
				RegionObjects.Add(Component);
			}
		}
	}
	
	return RegionObjects;
}

void URegionSubsystem::ReevaluateAllRegionObjects() const
{
	UWorld* World = GetWorld();
	if (!World)
		return;
	
	for (TActorIterator<AActor> ActorIt(World); ActorIt; ++ActorIt)
	{
		AActor* Actor = *ActorIt;
		if (!Actor)
			continue;
		
		ReevaluateRegionObject(Actor);
		
		for (TComponentIterator<UActorComponent> CompIt(Actor); CompIt; ++CompIt)
		{
			UActorComponent* Component = *CompIt;
			ReevaluateRegionObject(Component);
		}
	}
}

bool URegionSubsystem::ReevaluateRegionObject(UObject* Object) const
{
	if (!Object || !Object->Implements<URegionObject>())
		return false;

	UE_LOG(LogRegions, Log, TEXT("Reevaluating Region Object: %s"), *Object->GetName());
	FGameplayTag OldRegionTag = IRegionObject::Execute_GetRegionTag(Object);
	
	FVector Location {};
	ERegionTypes DesiredType {};
	bool bDisableRuntimeChecks = false;
	IRegionObject::Execute_GetCheckData(Object, Location, DesiredType, bDisableRuntimeChecks);

	//Return jf in game and no runtime checks
#if WITH_EDITOR
	if (GetWorld() && GetWorld()->WorldType == EWorldType::Editor)
		bDisableRuntimeChecks = false;
#endif
	if (bDisableRuntimeChecks)
	{
		UE_LOG(LogRegions, Log, TEXT("Skipping Reevaluation of Region Object: %s. Runtime checks disabled."), *Object->GetName());
		return false;
	}

	//Return if no change
	FGameplayTag NewRegionTag = GetRegionTagByLocation(Location, DesiredType);
	if (NewRegionTag == OldRegionTag)
	{
		UE_LOG(LogRegions, Log, TEXT("Region Object: %s has not changed region: %s"), *Object->GetName(), *OldRegionTag.ToString());
		return true;
	}
	
	UE_LOG(LogRegions, Log, TEXT("Region Object: %s changed region from: %s to: %s"), *Object->GetName(), *OldRegionTag.ToString(), *NewRegionTag.ToString());
	IRegionObject::Execute_ForceSetRegion(Object, GetRegionTagByLocation(Location, DesiredType));
	return true;
}

TSet<URegion*> URegionSubsystem::GetAllRegions() const
{
	TSet<URegion*> Regions;
	for (auto Region : RegionMap)
	{
		Regions.Add(Region.Value);
	}
	return Regions;
}

FGameplayTagContainer URegionSubsystem::GetAllRegionTags() const
{
	FGameplayTagContainer RegionTags;
	for (auto Region : RegionMap)
	{
		RegionTags.AddTag(Region.Key);
	}
	return RegionTags;
}

TSet<ARegionVolume*> URegionSubsystem::GetAllRegionVolumes() const
{
	TSet<ARegionVolume*> Volumes;
	for (auto Region : RegionMap)
	{
		if (Region.Value)
		{
			for (auto Volume : Region.Value->GetRegionVolumes())
			{
				Volumes.Add(Volume);
			}
		}
	}
	return Volumes;
}

URegion* URegionSubsystem::CreateNewRegion(FGameplayTag RegionTag)
{
	URegion* Region = NewObject<URegion>(this);
	Region->RegionTag = RegionTag;

	URegion* ParentRegion = Region->GetParentRegion();
	TSet<URegion*> ChildRegions = Region->GetChildRegions();
	RegionMap.Add(RegionTag, Region);

	CreateRegionModules(Region);

	//Notify Parent Region
	if (ParentRegion)
		ParentRegion->OnNewChild(Region);

	//Notify Child Regions
	for (auto ChildRegion : ChildRegions)
		ChildRegion->OnNewParent(ParentRegion, Region);

	OnRegionAdded.Broadcast(Region);
	return Region;
}

void URegionSubsystem::CreateRegionModules(URegion* Region)
{
	//Create Modules
	for (auto ModuleClass : URegionSettings::Get()->ImplementedModuleClasses)
		Region->RegionModules.Add(NewObject<URegionModule>(Region, ModuleClass));

	//Start Modules
	for (auto Module : Region->RegionModules)
		Module->StartModule(Region);
}

void URegionSubsystem::DestroyRegion(FGameplayTag RegionTag)
{
	if (!RegionMap.Contains(RegionTag))
		return;
	
	DestroyRegion(RegionMap[RegionTag]);
}

void URegionSubsystem::DestroyRegion(URegion* Region)
{
	for (auto Volume : Region->Volumes)
		Volume.Key->bRegistered = false;

	OnRegionAdded.Broadcast(Region);
	Region->EndRegion();
	RegionMap.Remove(Region->GetRegionTag());
	Region->MarkAsGarbage();
}

void URegionSubsystem::RegisterVolume(ARegionVolume* Volume)
{
	FGameplayTag RegionTag = Volume->GetRegionTag();
	TObjectPtr<URegion>* RegionPtr = RegionMap.Find(RegionTag);
	
	TObjectPtr<URegion> Region {};
	if (RegionPtr)
		Region = *RegionPtr;
	else
		Region = CreateNewRegion(RegionTag);

	Region->AddVolume(Volume);
}

void URegionSubsystem::DeregisterVolume(ARegionVolume* Volume)
{
	FGameplayTag RegionTag = Volume->GetRegionTag();
	TObjectPtr<URegion>* RegionPtr = RegionMap.Find(RegionTag);
	if (!RegionPtr)
		return;
	
	TObjectPtr<URegion> Region = *RegionPtr;
	Region->RemoveVolume(Volume);

	if (Region->Volumes.Num()<=0)
		DestroyRegion(Region);
}

bool URegionSubsystem::EnterRegionVolume(URegionTracker* Tracker, const ARegionVolume* Volume) const
{
	if (!Volume || !Tracker)
		return false;

	URegion* Region = GetRegionByTag(Volume->GetRegionTag());
	if (!Region)
		return false;

	return Region->TryEnterRegion(Tracker, Volume);
}

bool URegionSubsystem::ExitRegionVolume(URegionTracker* Tracker, const ARegionVolume* Volume) const
{
	if (!Volume || !Tracker)
		return false;

	URegion* Region = GetRegionByTag(Volume->GetRegionTag());
	if (!Region)
		return false;

	return Region->TryExitRegion(Tracker, Volume);
}

URegion* URegionSubsystem::GetRegionByTag(FGameplayTag RegionTag, bool AllowParents) const
{
	if (!RegionTag.IsValid())
		return nullptr;
    
	FGameplayTag ParentTag = RegionTag;
	do 
	{
		if (RegionMap.Contains(ParentTag))
			return RegionMap[ParentTag];
		ParentTag = UGameplayTagExtensions::RemoveTagDepth(ParentTag);
	} while (ParentTag.IsValid() && AllowParents);
        
	return nullptr;
}

URegion* URegionSubsystem::GetRegionByLocation(FVector Location, ERegionTypes DesiredType) const
{
	FGameplayTag RegionTag = GetRegionTagByLocation(Location, DesiredType);
	return GetRegionByTag(RegionTag);
}

URegion* URegionSubsystem::GetRegionByVolume(FVector Location, FVector BoxExtent, ERegionTypes DesiredType) const
{
	FGameplayTag RegionTag = GetRegionTagByVolume(Location, BoxExtent, DesiredType);
	return GetRegionByTag(RegionTag);
}

URegion* URegionSubsystem::GetRegionByActor(AActor* Actor) const
{
	FGameplayTag RegionTag = GetRegionTagByActor(Actor);
	return GetRegionByTag(RegionTag);
}

URegion* URegionSubsystem::GetRegionByInterface(UObject* Object) const
{
	FGameplayTag RegionTag = GetRegionTagByInterface(Object);
	return GetRegionByTag(RegionTag);
}

URegion* URegionSubsystem::GetRegionByTracker(URegionTracker* Tracker) const
{
	FGameplayTag RegionTag = GetRegionTagByTracker(Tracker);
	return GetRegionByTag(RegionTag);
}

URegion* URegionSubsystem::GetRegionByState(APlayerState* PlayerState) const
{
	FGameplayTag RegionTag = GetRegionTagByState(PlayerState);
	return GetRegionByTag(RegionTag);
}

FGameplayTag URegionSubsystem::GetRegionTagByLocation(FVector Location, ERegionTypes DesiredType) const
{
	//TODO: Rework for efficient child based search
	FGameplayTagContainer ContainedRegionTags;
	FGameplayTagContainer ContainedDesiredRegionTags;
	for (auto RegionPair : RegionMap)
	{
		if (RegionPair.Value->Contains(Location))
		{
			ContainedRegionTags.AddTag(RegionPair.Key);
			if (RegionPair.Value->GetRegionType() == DesiredType)
				ContainedDesiredRegionTags.AddTag(RegionPair.Key);
		}
	}
	FGameplayTag DesiredTag = UGameplayTagExtensions::GetMostDetailedTag(ContainedDesiredRegionTags);
	if (DesiredTag.IsValid())
		return DesiredTag;

	FGameplayTag FallbackTag = UGameplayTagExtensions::GetMostDetailedTag(ContainedRegionTags);
	UE_LOG(LogRegions, Warning, TEXT("No matching region of type [%s] found at location [%s]. Returning most detailed available tag [%s]."), 
		*UEnum::GetValueAsString(DesiredType), *Location.ToString(), *FallbackTag.GetTagName().ToString());
	
	return FallbackTag;
}

FGameplayTag URegionSubsystem::GetRegionTagByVolume(const FVector Location, const FVector BoxExtent, ERegionTypes DesiredType) const
{
	FGameplayTagContainer ContainedRegionTags;
	FGameplayTagContainer ContainedDesiredRegionTags;
	for (auto RegionPair : RegionMap)
	{
		if (RegionPair.Value->ContainsFully(Location, BoxExtent))
		{
			ContainedRegionTags.AddTag(RegionPair.Key);
			if (RegionPair.Value->GetRegionType() == DesiredType)
				ContainedDesiredRegionTags.AddTag(RegionPair.Key);
		}
	}
	FGameplayTag DesiredTag = UGameplayTagExtensions::GetMostDetailedTag(ContainedDesiredRegionTags);
	if (DesiredTag.IsValid())
		return DesiredTag;

	FGameplayTag FallbackTag = UGameplayTagExtensions::GetMostDetailedTag(ContainedRegionTags);
	UE_LOG(LogRegions, Warning, TEXT("No matching region of type [%s] found at volume location [%s] and extent [%s]. Returning most detailed available tag [%s]."), 
		*UEnum::GetValueAsString(DesiredType), *Location.ToString(), *BoxExtent.ToString(), *FallbackTag.GetTagName().ToString());

	return FallbackTag;
}

FGameplayTag URegionSubsystem::GetRegionTagByActor(AActor* Actor) const
{
	
	if (!Actor)
		return {};
	
	URegionTracker* Tracker = Actor->GetComponentByClass<URegionTracker>();
	if (Tracker && Tracker->IsInValidRegion())
		return Tracker->GetContainingRegionTag();

	FGameplayTag ReturnTag = GetRegionTagByInterface(Actor);
	if (ReturnTag.IsValid())
		return ReturnTag;
	
	return GetRegionTagByLocation(Actor->GetActorLocation());
}

FGameplayTag URegionSubsystem::GetRegionTagByInterface(UObject* Object) const
{
	if (Object->Implements<URegionObject>())
		return IRegionObject::Execute_GetRegionTag(Object);

	return FGameplayTag();
}

FGameplayTag URegionSubsystem::GetRegionTagByTracker(URegionTracker* Tracker) const
{
	if (!Tracker)
		return {};
	
	if (Tracker->IsInValidRegion())
		return Tracker->GetContainingRegionTag();

	if (Tracker->GetOwner())
		return GetRegionTagByLocation(Tracker->GetOwner()->GetActorLocation());

	return {};
}

FGameplayTag URegionSubsystem::GetRegionTagByState(APlayerState* PlayerState) const
{
	if (!PlayerState)
		return {};

	return GetRegionTagByActor(PlayerState->GetPawn());
}

#if WITH_EDITOR
void URegionSubsystem::ShowAllRegions()
{
	for (auto RegionVolume : GetAllRegionVolumes())
	{
		if (RegionVolume)
		{
			RegionVolume->ShowVolume();
		}
	}
}

void URegionSubsystem::HideAllRegions()
{
	for (auto RegionVolume : GetAllRegionVolumes())
	{
		if (RegionVolume)
		{
			RegionVolume->HideVolume();
		}
	}
}

void URegionSubsystem::HighlightRegion(FGameplayTag Region, bool bHideChildren)
{
	for (auto RegionVolume : GetAllRegionVolumes())
	{
		if (RegionVolume)
		{
			if (RegionVolume->GetRegionTag().MatchesTagExact(Region))
			{
				RegionVolume->SetHighlight(0);
			}
			else if (!bHideChildren && RegionVolume->GetRegionTag().MatchesTag(Region))
			{
				RegionVolume->SetHighlight(1);
			}
			else
			{
				RegionVolume->HideVolume();
			}
		}
	}
}

void URegionSubsystem::BakeRegions()
{
	for (auto Pair : RegionMap)
	{
		if (!Pair.Value)
			continue;
		
		BakeRegion(Pair.Value);
	}
}

void URegionSubsystem::BakeRegion(FGameplayTag RegionTag)
{
	URegion* Region = GetRegionByTag(RegionTag);
	if (!Region)
	{
		UE_LOG(LogRegions, Error, TEXT("Region is invalid when trying to bake!"));
		return;
	}

	BakeRegion(Region);
}

void URegionSubsystem::ForceRefreshRegions()
{
	RefreshRegions();
	// for (auto Map : RegionMap)
	// {
	// 	DestroyRegion(Map.Value);
	// }
	//
	// RegionMap.Empty();
	//
	// for (auto RegionVolume : FindAllRegionVolumes())
	// {
	// 	RegionVolume->bRegistered = false;
	// 	RegionVolume->RegisterSelfWithSubsystem();
	// }
}

void URegionSubsystem::BakeRegion(URegion* Region)
{
	TArray<ARegionVolume*> Volumes = Region->GetRegionVolumes();
	for (auto Volume : Volumes)
	{
		Volume->Bake();
	}

	if (Volumes.Num() <= 0)
	{
		UE_LOG(LogRegions, Log, TEXT("Found No Volumes with requested tag!"))
	}
}
#endif
