// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Constants/Configs/ConstantConfigs.h"
#include "StringConstants.generated.h"

UCLASS()
class SAVESYSTEM_API UStringConstants : public UConstantConfigs
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	static FString GetConstantString(FGameplayTag StringTag);

protected:

	virtual TMap<FGameplayTag, void*> GetArray() override;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ForceInlineRow))
	TMap<FGameplayTag, FString> Strings;
};

inline FString UStringConstants::GetConstantString(FGameplayTag StringTag)
{
	return GetConstantData<FString>(StringTag, StaticClass());
}

inline TMap<FGameplayTag, void*> UStringConstants::GetArray()
{
	return GetAsVoidPointer(Strings);
}
