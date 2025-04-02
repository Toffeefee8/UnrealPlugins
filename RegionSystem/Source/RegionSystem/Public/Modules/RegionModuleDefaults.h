#pragma once
#include "ObjectWrapper/ObjectWrapper.h"
#include "RegionModuleDefaults.generated.h"

class URegionModule;

USTRUCT(BlueprintType)
struct FRegionModuleDefaults
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
	TArray<TObjectPtr<URegionModule>> RegionModules;

	bool Contains(const TSubclassOf<URegionModule>& Class) const;
	URegionModule* GetRegionModule(const TSubclassOf<URegionModule>& Class) const;

	template<typename T>
	T* GetRegionModule();
};

template <typename T>
T* FRegionModuleDefaults::GetRegionModule()
{
	static_assert(std::is_base_of_v<URegionModule, T>, "T must inherit from URegionModule");
	return Cast<T>(GetRegionModule(T::StaticClass()));
}

UCLASS(Blueprintable)
class URegionModuleDefaultDefinition : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSubclassOf<URegionModule>> ImplementedModules;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRegionModuleDefaults Defaults;
};
