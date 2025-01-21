// Copyright Phoenix Dawn Development LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EngineUtils.h"
#include "GameplayTagContainer.h"
#include "Engine/LevelStreamingDynamic.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "HelperFunctionLibrary.generated.h"

UENUM(Blueprintable)
enum ERelevanceType
{
	LocalServer,
	Server,
	Local,
	Simulated
};

DEFINE_LOG_CATEGORY_STATIC(LogHelperFunctionLibrary,Display,Display);

UCLASS()
class OBJECTEXTENSIONS_API UHelperFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static ERelevanceType GetRelevancePawn(APawn* Pawn);
	static ERelevanceType GetRelevanceController(AController* Actor);

	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext), Category = "Replication")
	static bool GetLocalAuthority(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static int ConvertEnumToInteger(uint8 Byte);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	
	static bool BitmaskContainsEnum(uint8 EnumByte, int Bitmask);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Math")
	static float ClampAbsoluteValue(float Value, float MinValue, float MaxValue);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Math")
	static float MaxAbsoluteValue(float ValueToUseAbsolute, float B);

	UFUNCTION(BlueprintCallable, Category = "Actor", meta=(DeterminesOutputType="ActorClass", DynamicOutputParam="OutActors"))
	static  void GetAllActorsOfClassFromStreamLevel(
	ULevelStreaming* LevelStreaming,
	TSubclassOf<AActor> ActorClass,
	TArray<AActor *> & OutActors);

	UFUNCTION(BlueprintCallable, Category = "Actor", meta=(DeterminesOutputType="ActorClass", DynamicOutputParam="OutActors"))
	static void GetAllActorsOfClassFromLevel(
	ULevel* Level,
	TSubclassOf<AActor> ActorClass,
	TArray<AActor *> & OutActors);

	template <typename ComponentType>
	static void GetAllComponentsOfType(UWorld* World, TArray<ComponentType*>& OutComponents);

	UFUNCTION(BlueprintCallable,BlueprintPure, Category = "Actor")
	static bool IsActorFullyInBox2D(const AActor* Actor, const FBox2f& Box2D);

	UFUNCTION(BlueprintCallable,BlueprintPure, Category = "LevelStreaming")
	static TSoftObjectPtr<UWorld> GetWorldSoftObject(ULevel* Level);

	static int GetMaxDecimalPlaces(TArray<float> FloatArray);

	static  bool IsRoundNumber(float Number);

	UFUNCTION(BlueprintCallable, Category = "Actor")
	static void DestroyAllValidActors(TArray<AActor*> Actors);

	UFUNCTION()
	static TArray<FGameplayTag> GetFilteredGameplayTags(const FString& Prefix);

	static void DestroyAllValidActors(
	TArray<AActor*> Actors,
	TFunction<bool(AActor*)> AdditionalConditions);

	static void UnloadAllValidLevels(
		TArray<ULevelStreamingDynamic*> Levels,
		TFunction<bool(ULevelStreamingDynamic*)> AdditionalConditions = nullptr);

	template <typename T>
	static void GetAllActorsOfClassFromStreamLevel(
	ULevelStreaming* LevelStreaming,
	TArray<T*>& OutActors);

	template <typename T>
	static  void GetAllActorsWithInterfaceFromStreamLevel(
	ULevelStreaming* LevelStreaming,
	TArray<AActor*>& OutActors);

	template <typename T>
	static void GetAllActorsWithComponentFromStreamLevel(
		ULevelStreaming* LevelStreaming,
		TArray<AActor*>& OutActors);
	
};

template <typename ComponentType>
void UHelperFunctionLibrary::GetAllComponentsOfType(UWorld* World, TArray<ComponentType*>& OutComponents)
{
	OutComponents.Empty();
	for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
	{
		AActor* Actor = *ActorItr;
		
		TArray<ComponentType*> Components;
		Actor->GetComponents(Components);
		OutComponents.Append(Components);
	}
}

template <typename T>
void UHelperFunctionLibrary::GetAllActorsOfClassFromStreamLevel(ULevelStreaming* LevelStreaming, TArray<T*>& OutActors)
{
	static_assert(
		TIsDerivedFrom<T, AActor>::Value || std::is_same<T, AActor>::value,
		"ImportActorClass must be a subclass of AActor or AActor itself."
	);
	
	OutActors.Empty();

	if (!LevelStreaming || !LevelStreaming->GetLoadedLevel())
	{
		return;
	}

	// Use the template type T to filter the actors
	OutActors = LevelStreaming->GetLoadedLevel()->Actors.FilterByPredicate([](T* Actor)
	{
		return Actor && Actor->IsA(T::StaticClass());
	});
}

template <typename T>
void UHelperFunctionLibrary::GetAllActorsWithInterfaceFromStreamLevel(ULevelStreaming* LevelStreaming,TArray<AActor*>& OutActors)
{
	static_assert(
	TIsDerivedFrom<T, IInterface>::Value,
	"T must be an interface type."
	);

	if (!IsValid(LevelStreaming))
	{
		UE_LOG(LogHelperFunctionLibrary, Error, TEXT("Trying to get actors from invalid levelstreaming"))
		return;
	}

	if (!IsValid(LevelStreaming->GetLoadedLevel()))
	{
		UE_LOG(LogHelperFunctionLibrary, Error, TEXT("Trying to get actors from invalid loaded level"))
		return;
	}
	
	GetAllActorsOfClassFromStreamLevel<AActor>(LevelStreaming,OutActors);
	
	const TSubclassOf<UInterface> InterfaceClass = T::UClassType::StaticClass();
	OutActors = LevelStreaming->GetLoadedLevel()->Actors.FilterByPredicate([InterfaceClass](AActor* Actor)
	{
		return Actor && UKismetSystemLibrary::DoesClassImplementInterface(Actor->GetClass(),InterfaceClass);
	});
}

template <typename T>
void UHelperFunctionLibrary::GetAllActorsWithComponentFromStreamLevel(ULevelStreaming* LevelStreaming,TArray<AActor*>& OutActors)
{
	static_assert(
	TIsDerivedFrom<T, UActorComponent>::Value,
	"T must be an Actor Component."
	);
	
	if (!LevelStreaming || !LevelStreaming->GetLoadedLevel())
		return;
	
	GetAllActorsOfClassFromStreamLevel<AActor>(LevelStreaming,OutActors);
	
	const TSubclassOf<UInterface> InterfaceClass = T::UClassType::StaticClass();
	OutActors = LevelStreaming->GetLoadedLevel()->Actors.FilterByPredicate([InterfaceClass](AActor* Actor)
	{
		return Actor && Actor->GetComponentByClass<T>() != nullptr;
	});
}
