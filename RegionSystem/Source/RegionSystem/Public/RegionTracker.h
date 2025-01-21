// Copyright Phoenix Dawn Development LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "RegionTracker.generated.h"


class UAbilitySystemComponent;
class ARegionVolume;

DECLARE_LOG_CATEGORY_EXTERN(LogRegions, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRegionChange, FGameplayTag, RegionTag);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class REGIONSYSTEM_API URegionTracker : public UActorComponent
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable, Category="Region")
	FOnRegionChange OnRegionEnter;
	UPROPERTY(BlueprintAssignable, Category="Region")
	FOnRegionChange OnRegionExit;
	
	UFUNCTION(BlueprintCallable)
	bool IsInRegion(FGameplayTag Tag) const;
	UFUNCTION(BlueprintCallable)
	TSet<FGameplayTag> GetRegions() const;
	UFUNCTION(BlueprintCallable)
	FGameplayTag GetRelevantRegion() const;
	UFUNCTION(BlueprintCallable)
	TSet<ARegionVolume*> GetRegionRefs() const;

protected:

	friend ARegionVolume;

	UFUNCTION()
	void AddRegion(ARegionVolume* Volume);
	UFUNCTION()
	void RemoveRegion(ARegionVolume* Volume);
	UFUNCTION()
	FGameplayTag CalculateRelevantRegion() const;
	
	UPROPERTY(Transient)
	TSet<ARegionVolume*> RegionRefs;
	UPROPERTY(Transient)
	mutable FGameplayTag CachedRegionTag;
	
};
