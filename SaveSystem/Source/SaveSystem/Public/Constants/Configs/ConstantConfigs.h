// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SaveSubSystem.h"
#include "UObject/Object.h"
#include "ConstantConfigs.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, DefaultToInstanced, EditInlineNew, Abstract)
class SAVESYSTEM_API UConstantConfigs : public UObject
{
	GENERATED_BODY()

public:
	
	template<typename T>
	bool GetData(FGameplayTag Tag, T& Value); 	
	template<typename T>
	static T GetConstantData(FGameplayTag Tag, TSubclassOf<UConstantConfigs> Class);
	template<typename T>
	TMap<FGameplayTag, void*> GetAsVoidPointer(TMap<FGameplayTag, T> Map);

protected:
	
	virtual TMap<FGameplayTag, void*> GetArray();
	// public:
	//
	// 	UFUNCTION(BlueprintCallable)
	// 	static VALUE GetConstantVALUE(FGameplayTag VALUETag);
	//
	// protected:
	// 	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ForceInlineRow))
	// 	TMap<FGameplayTag, VALUE> VALUEs;

};

template <typename T>
bool UConstantConfigs::GetData(FGameplayTag Tag, T& Value)
{
	TMap<FGameplayTag, void*> Map = GetArray();
	for (auto Data : Map)
	{
		if (Data.Key.MatchesTagExact(Tag))
		{
			Value = *static_cast<T*>(Data.Value);
			return true;
		}
	}
	Value = T();
	return false;
}

template <typename T>
T UConstantConfigs::GetConstantData(FGameplayTag Tag, TSubclassOf<UConstantConfigs> Class)
{
	if (!Tag.IsValid())
		return T();
	
	USaveSubSystem* System = USaveSubSystem::Get();
	TArray<UConstantConfigs*> Configs = System->GetAllConfigsOfType(Class);

	for (auto Config : Configs)
	{
		T Data {};
		if (Config->GetData<T>(Tag, Data))
		{
			return Data;
		}
	}
	UE_LOG(LogTemp, Error, TEXT("Constant with tag -%s- not found!"), *Tag.GetTagName().ToString())
	return T();
}

template <typename T>
TMap<FGameplayTag, void*> UConstantConfigs::GetAsVoidPointer(TMap<FGameplayTag, T> Map)
{
	TMap<FGameplayTag, void*> Result;

	for (const TPair<FGameplayTag, T>& Pair : Map)
	{
		T* ColorPtr = new T(Pair.Value);
		Result.Add(Pair.Key, ColorPtr);
	}

	return Result;
}


#pragma region Macros
#define DEFINE_CONSTANT_CONFIG(Type, TypeName) \
UCLASS() class SAVESYSTEM_API U##TypeName##Constants : public UConstantConfigs \
{ \
GENERATED_BODY() \
\
public: \
\
UFUNCTION(BlueprintCallable) \
static Type GetConstant##TypeName(FGameplayTag Tag) \
{ \
return GetConstantData<Type>(Tag, StaticClass()); \
} \
\
protected: \
\
virtual TMap<FGameplayTag, void*> GetArray() override \
{ \
return GetAsVoidPointer(TypeName##s); \
} \
\
UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ForceInlineRow)) \
TMap<FGameplayTag, Type> TypeName##s; \
};

#define DEFINE_CONSTANT_CONFIG_CONTENT(Type, TypeName) \
public: \
\
UFUNCTION(BlueprintCallable) \
static Type GetConstant##TypeName(FGameplayTag Tag) \
{ \
return GetConstantData<Type>(Tag, StaticClass()); \
} \
\
protected: \
\
virtual TMap<FGameplayTag, void*> GetArray() override \
{ \
return GetAsVoidPointer(TypeName##s); \
} \
\
UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ForceInlineRow)) \
TMap<FGameplayTag, Type> TypeName##s;
#pragma endregion