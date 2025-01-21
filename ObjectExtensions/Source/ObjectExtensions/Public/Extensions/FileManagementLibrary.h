// Copyright Phoenix Dawn Development LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FileManagementLibrary.generated.h"

/**
 * 
 */

UCLASS(Blueprintable)
class OBJECTEXTENSIONS_API UFileManagementLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Save Management")
	static TArray<FString> GetSavedFiles(FString Subdirectory);

	UFUNCTION(BlueprintCallable, Category = "Save Management")
	static void GetFiles(TArray<FString>& FoundFiles, const FString Directory, const FString FileExtension);

	UFUNCTION(BlueprintCallable, Category = "Save Management")
	static void DeleteDirectory(const FString Directory);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Save Management")
	static FString GetSavePath(bool FullPath = false);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "String")
	static FString GetUppercaseString(FString String);
};
