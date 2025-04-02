#pragma once

#include "CoreMinimal.h"
#include "DebugConfig.h"
#include "GameplayTagContainer.h"
#include "Engine/DeveloperSettings.h"
#include "DebugSettings.generated.h"

/**
 * 
 */
UCLASS(Config = EditorPerProjectUserSettings, GlobalUserConfig)
class DEBUGSYSTEM_API UDebugSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DevelopmentOnly))
	bool ShouldDebug(FGameplayTag DebugTagIn, EDebugDisplayType DebugTypeIn) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DevelopmentOnly))
	float GetDebugDuration(FGameplayTag DebugTagIn, EDebugDisplayType DebugTypeIn) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DevelopmentOnly))
	FDebugConfig GetShownDebugConfig(FGameplayTag DebugTag) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DevelopmentOnly))
	FDebugConfig GetSuppressedDebugConfig(FGameplayTag DebugTag) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DevelopmentOnly))
	bool SuppressAllDebugs() const;
	
	static UDebugSettings* Get();

	virtual FName GetCategoryName() const override;
	
private:
	
	bool ShouldShowDebugType(EDebugDisplayType DebugTypeIn) const;

protected:
	UPROPERTY(Config, EditAnywhere, Category = "Debug Settings")
	bool bSuppressAllDebugs = false;

	//Custom Suppressors
	UPROPERTY(Config, EditAnywhere, Category = "Debug Settings|AllowedDebugTypes")
	bool bAllowLog = true;
	UPROPERTY(Config, EditAnywhere, Category = "Debug Settings|AllowedDebugTypes")
	bool bAllowPrint = true;
	UPROPERTY(Config, EditAnywhere, Category = "Debug Settings|AllowedDebugTypes")
	bool bAllowVisual = true;
	UPROPERTY(Config, EditAnywhere, Category = "Debug Settings|AllowedDebugTypes")
	bool bAllowSound = true;

	//Defaults
	UPROPERTY(Config, EditAnywhere, Category = "Debug Settings|Defaults")
	bool bLogByDefault = true;
	UPROPERTY(Config, EditAnywhere, Category = "Debug Settings|Defaults")
	bool bPrintByDefault = false;
	UPROPERTY(Config, EditAnywhere, Category = "Debug Settings|Defaults")
	bool bVisualByDefault = false;
	UPROPERTY(Config, EditAnywhere, Category = "Debug Settings|Defaults")
	bool bSoundByDefault = false;
	
	UPROPERTY(Config, EditAnywhere, Category = "Debug Settings", meta = (ForceInlineRow))
	TMap<FGameplayTag, FDebugConfig> ShownDebugConfigs;
	
	UPROPERTY(Config, EditAnywhere, Category = "Debug Settings", meta = (ForceInlineRow))
	TMap<FGameplayTag, FDebugConfig> SuppressedDebugConfigs;


};
