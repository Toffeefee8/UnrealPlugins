// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Constants/Configs/ConstantConfigs.h"
#include "WorldConstants.generated.h"

UCLASS()
class SAVESYSTEM_API UWorldConstants : public UConstantConfigs
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	static TSoftObjectPtr<UWorld> GetConstantWorld(FGameplayTag WorldTag);

protected:
	
	virtual TMap<FGameplayTag, void*> GetArray() override;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ForceInlineRow))
	TMap<FGameplayTag, TSoftObjectPtr<UWorld>> Worlds;
};

inline TSoftObjectPtr<UWorld> UWorldConstants::GetConstantWorld(FGameplayTag WorldTag)
{
	return GetConstantData<TSoftObjectPtr<UWorld>>(WorldTag, StaticClass());
}

inline TMap<FGameplayTag, void*> UWorldConstants::GetArray()
{
	return GetAsVoidPointer(Worlds);
}
