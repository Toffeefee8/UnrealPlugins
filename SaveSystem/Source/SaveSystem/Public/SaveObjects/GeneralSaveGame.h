// Copyright Phoenix Dawn Development LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/SaveGame.h"
#include "Save/ObjectSerializationLibrary.h"
#include "GeneralSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class SAVESYSTEM_API UGeneralSaveGame : public USaveGame
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	void AddData(FGameplayTag Tag, FObjectData Data);
	UFUNCTION(BlueprintCallable)
	bool RemoveData(FGameplayTag Tag);
	UFUNCTION(BlueprintCallable)
	bool GetData(FGameplayTag Tag, FObjectData& Data);
	UFUNCTION(BlueprintCallable)
	TMap<FGameplayTag, FObjectData> GetAllData();

protected:

	UPROPERTY(SaveGame)
	TMap<FGameplayTag, FObjectData> TagDataPairs;
};
