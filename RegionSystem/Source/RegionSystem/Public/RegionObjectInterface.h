#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Structs/RegionTypes.h"
#include "UObject/Interface.h"
#include "RegionObjectInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class URegionObject : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class REGIONSYSTEM_API IRegionObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Regions", meta = (Categories = "Regions.Areas"))
	FGameplayTag GetRegionTag() const;
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Regions", meta = (Categories = "Regions.Areas"))
	void ForceSetRegion(FGameplayTag NewRegion);
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Regions", meta = (Categories = "Regions.Areas"))
	void GetCheckData(FVector& OutCheckLocation, ERegionTypes& OutDesiredType, bool& bOutDisableRuntimeChecks) const;
};
