// Copyright Phoenix Dawn Development LLC. All Rights Reserved.

#include "Save/ObjectSerializationLibrary.h"
#include "Save/ObjectSerializationInterface.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Runtime/Engine/Public/EngineGlobals.h"
#include "Misc/FileHelper.h"

bool UObjectSerializationLibrary::ObjectSerialize(TArray<uint8>& OutSerializedData, UObject* InObject, bool OnlySaveGame,bool CallInterface)
{
	if (!IsValid(InObject))
	{
		UE_LOG(ObjectSerializationLog, Warning, TEXT("Object Serialize Failed."))
		return false;
	}

	if (CallInterface)
	CallSerializationInterface(InObject, true);
	
	FMemoryWriter Writer(OutSerializedData, true);
	FObjectAndNameAsStringProxyArchive Archive(Writer, true);
	Archive.ArIsSaveGame = OnlySaveGame;
	Writer.SetIsSaving(true);

	InObject->Serialize(Archive);

	UE_LOG(ObjectSerializationLog, Log, TEXT("Object Serialize Success."))
	return true;
}


bool UObjectSerializationLibrary::ApplySerialization(const TArray<uint8>& SerializedData, UObject* InObject, bool OnlySaveGame, bool CallInterface)
{
	if (!InObject || !IsValid(InObject) || SerializedData.Num() <= 0)
	{
		return false;
	}
	
	FMemoryReader ActorReader(SerializedData, true);
	FObjectAndNameAsStringProxyArchive Archive(ActorReader, true);
	Archive.ArIsSaveGame = OnlySaveGame;
	ActorReader.SetIsLoading(true);
     
	InObject->Serialize(Archive);

	if (CallInterface)
		CallSerializationInterface(InObject, false);

	return true;
}

bool UObjectSerializationLibrary::CaptureObject(FObjectData& ObjectData, UObject* ObjectToSerialize, const bool SaveGame)
{
	return CaptureObject(ObjectData.ObjectClass,ObjectData.Data, ObjectToSerialize, SaveGame);
}

bool UObjectSerializationLibrary::CaptureObject(TSubclassOf<UObject>& ObjectClass, TArray<uint8>& Data,
	UObject* ObjectToSerialize, bool SaveGame)
{
	if (ObjectToSerialize)
	{
		ObjectClass= ObjectToSerialize->GetClass();
		return ObjectSerialize(Data, ObjectToSerialize, SaveGame);
	}
	UE_LOG(ObjectSerializationLog, Warning, TEXT("Object Capture Failed."))
	return false;
}

bool UObjectSerializationLibrary::BuildObject(UObject*& DeserializedObject, const FObjectData& ObjectData, UObject* Outer, const bool SaveGame)
{
	return BuildObject(DeserializedObject, ObjectData.ObjectClass, ObjectData.Data, Outer, SaveGame);
}

bool UObjectSerializationLibrary::BuildObject(UObject*& DeserializedObject, const TSubclassOf<UObject>& ObjectClass,
	const TArray<uint8>& Data, UObject* Outer, bool SaveGame)
{
	DeserializedObject = nullptr;
	
	if (ObjectClass)
	{
		UObject* TemplateObject = NewObject<UObject>(GetTransientPackage(), ObjectClass, NAME_None, RF_ArchetypeObject);
		if (ApplySerialization(Data, TemplateObject, SaveGame, false))
		{
			if (!Outer)
				Outer = GetTransientPackage();
			
			DeserializedObject = NewObject<UObject>(Outer, ObjectClass, NAME_None, RF_NoFlags, TemplateObject);
			CallSerializationInterface(DeserializedObject, false);
			
			UE_LOG(ObjectSerializationLog, Log, TEXT("Build Object Success."))
			return true;
		}
		
		UE_LOG(ObjectSerializationLog, Warning, TEXT("Build Object Failed: Apply Serialization Failed!"))
		return false;
	}

	UE_LOG(ObjectSerializationLog, Warning, TEXT("Build Object Failed: Object Class Invalid!"))
	return false;
}

bool UObjectSerializationLibrary::SaveObject(FObjectData& ObjectData, UObject* ObjectToSerialize)
{
	return CaptureObject(ObjectData, ObjectToSerialize, true);
}

bool UObjectSerializationLibrary::LoadObject(UObject*& DeserializedObject, FObjectData ObjectData, UObject* Outer)
{
	return BuildObject(DeserializedObject, ObjectData, Outer, true);
}

void UObjectSerializationLibrary::CallSerializationInterface(UObject* InObject, const bool Serialize)
{
	if (InObject && InObject->Implements<UObjectSerializationInterface>())
	{
		if (Serialize)
			IObjectSerializationInterface::Execute_OnSerialize(InObject);
		else
			IObjectSerializationInterface::Execute_OnDeserialize(InObject);
	}
}

void UObjectSerializationLibrary::TrySaveObjectToPackage(ESuccessType& Success, FObjectData& ObjectData, UObject* ObjectToSerialize)
{
	Success = SaveObject(ObjectData,ObjectToSerialize) == true ? ESuccessType::Succeeded : ESuccessType::Failed;
}

void UObjectSerializationLibrary::TryCreateObjectFromPackage(ESuccessType& Success, UObject*& DeserializedObject, FObjectData ObjectData,
	UObject* Outer)
{
	Success = LoadObject(DeserializedObject, ObjectData, Outer) == true ? ESuccessType::Succeeded : ESuccessType::Failed;
}
