#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "RegionVolume.h"
#include "UObject/Object.h"
#include "Region.generated.h"

class URegionTracker;

USTRUCT(BlueprintType)
struct FContainedTrackers
{
	GENERATED_BODY()

public:

	operator bool() const
	{
		return ContainedTrackers.Num() > 0;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TObjectPtr<URegionTracker>> ContainedTrackers;
	
};

UCLASS()
class REGIONSYSTEM_API URegion : public UObject
{
	GENERATED_BODY()

#pragma region Functions

public:

	//Executions
	void StartRegion();
	void RefreshRegion();
	void EndRegion();
	
	void OnNewChild(URegion* NewChild);
	void OnNewParent(URegion* OldParent, URegion* NewParent);

	//RegionTag
	UFUNCTION(BlueprintCallable, Category = "Regions")
	FGameplayTag GetRegionTag() const;
	UFUNCTION(BlueprintCallable, Category = "Regions")
	FGameplayTag GetRegionTagOfType(ERegionTypes RegionType) const;
	UFUNCTION(BlueprintCosmetic, Category = "Regions")
	int8 GetRegionDepth() const;

	//RegionType
	UFUNCTION(BlueprintCallable, Category = "Regions")
	ERegionTypes GetRegionType() const;

	//Parenting
	UFUNCTION(BlueprintCallable, Category = "Regions")
	bool IsParentRegion(UPARAM(meta=(Categories="Regions.Areas")) FGameplayTag InRegionTag) const;
	UFUNCTION(BlueprintCallable, Category = "Regions")
	bool IsChildRegion(UPARAM(meta=(Categories="Regions.Areas")) FGameplayTag InRegionTag) const;
	UFUNCTION(BlueprintCallable, Category = "Regions")
	bool IsRegionExact(UPARAM(meta=(Categories="Regions.Areas")) FGameplayTag InRegionTag) const;

	//Checks
	UFUNCTION(BlueprintCallable, Category = "Regions")
	bool Contains(FVector Location) const;
	UFUNCTION(BlueprintCallable, Category = "Regions")
	bool ContainsFully(FVector Location, FVector BoxExtents) const;

	//POI
	UFUNCTION(BlueprintCallable, Category = "Regions")
	FRegionPOIData GetRandomPOI() const;
	UFUNCTION(BlueprintCallable, Category = "Regions")
	FRegionPOIData GetClosestPOI(FVector Vector) const;

	//Gets
	UFUNCTION(BlueprintCallable, Category = "Regions")
	TArray<ARegionVolume*> GetRegionVolumes() const;

	//Sister Regions
	UFUNCTION(BlueprintCallable, Category = "Regions")
	const URegion* GetRegionOfType(ERegionTypes RegionType, bool AllowWrongIfNotFound = true) const;
	UFUNCTION(BlueprintCallable, Category = "Regions")
	URegion* GetParentRegion() const;
	UFUNCTION(BlueprintCallable, Category = "Regions")
	FGameplayTag GetParentRegionTag() const;
	UFUNCTION(BlueprintCallable, Category = "Regions")
	TSet<URegion*> GetChildRegions() const;
	UFUNCTION(BlueprintCallable, Category = "Regions")
	FGameplayTagContainer GetChildRegionTags() const;

	//Modules
	UFUNCTION(BlueprintCallable, Category = "Regions", meta = (DeterminesOutputType = "ModuleClass", AdvancedDisplay = 1))
	URegionModule* GetRegionModule(TSubclassOf<URegionModule> ModuleClass, bool AllowParentSearch = true) const;
	template<typename T>
	T* GetRegionModule(bool AllowParentSearch = true) const;

protected:

	//Enter/Exit
	bool TryEnterRegion(URegionTracker* Tracker, const ARegionVolume* Volume);
	bool TryExitRegion(URegionTracker* Tracker, const ARegionVolume* Volume);
		
	//Overlaps
	void OnEnterRegion(URegionTracker* Tracker);
	void OnExitRegion(URegionTracker* Tracker);
	
	friend URegionSubsystem;

	//Volume Registration
	void AddVolume(ARegionVolume* Volume);
	void RemoveVolume(ARegionVolume* Volume);

	//Helpers
	TArray<FRegionPOIData> GetAllPOIs() const;
	
#pragma endregion

#pragma region Properties

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Region")
	FGameplayTag RegionTag {};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Region|Volumes")
	TMap<TObjectPtr<ARegionVolume>, FContainedTrackers> Volumes {};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Region|Modules")
	TArray<TObjectPtr<URegionModule>> RegionModules {};

#pragma endregion

#pragma region Operators
public:
	friend bool operator<(const URegion& Lhs, const URegion& RHS)
	{
		return Lhs.GetRegionDepth() < RHS.GetRegionDepth();
	}

	friend bool operator<=(const URegion& Lhs, const URegion& RHS)
	{
		return !(RHS < Lhs);
	}

	friend bool operator>(const URegion& Lhs, const URegion& RHS)
	{
		return RHS < Lhs;
	}

	friend bool operator>=(const URegion& Lhs, const URegion& RHS)
	{
		return !(Lhs < RHS);
	}
#pragma endregion
};

template <typename T>
T* URegion::GetRegionModule(bool AllowParentSearch) const
{
	return Cast<T>(GetRegionModule(T::StaticClass(), AllowParentSearch));
}
