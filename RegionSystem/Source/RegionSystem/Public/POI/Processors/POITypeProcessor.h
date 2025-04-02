#pragma once

#include "CoreMinimal.h"
#include "POI/RegionPOI.h"
#include "Structs/DistanceData.h"
#include "UObject/Object.h"
#include "POITypeProcessor.generated.h"

class ARegionPOI;
/**
 * 
 */
UCLASS(Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced, Abstract)
class REGIONSYSTEM_API UPOITypeProcessor : public UObject
{
	GENERATED_BODY()

public:
	virtual bool IsEditorOnly() const override { return true; }

#if WITH_EDITOR

	//Do not modify
	UFUNCTION(BlueprintCallable, Category = "POI")
	FLocationCache GetPOIRelevantPoints(ARegionPOI* POI) const;

protected:

	//Overwrite this
	UFUNCTION(BlueprintNativeEvent, Category = "POI")
	void CalculateRelevantPoints(ARegionPOI* POI, TArray<FVector>& OutPoints, bool& OutIsLocal) const;
	
	void MakeArrayLocal(TArray<FVector>& Array, const FVector& Origin) const;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|LineOfSight", meta = (EditCondition = bShowCustomSettings, EditConditionHides))
	bool bLineOfSight = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|LineOfSight", meta = (EditCondition = bShowCustomSettings, EditConditionHides))
	FDistanceData HeightOffset = FDistanceData(1, EDistanceType::Meters);
	UPROPERTY()
	bool bShowCustomSettings = true;
#endif

#endif
};

