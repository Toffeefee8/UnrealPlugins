// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Constants/Configs/ConstantConfigs.h"
#include "FloatConstants.generated.h"

UCLASS()
class SAVESYSTEM_API UFloatConstants : public UConstantConfigs
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	static float GetConstantTagToText(FGameplayTag FloatTag);

protected:
	virtual TMap<FGameplayTag, void*> GetArray() override;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ForceInlineRow))
	TMap<FGameplayTag, float> Floats;
};

inline float UFloatConstants::GetConstantTagToText(FGameplayTag FloatTag)
{
	return GetConstantData<float>(FloatTag, StaticClass());
}

inline TMap<FGameplayTag, void*> UFloatConstants::GetArray()
{
	return GetAsVoidPointer(Floats);

}
