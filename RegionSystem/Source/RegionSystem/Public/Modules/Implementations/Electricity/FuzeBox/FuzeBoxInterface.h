#pragma once

#include "CoreMinimal.h"

#include "UObject/Interface.h"
#include "FuzeBoxInterface.generated.h"

class UElectricityModule;

USTRUCT(BlueprintType)
struct FFuzeBoxData
{
	GENERATED_BODY()

public:
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ActivationDelay = 0.2f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bIndependent = false;
	
};

// This class does not need to be modified.
UINTERFACE()
class UFuzeBoxInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class REGIONSYSTEM_API IFuzeBoxInterface
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Regions|Modules|Electricity|FuzeBox")
	void GetFuzeBoxData(FFuzeBoxData& OutData);
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Regions|Modules|Electricity|FuzeBox")
	void OnReceiveModule(UElectricityModule* Module);
};
