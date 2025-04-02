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

	UFUNCTION(BlueprintCallable)
	static FString RemoveParentTagsFromTag(FGameplayTag SourceTag, FGameplayTag ParentToRemove);

	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext), Category = "Replication")
	static bool GetLocalAuthority(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static int ConvertEnumToInteger(uint8 Byte);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject",AdvancedDisplay = "5"))
	static TArray<FVector> GeneratePointsOnCircleProjected(
		const UObject* WorldContextObject,
		const FVector& Center,
		float CircleRadius = 150,
		const FVector& Normal = FVector(0, 0, -1),
		const int32 NumPoints = 8,
		const bool bProjectToNavMesh = false,
		const float ProjectionRadius = 150.f);

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

	template <typename ComponentType>
	static void GetAllComponentsOfType(const TArray<AActor*> AvailableActors, TArray<ComponentType*>& OutComponents);

	UFUNCTION(BlueprintCallable,BlueprintPure, Category = "Actor")
	static bool IsActorFullyInBox2D(const AActor* Actor, const FBox2f& Box2D);

	UFUNCTION(BlueprintCallable,BlueprintPure, Category = "LevelStreaming")
	static TSoftObjectPtr<UWorld> GetWorldSoftObject(ULevel* Level);

	UFUNCTION(BlueprintCallable, Category = "ActorClass", meta=(DeterminesOutputType="ActorClass"))
	static UObject* GetDefaultObjectBP(TSubclassOf<UObject> ActorClass);
	
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
	template <class EnumType>
	
	static  FString GetEnumAsString(EnumType EnumValue, bool RemoveEnumName = true);
	template <class EnumType>

  	static FString GetEnumAsString(int32 EnumValue, bool RemoveEnumName= true);

	 template <class EnumType>
	static FName GetEnumAsName(EnumType EnumValue, bool RemoveEnumName= true);

	template <class EnumType>
	static FName GetEnumAsName(int32 EnumValue, bool RemoveEnumName= true);
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

template <typename ComponentType>
void UHelperFunctionLibrary::GetAllComponentsOfType(const TArray<AActor*> AvailableActors, TArray<ComponentType*>& OutComponents)
{
	OutComponents.Empty();
	for (AActor* Actor: AvailableActors)
	{
		if (!IsValid(Actor))
			continue;
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

template <typename EnumType>
FString UHelperFunctionLibrary::GetEnumAsString(EnumType EnumValue,const bool RemoveEnumName)
{
	static_assert(TIsEnum<EnumType>::Value, "GetEnumAsString can only be used with enum types!");
	return  GetEnumAsName<EnumType>(EnumValue,RemoveEnumName).ToString();
}

template <typename EnumType>
FString UHelperFunctionLibrary::GetEnumAsString(int32 EnumValue,const bool RemoveEnumName)
{
	static_assert(TIsEnum<EnumType>::Value, "GetEnumAsString can only be used with enum types!");
	return  GetEnumAsName<EnumType>(EnumValue,RemoveEnumName).ToString();
}

template <typename EnumType>
FName UHelperFunctionLibrary::GetEnumAsName(EnumType EnumValue,const bool RemoveEnumName)
{
	static_assert(TIsEnum<EnumType>::Value, "GetEnumAsName can only be used with enum types!");
	return  GetEnumAsName<EnumType>(static_cast<int32>(EnumValue),RemoveEnumName);
}

template <typename EnumType>
FName UHelperFunctionLibrary::GetEnumAsName(const int32 EnumValue, const bool RemoveEnumName)
{
	static_assert(TIsEnum<EnumType>::Value, "GetEnumAsName can only be used with enum types!");

	const UEnum* EnumInfo = StaticEnum<EnumType>();
	const FString FullEnumName = EnumInfo->GetNameByValue(EnumValue).ToString();

	if (RemoveEnumName)
	{
		FString EnumValueName;
		FullEnumName.Split(TEXT("::"), nullptr, &EnumValueName);
		return FName(EnumValueName);
	}
	else
	{
		return FName(FullEnumName);
	}

}
