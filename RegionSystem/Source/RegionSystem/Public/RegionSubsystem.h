#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Modules/RegionModuleDefaults.h"
#include "Structs/RegionTypes.h"
#include "RegionSubsystem.generated.h"

class URegionModule;
class URegionTracker;
class URegion;
class ARegionVolume;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRegionChange, URegion*, Region);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRegionRefresh);

UCLASS()
class REGIONSYSTEM_API URegionSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:

	static URegionSubsystem* Get(const UObject* WorldContextObject);

	//Delegates
	UPROPERTY(BlueprintAssignable)
	FRegionChange OnRegionAdded;
	UPROPERTY(BlueprintAssignable)
	FRegionChange OnRegionRemoved;

	UPROPERTY(BlueprintAssignable)
	FRegionRefresh OnRegionRefresh;

	//Helpers
	TSet<ARegionVolume*> FindAllRegionVolumes() const;
	
	//Region Objects
	TArray<UObject*> GetAllRegionObjects() const;
	void ReevaluateAllRegionObjects() const;
	bool ReevaluateRegionObject(UObject* Object) const;

	//Extern Actions
	UFUNCTION(BlueprintCallable)
	void RefreshRegions();

	//Gets
	UFUNCTION(BlueprintCallable)
	TSet<URegion*> GetAllRegions() const;
	UFUNCTION(BlueprintCallable)
	FGameplayTagContainer GetAllRegionTags() const;
	UFUNCTION(BlueprintCallable)
	TSet<ARegionVolume*> GetAllRegionVolumes() const;
	
	//Region Gets
	UFUNCTION(BlueprintCallable, DisplayName = "Get Region (By Tag)", meta = (Categories = "Regions.Areas", AdvancedDisplay = 1))
	URegion* GetRegionByTag(FGameplayTag RegionTag, bool AllowParents = true) const;
	UFUNCTION(BlueprintCallable, DisplayName = "Get Region (By Location)", meta = (AdvancedDisplay = 1))
	URegion* GetRegionByLocation(FVector Location, ERegionTypes DesiredType = ERegionTypes::Room) const;
	UFUNCTION(BlueprintCallable, DisplayName = "Get Region (By Volume)", meta = (AdvancedDisplay = 2))
	URegion* GetRegionByVolume(FVector Location, FVector BoxExtent, ERegionTypes DesiredType = ERegionTypes::Room) const;
	UFUNCTION(BlueprintCallable, DisplayName = "Get Region (By Actor)")
	URegion* GetRegionByActor(AActor* Actor) const;
	UFUNCTION(BlueprintCallable, DisplayName = "Get Region (By Interface)")
	URegion* GetRegionByInterface(UObject* Object) const;
	UFUNCTION(BlueprintCallable, DisplayName = "Get Region (By Tracker)")
	URegion* GetRegionByTracker(URegionTracker* Tracker) const;
	UFUNCTION(BlueprintCallable, DisplayName = "Get Region (By Player State)")
	URegion* GetRegionByState(APlayerState* PlayerState) const;

	//Region Tag Gets
	UFUNCTION(BlueprintCallable, DisplayName = "Get Region Tag (By Location)", meta = (AdvancedDisplay = 1))
	FGameplayTag GetRegionTagByLocation(FVector Location, ERegionTypes DesiredType = ERegionTypes::Room) const;
	UFUNCTION(BlueprintCallable, DisplayName = "Get Region Tag (By Volume)", meta = (AdvancedDisplay = 2))
	FGameplayTag GetRegionTagByVolume(const FVector Location, const FVector BoxExtent, ERegionTypes DesiredType = ERegionTypes::Room) const;
	UFUNCTION(BlueprintCallable, DisplayName = "Get Region Tag (By Actor)")
	FGameplayTag GetRegionTagByActor(AActor* Actor) const;
	UFUNCTION(BlueprintCallable, DisplayName = "Get Region Tag (By Interface)")
	FGameplayTag GetRegionTagByInterface(UObject* Object) const;
	UFUNCTION(BlueprintCallable, DisplayName = "Get Region Tag (By Tracker)")
	FGameplayTag GetRegionTagByTracker(URegionTracker* Tracker) const;
	UFUNCTION(BlueprintCallable, DisplayName = "Get Region Tag (By Player State)")
	FGameplayTag GetRegionTagByState(APlayerState* PlayerState) const;
	
private:

	UPROPERTY(Transient)
	TMap<FGameplayTag, TObjectPtr<URegion>> RegionMap;

	UPROPERTY(Transient)
	FRegionModuleDefaults ModuleDefaults {};
	
	URegion* CreateNewRegion(FGameplayTag RegionTag);
	void CreateRegionModules(URegion* Region);
	void DestroyRegion(FGameplayTag RegionTag);
	void DestroyRegion(URegion* Region);

#pragma region Volumes
protected:

	friend ARegionVolume;

	//ONLY FOR REGION VOLUMES TO CALL
	void RegisterVolume(ARegionVolume* Volume);
	void DeregisterVolume(ARegionVolume* Volume);

	bool EnterRegionVolume(URegionTracker* Tracker, const ARegionVolume* Volume) const;
	bool ExitRegionVolume(URegionTracker* Tracker, const ARegionVolume* Volume) const;
#pragma endregion

#pragma region Editor
#if WITH_EDITOR
public:
	UFUNCTION()
	void ShowAllRegions();
	UFUNCTION()
	void HideAllRegions();
	UFUNCTION()
	void HighlightRegion(FGameplayTag Region, bool bHideChildren = true);
	
	UFUNCTION()
	void BakeRegions();
	UFUNCTION()
	void BakeRegion(FGameplayTag RegionTag);

	UFUNCTION()
	void ForceRefreshRegions();

private:
	void BakeRegion(URegion* Region);

#endif
#pragma endregion
};