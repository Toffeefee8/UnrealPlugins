#pragma once

#include "CoreMinimal.h"
#include "DebugSettings.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DebugFunctionLibrary.generated.h"

#define SHOULD_DEBUG(Tag, DebugType) \
(UDebugFunctionLibrary::ShouldDebug(Tag, DebugType))

#define DEBUG_LOG(LogType, LogCategory, Text, DebugTag) \
if (SHOULD_DEBUG(DebugTag, EDebugDisplayType::Log))    \
{                                               \
	UE_LOG(LogType, LogCategory, TEXT("[%s] %s"), *DebugTag.GetTag().ToString(), *FString(Text)); \
}

#define DEBUG_PRINT_TO_SCREEN(Key, Color, Text, DebugTag)               \
if (SHOULD_DEBUG(DebugTag, EDebugDisplayType::Print))                                    \
{                                                                                 \
	if (GEngine)                                                                  \
	{                                                                             \
		GEngine->AddOnScreenDebugMessage(Key, UDebugFunctionLibrary::GetDebugDuration(DebugTag, EDebugDisplayType::Print), Color,                 \
		FString::Printf(TEXT("[%s] %s"), *DebugTag.GetTag().ToString(), *FString(Text))); \
	}                                                                             \
}

#define DEBUG_SIMPLE(LogType, LogCategory, Color, Text, DebugTag) \
	DEBUG_LOG(LogType, LogCategory, Text, DebugTag) \
	DEBUG_PRINT_TO_SCREEN(-1, Color, Text, DebugTag)

UCLASS()
class DEBUGSYSTEM_API UDebugFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static bool ShouldDebug(FGameplayTag DebugTag, EDebugDisplayType DebugType);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static float GetDebugDuration(FGameplayTag DebugTag, EDebugDisplayType DebugType);

	UFUNCTION(BlueprintCallable, meta=(WorldContext="WorldContextObject", CallableWithoutWorldContext, Keywords = "log print", AdvancedDisplay = "3", DevelopmentOnly), Category="Development")
	static void DebugString(
		UObject* WorldContextObject,
		const FString& InString= FString("Hello"),
		const FGameplayTag InDebugTag = FGameplayTag(),
		// const bool bPrintToScreen = true,
		// const bool bPrintToLog = true,
		const FLinearColor TextColor = FLinearColor(0, 0.66, 1),
		// const float Duration = 2,
		const FName Key = NAME_None);

	UFUNCTION(BlueprintCallable, meta=(WorldContext="WorldContextObject", CallableWithoutWorldContext, Keywords = "error throw exception", AdvancedDisplay = "2", DevelopmentOnly), Category="Development")
	static void DebugError(
		const UObject* WorldContextObject,
		const FString& ErrorMessage= FString("Error"),
		const bool bPrintToScreen = true,
		const bool bPrintToLog= true);

	UFUNCTION(BlueprintCallable, Category="Debug|Rendering", meta=(WorldContext="WorldContextObject", DevelopmentOnly))
	static void DebugDrawSphere(
		const UObject* WorldContextObject,
		const FGameplayTag DebugTag,
		const FVector Center,
		float Radius = 100.f,
		int32 Segments = 12,
		FLinearColor LineColor = FLinearColor::White,
		// float Duration = 0.f,
		float Thickness = 0.f);
	
	UFUNCTION(BlueprintCallable, Category="Debug|Rendering", meta=(WorldContext="WorldContextObject", DevelopmentOnly))
	static void DebugDrawArrow(
		const UObject* WorldContextObject,
		const FGameplayTag DebugTag,
		const FVector Start,
		const FVector End,
		int32 Segments = 12,
		FLinearColor LineColor = FLinearColor::White,
		// float Duration = 0.f,
		float Thickness = 0.f);

	UFUNCTION(BlueprintCallable, Category="Debug|Rendering", meta=(WorldContext="WorldContextObject", DevelopmentOnly))
	static void DebugDrawString(
		const UObject* WorldContextObject,
		const FGameplayTag DebugTag,
		const FVector TextLocation,
		const FString& Text,
		AActor* TestBaseActor = nullptr,
		FLinearColor TextColor = FLinearColor::White,
		float Duration = 0.f);

	
	
};
