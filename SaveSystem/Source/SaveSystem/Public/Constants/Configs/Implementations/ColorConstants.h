#pragma once

#include "CoreMinimal.h"
#include "Constants/Configs/ConstantConfigs.h"
#include "ColorConstants.generated.h"

UCLASS()
class SAVESYSTEM_API UColorConstants : public UConstantConfigs
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static FColor GetConstantColor(FGameplayTag ColorTag);

protected:
	
	virtual TMap<FGameplayTag, void*> GetArray() override;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ForceInlineRow))
	TMap<FGameplayTag, FColor> Colors;
};

inline FColor UColorConstants::GetConstantColor(FGameplayTag ColorTag)
{
	return GetConstantData<FColor>(ColorTag, StaticClass());
}

inline TMap<FGameplayTag, void*> UColorConstants::GetArray()
{
	return GetAsVoidPointer(Colors);
}
