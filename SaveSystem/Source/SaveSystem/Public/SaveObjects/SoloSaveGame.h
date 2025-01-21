// Copyright Phoenix Dawn Development LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/SaveGame.h"
#include "SoloSaveGame.generated.h"

class USaveSubSystem;

UCLASS()
class SAVESYSTEM_API USoloSaveGame : public USaveGame
{
	GENERATED_BODY()

public:

	//Floats
	UFUNCTION(BlueprintCallable)
	void SaveFloat(FGameplayTag Tag, float Value);
	UFUNCTION(BlueprintCallable)
	bool LoadFloat(FGameplayTag Tag, float& Value) const;
	UFUNCTION(BlueprintCallable)
	void ClearFloat(FGameplayTag Tag);

	//Strings
	UFUNCTION(BlueprintCallable)
	void SaveString(FGameplayTag Tag, FString Value);
	UFUNCTION(BlueprintCallable)
	bool LoadString(FGameplayTag Tag, FString& Value) const;
	UFUNCTION(BlueprintCallable)
	void ClearString(FGameplayTag Tag);

protected:
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, meta = (ForceInlineRow))
	TMap<FGameplayTag, float> FloatValues;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, meta = (ForceInlineRow))
	TMap<FGameplayTag, FString> StringValues;

	friend USaveSubSystem;

	UPROPERTY(Transient)
	mutable bool bDirty = false;
};
