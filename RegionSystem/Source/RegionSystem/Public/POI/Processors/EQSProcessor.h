#pragma once

#include "CoreMinimal.h"
#include "POI/Processors/POITypeProcessor.h"
#include "EQSProcessor.generated.h"

/**
 * 
 */
UCLASS()
class REGIONSYSTEM_API UEQSProcessor : public UPOITypeProcessor
{
	GENERATED_BODY()

public:

	UEQSProcessor()
	{
#if WITH_EDITOR
		bShowCustomSettings = false;
#endif
	}

#if WITH_EDITOR
	virtual void CalculateRelevantPoints_Implementation(ARegionPOI* POI, TArray<FVector>& OutPoints, bool& OutIsLocal) const override;
#endif

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UEnvQuery> EnvQuery = nullptr;
#endif
};
