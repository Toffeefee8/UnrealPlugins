#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "RegionModule.generated.h"

class URegion;

UCLASS(Blueprintable, BlueprintType, DefaultToInstanced, EditInlineNew, Abstract)
class REGIONSYSTEM_API URegionModule : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, Category = "Regions|Modules")
	void StartModule(URegion* OwningRegion);
	UFUNCTION(BlueprintNativeEvent, Category = "Regions|Modules")
	void RefreshModule();
	UFUNCTION(BlueprintNativeEvent, Category = "Regions|Modules")
	void EndModule();

	UFUNCTION(BlueprintNativeEvent, Category = "Regions|Modules")
	void NewParent(URegion* OldParentRegion, URegion* NewParentRegion);
	UFUNCTION(BlueprintNativeEvent, Category = "Regions|Modules")
	void NewChild(URegion* NewChild);

	UFUNCTION(BlueprintCallable, Category = "Regions|Modules")
	URegion* GetOwningRegion() const;
	UFUNCTION(BlueprintCallable, Category = "Regions|Modules")
	FGameplayTag GetOwningRegionTag() const;

	UFUNCTION(BlueprintCallable, Category = "Regions|Modules")
	URegion* GetParentRegion() const;
	UFUNCTION(BlueprintCallable, Category = "Regions|Modules")
	TSet<URegion*> GetChildRegions() const;

	UFUNCTION(BlueprintCallable, Category = "Regions|Modules")
	bool HasParentRegion() const;
	UFUNCTION(BlueprintCallable, Category = "Regions|Modules")
	bool HasRegionModule(TSubclassOf<URegionModule> ModuleClass, bool AllowParentSearch) const;
	
	UFUNCTION(BlueprintCallable, Category = "Regions|Modules")
	bool HasParentRegionModule(TSubclassOf<URegionModule> ModuleClass, bool AllowParentSearch) const;
	template<typename T>
	bool HasParentRegionModule(bool AllowParentSearch = true) const;
	
	UFUNCTION(BlueprintCallable, Category = "Regions|Modules", meta = (DeterminesOutputType = "ModuleClass"))
	URegionModule* GetRegionModule(TSubclassOf<URegionModule> ModuleClass, bool AllowParentSearch = true) const;
	template<typename T>
	T* GetRegionModule(bool AllowParentSearch = true) const;

	UFUNCTION(BlueprintCallable, Category = "Regions|Modules", meta = (DeterminesOutputType = "ModuleClass"))
	URegionModule* GetParentRegionModule(TSubclassOf<URegionModule> ModuleClass, bool AllowParentSearch = true) const;
	template<typename T>
	T* GetParentRegionModule(bool AllowParentSearch = true) const;
};

template <typename T>
bool URegionModule::HasParentRegionModule(bool AllowParentSearch) const
{
	return HasParentRegionModule(T::StaticClass(), AllowParentSearch);
}

template <typename T>
T* URegionModule::GetRegionModule(bool AllowParentSearch) const
{
	return Cast<T>(GetRegionModule(T::StaticClass(), AllowParentSearch));
}

template <typename T>
T* URegionModule::GetParentRegionModule(bool AllowParentSearch) const
{
	return Cast<T>(GetParentRegionModule(T::StaticClass(), AllowParentSearch));
}
