#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Modules/RegionModuleDefaults.h"
#include "Modules/Implementations/Electricity/Consumer/ElectricityConsumerType.h"
#include "Structs/TimeData.h"
#include "RegionSettings.generated.h"

/**
 * 
 */
UCLASS(Config = Regions, DefaultConfig, meta = (DisplayName = "Region Settings"))
class REGIONSYSTEM_API URegionSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	static const URegionSettings* Get();

	static bool GetDefaultModuleFuzeState();
	static bool IsTypeEnabledByDefault(EElectricityConsumerType ElectricityConsumer);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Config)
	TSet<TSubclassOf<URegionModule>> ImplementedModuleClasses;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Config)
	bool bAutoBakePOIs = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Config)
	bool bForceVolumeBoxChecks = false;

	//Electricity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Config, Category = "Electricity")
	FTimeData DefaultActivationDelay = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Config, Category = "Electricity")
	bool bDefaultModuleFuzeState = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Config, Category = "Electricity")
	bool bDefaultConsumerTypeState = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Config, Category = "Electricity")
	TMap<EElectricityConsumerType, bool> TypeDefaults = {};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Config, Category = "Electricity|FuzeBox")
	bool bDeactivateAllTypesOnBreak = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Config, Category = "Electricity|FuzeBox")
	bool bDeactivateModuleOnBreak = true;
};
