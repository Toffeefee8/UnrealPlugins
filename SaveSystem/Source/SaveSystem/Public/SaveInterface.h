// Copyright Phoenix Dawn Development LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SaveTags.h"
#include "UObject/Interface.h"
#include "SaveInterface.generated.h"

class USoloSaveGame;
class USaveGame;

UINTERFACE()
class USaveInterface : public UInterface
{
	GENERATED_BODY()
};

class SAVESYSTEM_API ISaveInterface
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Save")
	void OnSave(USaveGame* SaveObject, USoloSaveGame* SoloSaveGame, FGameplayTag SaveType);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Save")
	void OnLoad(USaveGame* SaveObject, USoloSaveGame* SoloSaveGame, FGameplayTag SaveType, bool bValid);

	//Must Implement to specify data
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Save")
	void GetSaveIDs(UPARAM(meta = (Categories = "Save.IDs")) FGameplayTag& SaveTag, TSubclassOf<USaveGame>& SaveClass) const;

	//Must Implement (Use SaveTags::Types::World for world saves and SaveTags::Types::Player for player saves)
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Save")
	void GetSupportedSaveTypes(UPARAM(meta = (Categories = "Save.Type")) FGameplayTagContainer& SupportedTags) const;
};
