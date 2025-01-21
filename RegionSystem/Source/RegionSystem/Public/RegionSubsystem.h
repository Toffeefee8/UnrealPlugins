// Copyright Phoenix Dawn Development LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RegionSubsystem.generated.h"

class ARegionVolume;
/**
 * Region Subsystem that caches ARegionVolume instances
 */
UCLASS()
class REGIONSYSTEM_API URegionSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:

	static URegionSubsystem* Get(const UObject* WorldContextObject);

	//Helpers
	static FGameplayTag CreateTagFromParts(const TArray<FString>& Parts, int32 NumDetail);
	static TArray<FString> CreatePartsFromTag(FGameplayTag Tag);
	static TSet<ARegionVolume*> GetChildRegionVolumesByTag(const UObject* WorldContext, FGameplayTag RegionTag);
	static TSet<ARegionVolume*> GetParentRegionVolumesByTag(const UObject* WorldContext, FGameplayTag RegionTag);
	static TSet<ARegionVolume*> GetAllRegionVolumes(const UObject* WorldContext);

	//Location To Region
	UFUNCTION(BlueprintCallable)
	TSet<ARegionVolume*> GetContainingVolumes(FVector Location) const;
	UFUNCTION(BlueprintCallable)
	TSet<FGameplayTag> GetContainingRegions(FVector Location) const;
	UFUNCTION(BlueprintCallable)
	FGameplayTag GetRelevantRegion(FVector Location) const;

	UFUNCTION(BlueprintCallable)
	void ForceResetCache() const;

	//Region To Location
	UFUNCTION(BlueprintCallable)
	FVector ProjectPointToNavigationInRegion(UPARAM(meta=(Categories="Game.Region")) FGameplayTag RegionTag);
	UFUNCTION(BlueprintCallable)
	FVector GetRegionCenterPoint(UPARAM(meta=(Categories="Game.Region")) FGameplayTag RegionTag);
	UFUNCTION(BlueprintCallable)
	FVector GetRegionExtents(UPARAM(meta=(Categories="Game.Region")) FGameplayTag RegionTag);	
	UFUNCTION(BlueprintCallable)
	FTransform GetRegionTransform(UPARAM(meta=(Categories="Game.Region")) FGameplayTag RegionTag);
	
	UFUNCTION(BlueprintCallable)
	void RegisterVolume(ARegionVolume* Volume);
	UFUNCTION(BlueprintCallable)
	void DeregisterVolume(ARegionVolume* Volume);
	
protected:
	TSet<ARegionVolume*> GetVolumesByTag(const FGameplayTag& Tag) const;

private:
	UPROPERTY(Transient)
	mutable TArray<TWeakObjectPtr<ARegionVolume>> CachedRegionVolumes;
	
	void BuildCache() const;

#pragma region Editor
#if WITH_EDITOR
public:
	UFUNCTION()
	void ResetAllHighlights(UObject* WorldContext);
	UFUNCTION()
	void HighlightHierarchy(UObject* WorldContext, FGameplayTag RegionTag);
#endif
#pragma endregion
};


