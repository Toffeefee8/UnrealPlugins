#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ConstantsDataAsset.generated.h"

class UConstantConfigs;
/**
 * 
 */
UCLASS()
class SAVESYSTEM_API UConstantsDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:

	UFUNCTION()
	TArray<UConstantConfigs*> GetAllConfigsOfType(const TSubclassOf<UConstantConfigs>& Class) const;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Instanced)
	TArray<TObjectPtr<UConstantConfigs>> Configs {};
};
