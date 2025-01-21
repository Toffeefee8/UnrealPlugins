// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Constants/Configs/ConstantConfigs.h"
#include "TextConstants.generated.h"

UCLASS()
class SAVESYSTEM_API UFTextConstants : public UConstantConfigs
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	static FText GetConstantText(FGameplayTag FTextTag);

protected:
	virtual TMap<FGameplayTag, void*> GetArray() override;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ForceInlineRow, MultiLine))
	TMap<FGameplayTag, FText> FTexts;
};

inline FText UFTextConstants::GetConstantText(FGameplayTag FTextTag)
{
	return GetConstantData<FText>(FTextTag, StaticClass());
}

inline TMap<FGameplayTag, void*> UFTextConstants::GetArray()
{
	return GetAsVoidPointer(FTexts);

}
