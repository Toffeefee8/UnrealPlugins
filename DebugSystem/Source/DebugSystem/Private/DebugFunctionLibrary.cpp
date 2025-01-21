// Fill out your copyright notice in the Description page of Project Settings.


#include "DebugFunctionLibrary.h"
#include "DebugSettings.h"
#include "Kismet/KismetSystemLibrary.h"

bool UDebugFunctionLibrary::ShouldDebug(const FGameplayTag DebugTag, const EDebugDisplayType DebugType)
{
#if WITH_EDITOR
	return GetDefault<UDebugSettings>() && GetDefault<UDebugSettings>()->ShouldDebug(DebugTag, DebugType);
#else
	switch (DebugType)
	{
		case EDebugDisplayType::Log:
			return true;
		case EDebugDisplayType::Print:
		case EDebugDisplayType::Visual:
		case EDebugDisplayType::Sound:
			return false;
		default:
			return false;
	}
#endif
}

float UDebugFunctionLibrary::GetDebugDuration(FGameplayTag DebugTag, EDebugDisplayType DebugType)
{
	if (GetDefault<UDebugSettings>())
		return GetDefault<UDebugSettings>()->GetDebugDuration(DebugTag, DebugType);

	return 1;
}

void UDebugFunctionLibrary::DebugString(
	UObject* WorldContextObject,
	const FString& InString,
	const FGameplayTag InDebugTag,
	// const bool bPrintToScreen,
	// const bool bPrintToLog,
	const FLinearColor TextColor,
	// const float Duration,
	const FName Key) 
{
	const bool bShouldPrintToScreen = /*bPrintToScreen &&*/ ShouldDebug(InDebugTag,EDebugDisplayType::Print);
	const bool bShouldPrintToLog = /*bPrintToLog &&*/ ShouldDebug(InDebugTag,EDebugDisplayType::Log);
	if (!bShouldPrintToScreen && !bShouldPrintToLog)
		return;

	FString DebugTagString = InDebugTag.ToString();
	DebugTagString.RemoveFromStart("Debug.");
	const FString StringToPrint = "[" + DebugTagString + "] " + InString;
	
	UKismetSystemLibrary::PrintString(
		WorldContextObject,
		StringToPrint,
		bShouldPrintToScreen,
		bShouldPrintToLog,
		TextColor,
		GetDebugDuration(InDebugTag, EDebugDisplayType::Print),
		Key);
}

void UDebugFunctionLibrary::DebugError(const UObject* WorldContextObject, const FString& ErrorMessage, const bool bPrintToScreen,
	const bool bPrintToLog)
{
		if (const UDebugSettings* Settings = GetDefault<UDebugSettings>())
		{
			if (Settings->SuppressAllDebugs())
				return;
		}
	
		if (!IsValid(WorldContextObject))
		{
			UE_LOG(LogTemp, Error, TEXT("Sf_ThrowError called with invalid WorldContextObject."));
			return;
		}

		FString ContextObjectName = WorldContextObject->GetName();
		FString TimeStamp = FDateTime::Now().ToString(TEXT("%H:%M:%S"));

		const FString StringToPrint = FString::Printf(
			TEXT("[ERROR] [%s] [Context: %s] %s"),
			*TimeStamp,
			*ContextObjectName,
			*ErrorMessage
		);
	
		constexpr float Duration = 400.f;
		UKismetSystemLibrary::PrintString(WorldContextObject, StringToPrint, bPrintToScreen, bPrintToLog, FColor::Red, Duration);
}

void UDebugFunctionLibrary::DebugDrawSphere(
	const UObject* WorldContextObject,
	const FGameplayTag DebugTag,
	const FVector Center,
	const float Radius,
	const int32 Segments,
	const FLinearColor LineColor,
	// const float Duration,
	const float Thickness)
{
	if (ShouldDebug(DebugTag, EDebugDisplayType::Visual))
		UKismetSystemLibrary::DrawDebugSphere(
			WorldContextObject,
			Center,
			Radius,
			Segments,
			LineColor,
			GetDebugDuration(DebugTag, EDebugDisplayType::Visual),
			Thickness);
}

void UDebugFunctionLibrary::DebugDrawArrow(const UObject* WorldContextObject, const FGameplayTag DebugTag, const FVector Start, const FVector End, 
int32 Segments, FLinearColor LineColor, float Thickness)
{
	if (ShouldDebug(DebugTag, EDebugDisplayType::Visual))
		UKismetSystemLibrary::DrawDebugArrow(
			WorldContextObject,
			Start,
			End,
			Segments,
			LineColor,
			GetDebugDuration(DebugTag, EDebugDisplayType::Visual),
			Thickness);
}

void UDebugFunctionLibrary::DebugDrawString(const UObject* WorldContextObject, const FGameplayTag DebugTag, const FVector TextLocation,
                                            const FString& Text, AActor* TestBaseActor, const FLinearColor TextColor, const float Duration)
{
	if (ShouldDebug(DebugTag, EDebugDisplayType::Visual))
		UKismetSystemLibrary::DrawDebugString(WorldContextObject,TextLocation,Text,TestBaseActor,TextColor,Duration);
}

