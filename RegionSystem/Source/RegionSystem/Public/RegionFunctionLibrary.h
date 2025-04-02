#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Structs/RegionTypes.h"
#include "RegionFunctionLibrary.generated.h"

class ARegionVolume;
/**
 * 
 */
UCLASS()
class REGIONSYSTEM_API URegionFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Regions")
	static ERegionTypes GetRegionTypeByTag(FGameplayTag RegionTag);
	UFUNCTION(BlueprintCallable, Category = "Regions")
	static int32 GetRegionDepthByTag(FGameplayTag RegionTag);
};
