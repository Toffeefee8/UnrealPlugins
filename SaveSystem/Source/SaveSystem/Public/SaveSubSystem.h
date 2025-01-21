// Copyright Phoenix Dawn Development LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SaveTags.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SaveSubSystem.generated.h"

class UConstantConfigs;
class UConstantsDataAsset;
class USoloSaveGame;
class USaveGame;
class UGeneralSaveGame;
struct FGameplayTag;

DECLARE_LOG_CATEGORY_EXTERN(LogSaveSystem, Log, All);
/**
 * 
 */
UCLASS()
class SAVESYSTEM_API USaveSubSystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:

	static USaveSubSystem* Get();

	//Overrides
	virtual UWorld* GetWorld() const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
	void Save(UObject* WorldContextObject, UPARAM(meta = (Categories = "Save.Type")) FGameplayTag SaveTag);
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
	void Load(UObject* WorldContextObject, UPARAM(meta = (Categories = "Save.Type")) FGameplayTag SaveTag);

	//Requests
	UFUNCTION(BlueprintCallable)
	void RequestSaveForObject(UObject* Object);
	UFUNCTION(BlueprintCallable)
	void RequestLoadForObject(UObject* Object);
	UFUNCTION(BlueprintCallable)
	void RequestClearForObject(UObject* Object);

	UFUNCTION(BlueprintCallable)
	void RequestSaveForObjectBySaveType(UObject* Object, UPARAM(meta = (Categories = "Save.Type")) FGameplayTag SaveType);
	UFUNCTION(BlueprintCallable)
	void RequestLoadForObjectBySaveType(UObject* Object, UPARAM(meta = (Categories = "Save.Type")) FGameplayTag SaveType);
	UFUNCTION(BlueprintCallable)
	void RequestClearForObjectBySaveType(UObject* Object, UPARAM(meta = (Categories = "Save.Type")) FGameplayTag SaveType);

#pragma region Solos
	UFUNCTION(BlueprintCallable)
	void SaveSoloFloat(FGameplayTag Tag, float Value);
	UFUNCTION(BlueprintCallable)
	bool LoadSoloFloat(FGameplayTag Tag, float& Value) const;
	UFUNCTION(BlueprintCallable)
	void ClearSoloFloat(FGameplayTag Tag);

	UFUNCTION(BlueprintCallable)
	void SaveSoloString(FGameplayTag Tag, FString Value);
	UFUNCTION(BlueprintCallable)
	bool LoadSoloString(FGameplayTag Tag, FString& Value);
	UFUNCTION(BlueprintCallable)
	void ClearSoloString(FGameplayTag Tag);

	static USoloSaveGame* LoadSolos();
	static bool SaveSolos(USoloSaveGame* SoloToSave);
	static FString GetSoloSaveName();
#pragma endregion
	
#pragma region Constants
	
	TArray<UConstantConfigs*> GetAllConfigsOfType(const TSubclassOf<UConstantConfigs>& Class);

protected:

	void LoadAllConstants();
	void VerifyAllConstants();

	UPROPERTY(Transient)
	TArray<UConstantsDataAsset*> Constants {};
	
#pragma endregion
	
protected:

	static FString RemoveParentTagsFromTag(FGameplayTag SourceTag, FGameplayTag ParentToRemove);

	FString GetFullSaveName(const FGameplayTag& TypeTag, const FGameplayTag& SaveTag) const;
	TArray<UObject*> GetAllSaveObjects(const UObject* WorldContextObject) const;

	bool CanSaveByType(UObject* Object, FGameplayTag SaveType) const;
	bool GetSaveIDs(UObject* Object, FGameplayTag& SaveTag, TSubclassOf<USaveGame>& SaveClass) const;
	FGameplayTagContainer GetSupportedSaveTypes(UObject* Object) const;

	void SaveObject(UObject* Object, FGameplayTag SaveType) const;
	void LoadObject(UObject* Object, FGameplayTag SaveType) const;
	void ClearObject(UObject* Object, FGameplayTag SaveType) const;

	UPROPERTY(Transient)
	mutable USoloSaveGame* LoadedSolos = nullptr;

	void ReevaluateSolosForSave() const;
	void ReevaluateLoadedSolos() const;
	void VerifySoloDefaults() const;
	
	static FString SoloSaveDirectory;
	static FString SoloSaveName;
};
