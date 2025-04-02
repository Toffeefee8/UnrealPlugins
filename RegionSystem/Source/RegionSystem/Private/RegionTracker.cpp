#include "RegionTracker.h"

#include "AbilitySystemComponent.h"
#include "DebugFunctionLibrary.h"
#include "Region.h"
#include "RegionSystem.h"
#include "RegionTags.h"
#include "RegionVolume.h"
#include "Extensions/GameplayTagExtensions.h"
#include "GameFramework/Character.h"

void URegionTracker::ForceSetRegion_Implementation(FGameplayTag NewRegion)
{
	UE_LOG(LogRegions, Warning, TEXT("Forcefully setting region [%s] on region tracker! Previous Region: [%s]! "),
		*CachedRegionTag.GetTagName().ToString(),
		*NewRegion.GetTagName().ToString())
	
	CachedRegionTag = NewRegion;
}

FGameplayTag URegionTracker::GetRegionTag_Implementation() const
{
	return GetContainingRegionTag();
}

void URegionTracker::GetCheckData_Implementation(FVector& CheckLocation, ERegionTypes& DesiredType, bool& bDisableRuntimeChecks) const
{
	CheckLocation = GetOwner()->GetActorLocation();
	DesiredType = ERegionTypes::Room;
	bDisableRuntimeChecks = false;
}

void URegionTracker::BeginPlay()
{
	Super::BeginPlay();

	URegionSubsystem* RegionSubsystem = URegionSubsystem::Get(this);
	if (RegionSubsystem)
	{
		CachedRegionTag = RegionSubsystem->GetRegionTagByTracker(this);
	}
}

bool URegionTracker::IsInRegion(FGameplayTag Tag) const
{
	for (auto Region : GetRegionTags())
	{
		if (Region.MatchesTag(Tag))
			return true;
	}
	return false;
}

FGameplayTagContainer URegionTracker::GetRegionTags() const
{
	FGameplayTagContainer Regions;
	for (auto RegionRef : RegionRefs)
	{
		Regions.AddTag(RegionRef->GetRegionTag());
	}
	return Regions;
}

FGameplayTag URegionTracker::GetContainingRegionTag() const
{
	if (CachedRegionTag.IsValid())
		return CachedRegionTag;

	CachedRegionTag = CalculateRelevantRegion();
	return CachedRegionTag;
}

FGameplayTag URegionTracker::GetRegionTagOfType(ERegionTypes DesiredType) const
{
	URegionSubsystem* RegionSubsystem = URegionSubsystem::Get(this);
	if (!RegionSubsystem)
		return {};

	URegion* Region = RegionSubsystem->GetRegionByTag(GetContainingRegionTag());
	if (!Region)
		return {};

	return Region->GetRegionTagOfType(DesiredType);
}

TArray<URegion*> URegionTracker::GetRegionRefs() const
{
	TArray<URegion*> Result;
	Result.Reserve(RegionRefs.Num());
	
	for (const TWeakObjectPtr<URegion>& Region : RegionRefs)
	{
		if (Region.IsValid())
		{
			Result.Add(Region.Get());
		}
	}
	return Result;
}

bool URegionTracker::IsInValidRegion() const
{
	return RegionRefs.Num() > 0;
}

void URegionTracker::AddRegion(URegion* Region)
{
	const FGameplayTag RegionTag = Region->GetRegionTag();
	bool bShouldTrigger = !IsInRegion(RegionTag);
	
	RegionRefs.Add(Region);

	if (bShouldTrigger)
	{
		OnRegionEnter.Broadcast(RegionTag);
		DEBUG_SIMPLE(LogRegions, Log, FColor::Green, FString::Printf(TEXT("Entered %s"), *RegionTag.GetTagName().ToString()), RegionTags::Name);
	}
	else
	{
		DEBUG_SIMPLE(LogRegions, Log, FColor::White, FString::Printf(TEXT("Quietly Entered %s"), *RegionTag.GetTagName().ToString()), RegionTags::Name);
	}

	const FGameplayTag PreviousRegionTag = CachedRegionTag;
	CachedRegionTag = FGameplayTag();
	
	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		if (APlayerState* PlayerState = Character->GetPlayerState())
		{
			if (UAbilitySystemComponent* AbilitySystemComponent = PlayerState->GetComponentByClass<UAbilitySystemComponent>())
			{
				AbilitySystemComponent->RemoveLooseGameplayTag(PreviousRegionTag);
				AbilitySystemComponent->AddLooseGameplayTag(GetContainingRegionTag());
			}
		}
	}
}

void URegionTracker::RemoveRegion(URegion* Region)
{
	RegionRefs.Remove(Region);
	
	const FGameplayTag RegionTag = Region->GetRegionTag();
	if (!IsInRegion(RegionTag))
	{
		OnRegionExit.Broadcast(RegionTag);
		DEBUG_SIMPLE(LogRegions, Log, FColor::Red, FString::Printf(TEXT("Exited %s"), *RegionTag.GetTagName().ToString()), RegionTags::Name)
	}
	else
	{
		DEBUG_SIMPLE(LogRegions, Log, FColor::White, FString::Printf(TEXT("Quietly Exited %s"), *RegionTag.GetTagName().ToString()), RegionTags::Name)
	}
	
	const FGameplayTag PreviousRegionTag = CachedRegionTag;
	CachedRegionTag = FGameplayTag();
	
	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		if (APlayerState* PlayerState = Character->GetPlayerState())
		{
			if (UAbilitySystemComponent* AbilitySystemComponent = PlayerState->GetComponentByClass<UAbilitySystemComponent>())
			{
				AbilitySystemComponent->RemoveLooseGameplayTag(PreviousRegionTag);
				AbilitySystemComponent->AddLooseGameplayTag(GetContainingRegionTag());
			}
		}
	}
}

FGameplayTag URegionTracker::CalculateRelevantRegion() const
{
	return UGameplayTagExtensions::GetMostDetailedTag(GetRegionTags());
}
