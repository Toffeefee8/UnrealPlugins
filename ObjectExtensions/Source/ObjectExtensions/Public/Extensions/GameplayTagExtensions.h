#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagExtensions.generated.h"

/**
 * 
 */
UCLASS()
class OBJECTEXTENSIONS_API UGameplayTagExtensions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = GameplaytagExtentions)
	static FGameplayTag CreateTagFromParts(const TArray<FString>& Parts, int32 NumDetail = -1);
	UFUNCTION(BlueprintCallable, Category = GameplaytagExtentions)
	static TArray<FString> CreatePartsFromTag(FGameplayTag Tag);
	UFUNCTION(BlueprintCallable, Category = GameplaytagExtentions)
	static FGameplayTag GetMostDetailedTag(FGameplayTagContainer Tags);
	UFUNCTION(BlueprintCallable, Category = GameplaytagExtentions)
	static int32 GetTagDepth(FGameplayTag Tag);
	UFUNCTION(BlueprintCallable, Category = GameplaytagExtentions)
	static FGameplayTag RemoveTagDepth(FGameplayTag Tag, uint8 DepthToRemove = 1);
	UFUNCTION(BlueprintCallable, Category = GameplaytagExtentions)
	static FGameplayTag GetParentGameplayTag(FGameplayTag Tag);
	UFUNCTION(BlueprintCallable, Category = GameplaytagExtentions)
	static FName RemoveParentTagFromName(FGameplayTag Tag, FGameplayTag TagToRemove);
	UFUNCTION(BlueprintCallable, Category = GameplaytagExtentions)
	static FGameplayTagContainer InverseFilter(FGameplayTagContainer Container, FGameplayTagContainer SecondContainer);
	UFUNCTION(BlueprintCallable, Category = GameplaytagExtentions)
	static FGameplayTagContainer GetExclusiveTags(FGameplayTagContainer Source, FGameplayTagContainer ToRemove);

};