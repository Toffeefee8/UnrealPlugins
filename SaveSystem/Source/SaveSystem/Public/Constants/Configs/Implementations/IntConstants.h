// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Constants/Configs/ConstantConfigs.h"
#include "IntConstants.generated.h"

UCLASS()
class SAVESYSTEM_API UIntConstants : public UConstantConfigs
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	static int GetConstantInt(FGameplayTag IntTag);

protected:
	virtual TMap<FGameplayTag, void*> GetArray() override;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ForceInlineRow))
	TMap<FGameplayTag, int> Ints;
};

inline int UIntConstants::GetConstantInt(FGameplayTag IntTag)
{
	return GetConstantData<int>(IntTag, StaticClass());
}

inline TMap<FGameplayTag, void*> UIntConstants::GetArray()
{
	return GetAsVoidPointer(Ints);

}
