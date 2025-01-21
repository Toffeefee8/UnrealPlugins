// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Constants/Configs/ConstantConfigs.h"
#include "LinearColorConstants.generated.h"

UCLASS()
class SAVESYSTEM_API ULinearColorConstants : public UConstantConfigs
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	static FLinearColor GetConstantLinearColor(FGameplayTag LinearColorTag);

protected:
	
	virtual TMap<FGameplayTag, void*> GetArray() override;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ForceInlineRow))
	TMap<FGameplayTag, FLinearColor> LinearColors;
};

inline FLinearColor ULinearColorConstants::GetConstantLinearColor(FGameplayTag LinearColorTag)
{
	return GetConstantData<FLinearColor>(LinearColorTag, StaticClass());
}

inline TMap<FGameplayTag, void*> ULinearColorConstants::GetArray()
{
	return GetAsVoidPointer(LinearColors);
}
