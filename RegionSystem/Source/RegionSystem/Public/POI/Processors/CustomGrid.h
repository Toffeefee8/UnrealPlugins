#pragma once


#include "CoreMinimal.h"
#include "POITypeProcessor.h"
#include "Structs/DistanceData.h"
#include "CustomGrid.generated.h"

UCLASS()
class REGIONSYSTEM_API UCustomGrid : public UPOITypeProcessor
{
	GENERATED_BODY()

protected:

#if WITH_EDITOR
	
	virtual void CalculateRelevantPoints_Implementation(ARegionPOI* POI, TArray<FVector>& OutPoints, bool& OutIsLocal) const override;

	FVector2D GetBoxExtents(ARegionPOI* POI) const;

#endif

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "Settings")
	bool bUseBox = true;
	UPROPERTY(EditAnywhere, Category = "Settings", meta = (EditCondition = "bUseBox", EditConditionHides, AllowPreserveRatio))
	FVector2D BoxExtents = FVector2D(500);
	UPROPERTY(EditAnywhere, Category = "Settings", meta = (EditCondition = "!bUseBox", EditConditionHides))
	FDistanceData CircleRadius = FDistanceData(5, EDistanceType::Meters);
	
	UPROPERTY(EditAnywhere, Category = "Settings")
	FDistanceData NavPointSpacing = FDistanceData(3, EDistanceType::Meters);
	UPROPERTY(EditAnywhere, Category = "Settings")
	FDistanceData NavXYTolerance = FDistanceData(10, EDistanceType::Centimeters);
	UPROPERTY(EditAnywhere, Category = "Settings")
	FDistanceData MaxHeight = FDistanceData(2, EDistanceType::Meters);
	UPROPERTY(EditAnywhere, Category = "Settings")
	FDistanceData BottomOffset = FDistanceData(1, EDistanceType::Meters);
	UPROPERTY(EditAnywhere, Category = "Settings", meta = (ClampMin = 0, UIMin = 1, UIMax = 5))
	int32 HeightDivisions = 1;
#endif
};
