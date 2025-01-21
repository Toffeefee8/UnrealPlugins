// Copyright Phoenix Dawn Development LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Enums/SuccessType.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ObjectSerializationLibrary.generated.h"

DECLARE_LOG_CATEGORY_CLASS(ObjectSerializationLog, Log, All)

class FBufferArchive;
class FMemoryReader;

USTRUCT(BlueprintType)
struct OBJECTEXTENSIONS_API FObjectData
{
	GENERATED_BODY()

public:
	
	FObjectData() {  }
	FObjectData(const TSubclassOf<UObject>& InObjectClass, const TArray<uint8>& InData) :
		ObjectClass(InObjectClass),
		Data(InData)
	{  }
    
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, SaveGame, Category = "Serialization")
	TSubclassOf<UObject> ObjectClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, SaveGame, Category= "Serialization")
	TArray<uint8> Data;
    
	friend FArchive& operator << (FArchive& Ar, FObjectData& Object)
	{
		Ar << Object.Data;
		return Ar;
	}

	void Clear(bool SetToNull = true)
	{
		Data.Empty();
		ObjectClass = SetToNull ? nullptr : UObject::StaticClass();
	}

	FString GetClassName() const
	{
		if (ObjectClass)
			return ObjectClass->GetName();
		return "NULL";
	}
};

USTRUCT(BlueprintType)
struct OBJECTEXTENSIONS_API FActorData
{
	GENERATED_BODY()
    
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, SaveGame, Category = "Serialization")
	TSubclassOf<AActor> Class;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, SaveGame, Category = "Serialization")
	FTransform Transform;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, SaveGame, Category= "Serialization")
	TArray<uint8> Data;
};

UCLASS()
class OBJECTEXTENSIONS_API UObjectSerializationLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

#pragma  region Serialization
	
#pragma  region Backend
	UFUNCTION(BlueprintCallable, Category = "Serialization|Saving")
	static bool ObjectSerialize(TArray<uint8>& OutSerializedData, UObject* InObject, bool OnlySaveGame, bool CallInterface = true);

	UFUNCTION(BlueprintCallable, Category = "Serialization|Saving")
	static bool ApplySerialization(const TArray<uint8>& SerializedData, UObject* InObject, bool OnlySaveGame, bool CallInterface = true);
#pragma  endregion

	//Used To Save Objects
	UFUNCTION(BlueprintCallable, Category = "Serialization|Saving")
	static bool CaptureObject(FObjectData& ObjectData, UObject* ObjectToSerialize, bool SaveGame = false);
	static bool CaptureObject(TSubclassOf<UObject>& ObjectClass, TArray<uint8>& Data, UObject* ObjectToSerialize, bool SaveGame = false);
	//Used To Load Objects
	UFUNCTION(BlueprintCallable, Category = "Serialization|Saving", meta = (DeterminesOutputType = "ObjectData.ObjectClass", DynamicOutputParam = "DeserializedObject", DefaultToSelf = "Outer"))
	static bool BuildObject(UObject*& DeserializedObject, const FObjectData& ObjectData, UObject* Outer, bool SaveGame = false);
	static bool BuildObject(UObject*& DeserializedObject, const TSubclassOf<UObject>& ObjectClass, const TArray<uint8>& Data, UObject* Outer, bool SaveGame = false);

	template <typename T>
	static T* GetObject(const FObjectData& ObjectData, UObject* Outer, bool SaveGame = false);
	template <typename T>
	static T* GetObject(TSubclassOf<T> TargetClass, const FObjectData& ObjectData, UObject* Outer, bool SaveGame = false);

	//Used To Save Objects With SaveGame
	UFUNCTION(BlueprintCallable, Category = "Serialization|Saving")
	static bool SaveObject(FObjectData& ObjectData, UObject* ObjectToSerialize);
	//Used To Load Objects With SaveGame
	UFUNCTION(BlueprintCallable, Category = "Serialization|Saving")
	static bool LoadObject(UObject*& DeserializedObject,FObjectData ObjectData, UObject* Outer);

	//Helpers
	static void CallSerializationInterface(UObject* InObject, bool Serialize);

#pragma  region With Excecution Pins
	UFUNCTION(BlueprintCallable, Category = "Serialization|Saving", meta = (ExpandEnumAsExecs = "Success"))
	static void TrySaveObjectToPackage(ESuccessType& Success, FObjectData& ObjectData, UObject* ObjectToSerialize);

	UFUNCTION(BlueprintCallable, Category = "Serialization|Saving", meta = (ExpandEnumAsExecs = "Success"))
	static void TryCreateObjectFromPackage(ESuccessType& Success, UObject*& DeserializedObject, FObjectData ObjectData, UObject* Outer);
#pragma endregion
	
#pragma  endregion

#pragma region Conversion
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Conversion", meta = (DeterminesOutputType = "OutputClass"))
	static UObject* GetAsType(TSubclassOf<UObject> OutputClass, UObject* ObjectToCovert, bool& Success)
	{
	    Success = false;
		if (!ObjectToCovert)
		{
			return nullptr;
		}
		if (ObjectToCovert->GetClass()->IsChildOf(OutputClass))
		{
			Success = true;
			return ObjectToCovert;
		}
		return nullptr;
	}
	UFUNCTION(BlueprintCallable, Category = "Conversion", meta = (DeterminesOutputType = "OutputClass", ExpandEnumAsExecs = "Success"))
    static UObject* Cast(ESuccessType& Success, TSubclassOf<UObject> OutputClass, UObject* ObjectToCovert)
    {
    	if (!ObjectToCovert)
    	{
    		Success = ESuccessType::Failed;
    		return nullptr;
    	}
    	if (ObjectToCovert->GetClass()->IsChildOf(OutputClass))
    	{
    		Success = ESuccessType::Succeeded;
    		return ObjectToCovert;
    	}
		Success = ESuccessType::Failed;
		return nullptr;
    }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Conversion", meta = (DeterminesOutputType = "OutputClass"))
	static TSubclassOf<UObject> MakeClass(TSubclassOf<UObject> OutputClass)
	{
		return OutputClass;
	}
	UFUNCTION(BlueprintCallable, Category = "Conversion", meta = (ExpandBoolAsExecs = "InBool"))
	static bool ExecToBool(bool InBool)
	{
		return InBool;
	}
#pragma  endregion
};

#pragma region Templates
template <typename T>
T* UObjectSerializationLibrary::GetObject(const FObjectData& ObjectData, UObject* Outer, bool SaveGame)
{
	return GetObject(T::StaticClass(), ObjectData, Outer, SaveGame);
}

template <typename T>
T* UObjectSerializationLibrary::GetObject(TSubclassOf<T> TargetClass, const FObjectData& ObjectData, UObject* Outer, bool SaveGame)
{
	static_assert(std::is_base_of_v<UObject, T>, "T must be a subclass of UObject");
	
	UObject* ReturnObject = nullptr;
	if (ObjectData.ObjectClass == TargetClass)
	{
		BuildObject(ReturnObject, ObjectData, Outer, SaveGame);
	}
	else
	{
		ReturnObject = NewObject<T>(Outer, TargetClass);
	}

	return static_cast<T*>(ReturnObject);
}
#pragma endregion