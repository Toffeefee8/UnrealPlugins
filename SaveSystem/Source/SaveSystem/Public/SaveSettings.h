// Copyright Phoenix Dawn Development LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DeveloperSettings.h"
#include "Structs/TimeData.h"
#include "SaveSettings.generated.h"

class UConstantsDataAsset;
/**
 * 
 */
UCLASS(DefaultConfig, Config = Save, meta = (DisplayName = "Save Settings"))
class SAVESYSTEM_API USaveSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	static USaveSettings* Get();

	UPROPERTY(BlueprintReadOnly, Config, EditAnywhere, Category = "Save")
	FTimeData MinLoadTime = 3;
	UPROPERTY(BlueprintReadOnly, Config, EditAnywhere, Category = "Save")
	bool bLoadDataBeforeSave = false;

	UPROPERTY(BlueprintReadOnly, Config, EditAnywhere, Category = "Save", meta = (ForceInlineRow))
	TMap<FGameplayTag, FString> DefaultStringSolos;
	UPROPERTY(BlueprintReadOnly, Config, EditAnywhere, Category = "Save", meta = (ForceInlineRow))
	TMap<FGameplayTag, float> DefaultFloatSolos;

	UPROPERTY(BlueprintReadOnly, Config, EditAnywhere, Category = "Constants")
	TArray<TSoftObjectPtr<UConstantsDataAsset>> ConstantDefinitions {};
};
