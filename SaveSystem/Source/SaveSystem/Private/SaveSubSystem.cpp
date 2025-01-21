// Copyright Phoenix Dawn Development LLC. All Rights Reserved.


#include "SaveSubSystem.h"

#include "DebugFunctionLibrary.h"
#include "SaveInterface.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "SaveSettings.h"
#include "Components/GameFrameworkComponent.h"
#include "Constants/ConstantsDataAsset.h"
#include "SaveObjects/SoloSaveGame.h"

DEFINE_LOG_CATEGORY(LogSaveSystem)

FString USaveSubSystem::SoloSaveDirectory = "";
FString USaveSubSystem::SoloSaveName = "Solos";

USaveSubSystem* USaveSubSystem::Get()
{
	return GEngine->GetEngineSubsystem<USaveSubSystem>();
}

UWorld* USaveSubSystem::GetWorld() const
{
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		return nullptr;
	}

	if (GEngine && GEngine->GameViewport)
	{
		if (UWorld* World = GEngine->GameViewport->GetWorld())
		{
			UGameInstance* GameInstance = World->GetGameInstance();
			return GameInstance->GetWorld();
		}
	}

	return nullptr;
}

void USaveSubSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	//Maybe Preload some Save Games
	ReevaluateLoadedSolos();
	VerifySoloDefaults();
	LoadAllConstants();
}

void USaveSubSystem::Deinitialize()
{
	Super::Deinitialize();

	//Save Loaded Save Games
	ReevaluateSolosForSave();
}

void USaveSubSystem::Save(UObject* WorldContextObject, FGameplayTag SaveTag)
{
	TArray<UObject*> ObjectsToSave = GetAllSaveObjects(WorldContextObject);
	
	FString DebugString = FString::Printf(TEXT("Requested type save for %s"), *SaveTag.GetTagName().ToString());
	DEBUG_SIMPLE(LogSaveSystem, Log, FColor::White, *DebugString, SaveTags::Name)

	for (auto Object : ObjectsToSave)
	{
		RequestSaveForObjectBySaveType(Object, SaveTag);
	}
}

void USaveSubSystem::Load(UObject* WorldContextObject, FGameplayTag SaveTag)
{
	TArray<UObject*> ObjectsToSave = GetAllSaveObjects(WorldContextObject);

	FString DebugString = FString::Printf(TEXT("Requested type load for %s"), *SaveTag.GetTagName().ToString());
	DEBUG_SIMPLE(LogSaveSystem, Log, FColor::White, *DebugString, SaveTags::Name)

	for (auto Object : ObjectsToSave)
	{
		RequestLoadForObjectBySaveType(Object, SaveTag);
	}
}

void USaveSubSystem::RequestSaveForObject(UObject* Object)
{
	if (!Object)
		return;
	
	FString DebugString = FString::Printf(TEXT("Requested save for %s"), *Object->GetName());
	DEBUG_SIMPLE(LogSaveSystem, Log, FColor::White, *DebugString, SaveTags::Name)

	if (!Object->Implements<USaveInterface>())
		return;

	FGameplayTagContainer SaveTypes = GetSupportedSaveTypes(Object);
	for (auto Type : SaveTypes)
	{
		SaveObject(Object, Type);
	}
}

void USaveSubSystem::RequestLoadForObject(UObject* Object)
{
	if (!Object)
		return;

	FString DebugString = FString::Printf(TEXT("Requested load for %s"), *Object->GetName());
	DEBUG_SIMPLE(LogSaveSystem, Log, FColor::White, *DebugString, SaveTags::Name)

	if (!Object->Implements<USaveInterface>())
		return;

	FGameplayTagContainer SaveTypes = GetSupportedSaveTypes(Object);
	for (auto Type : SaveTypes)
	{
		LoadObject(Object, Type);
	}
}

void USaveSubSystem::RequestClearForObject(UObject* Object)
{
	if (!Object)
		return;

	FString DebugString = FString::Printf(TEXT("Requested clear for %s"), *Object->GetName());
	DEBUG_SIMPLE(LogSaveSystem, Log, FColor::White, *DebugString, SaveTags::Name)

	if (!Object->Implements<USaveInterface>())
		return;

	FGameplayTagContainer SaveTypes = GetSupportedSaveTypes(Object);
	for (auto Type : SaveTypes)
	{
		ClearObject(Object, Type);
	}	
}

void USaveSubSystem::RequestSaveForObjectBySaveType(UObject* Object, FGameplayTag SaveType)
{
	if (!Object)
		return;
	
	FString DebugString = FString::Printf(TEXT("Requested save for %s"), *Object->GetName());
	DEBUG_SIMPLE(LogSaveSystem, Log, FColor::White, *DebugString, SaveTags::Name)
	
	if (!CanSaveByType(Object, SaveType))
		return;

	SaveObject(Object, SaveType);
}

void USaveSubSystem::RequestLoadForObjectBySaveType(UObject* Object, FGameplayTag SaveType)
{
	if (!Object)
		return;
		
	FString DebugString = FString::Printf(TEXT("Requested load for %s"), *Object->GetName());
	DEBUG_SIMPLE(LogSaveSystem, Log, FColor::White, *DebugString, SaveTags::Name)

	if (!CanSaveByType(Object, SaveType))
		return;

	LoadObject(Object, SaveType);
}

void USaveSubSystem::RequestClearForObjectBySaveType(UObject* Object, FGameplayTag SaveType)
{
	if (!Object)
		return;
	
	if (!CanSaveByType(Object, SaveType))
		return;

	ClearObject(Object, SaveType);
}

void USaveSubSystem::SaveSoloFloat(FGameplayTag Tag, float Value)
{
	ReevaluateLoadedSolos();

	FString DebugString = FString::Printf(TEXT("Saving %s: %f"), *Tag.GetTagName().ToString(), Value);
	DEBUG_SIMPLE(LogSaveSystem, Log, FColor::White, *DebugString, SaveTags::Name)
	
	LoadedSolos->SaveFloat(Tag, Value);

	ReevaluateSolosForSave();
}

bool USaveSubSystem::LoadSoloFloat(FGameplayTag Tag, float& Value) const
{
	ReevaluateLoadedSolos();

	bool bResult = LoadedSolos->LoadFloat(Tag, Value);
	
	FString DebugString = FString::Printf(TEXT("Loading %s: %f"), *Tag.GetTagName().ToString(), Value);
	DEBUG_SIMPLE(LogSaveSystem, Log, FColor::White, *DebugString, SaveTags::Name)

	return bResult;
}

void USaveSubSystem::ClearSoloFloat(FGameplayTag Tag)
{
	ReevaluateLoadedSolos();

	FString DebugString = FString::Printf(TEXT("Clearing %s"), *Tag.GetTagName().ToString());
	DEBUG_SIMPLE(LogSaveSystem, Log, FColor::White, *DebugString, SaveTags::Name)

	LoadedSolos->ClearFloat(Tag);

	ReevaluateSolosForSave();
}

void USaveSubSystem::SaveSoloString(FGameplayTag Tag, FString Value)
{
	ReevaluateLoadedSolos();

	FString DebugString = FString::Printf(TEXT("Saving %s: %s"), *Tag.GetTagName().ToString(), *Value);
	DEBUG_SIMPLE(LogSaveSystem, Log, FColor::White, *DebugString, SaveTags::Name)
	
	LoadedSolos->SaveString(Tag, Value);

	ReevaluateSolosForSave();
}

bool USaveSubSystem::LoadSoloString(FGameplayTag Tag, FString& Value)
{
	ReevaluateLoadedSolos();
	
	bool bResult = LoadedSolos->LoadString(Tag, Value);
	
	FString DebugString = FString::Printf(TEXT("Loading %s: %s"), *Tag.GetTagName().ToString(), *Value);
	DEBUG_SIMPLE(LogSaveSystem, Log, FColor::White, *DebugString, SaveTags::Name)
	
	return bResult;
}

void USaveSubSystem::ClearSoloString(FGameplayTag Tag)
{
	ReevaluateLoadedSolos();

	FString DebugString = FString::Printf(TEXT("Clearing %s"), *Tag.GetTagName().ToString());
	DEBUG_SIMPLE(LogSaveSystem, Log, FColor::White, *DebugString, SaveTags::Name)
	
	LoadedSolos->ClearString(Tag);

	ReevaluateSolosForSave();
}

FString USaveSubSystem::RemoveParentTagsFromTag(FGameplayTag SourceTag, FGameplayTag ParentToRemove)
{
	const FString SourceString = SourceTag.ToString();
	const FString ParentString = ParentToRemove.ToString();
	
	if (!SourceString.StartsWith(ParentString))
	{
		return SourceString;
	}
	
	if (SourceString.Equals(ParentString, ESearchCase::CaseSensitive))
	{
		return SourceString;
	}
	
	const int32 ParentLen = ParentString.Len();


	int32 RemoveLen = ParentLen;
	if (SourceString.IsValidIndex(ParentLen) && SourceString[ParentLen] == TEXT('.'))
	{
		RemoveLen = ParentLen + 1;
	}
	
	return SourceString.Mid(RemoveLen);
}

FString USaveSubSystem::GetFullSaveName(const FGameplayTag& TypeTag, const FGameplayTag& SaveTag) const
{
	FString TypeName = RemoveParentTagsFromTag(TypeTag, SaveTags::Type::Name);
	FString SaveName = RemoveParentTagsFromTag(SaveTag, SaveTags::IDs::Name);
	return TypeName + "/" + SaveName;
}

TArray<UObject*> USaveSubSystem::GetAllSaveObjects(const UObject* WorldContextObject) const
{
	const UWorld* World = WorldContextObject->GetWorld();
	
	if (!World)
		return TArray<UObject*>();
	
	TArray<UObject*> SaveObjects;
	for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
	{
		if (!*ActorIterator)
			continue;

		if (ActorIterator->Implements<USaveInterface>())
			SaveObjects.Add(*ActorIterator);
		
		for (TComponentIterator<UActorComponent> ComponentIterator(*ActorIterator); ComponentIterator; ++ComponentIterator)
		{
			if (!*ComponentIterator)
				continue;

			if (ComponentIterator->Implements<USaveInterface>())
				SaveObjects.Add(*ComponentIterator);
		}
	}
	
	FString DebugString = FString::Printf(TEXT("Found Savables Num: %i"), SaveObjects.Num());
	DEBUG_SIMPLE(LogSaveSystem, Log, FColor::White, *DebugString, SaveTags::Name)
	
	return SaveObjects;
}

bool USaveSubSystem::CanSaveByType(UObject* Object, FGameplayTag SaveType) const
{
	if (Object->Implements<USaveInterface>())
	{
		FGameplayTagContainer Container {};
		ISaveInterface::Execute_GetSupportedSaveTypes(Object, Container);
		return Container.HasTagExact(SaveType);
	}
	
	FString DebugString = FString::Printf(TEXT("Object does not implement save interface! %s"), *Object->GetName());
	DEBUG_SIMPLE(LogSaveSystem, Warning, FColor::Orange, *DebugString, SaveTags::Name)
	
	return false;
}

bool USaveSubSystem::GetSaveIDs(UObject* Object, FGameplayTag& SaveTag, TSubclassOf<USaveGame>& SaveClass) const
{
	if (!Object->Implements<USaveInterface>())
		return false;
	
	ISaveInterface::Execute_GetSaveIDs(Object, SaveTag, SaveClass);
	return SaveTag.IsValid() && SaveClass !=  nullptr;
}

FGameplayTagContainer USaveSubSystem::GetSupportedSaveTypes(UObject* Object) const
{
	FGameplayTagContainer Container {};
	if (Object->Implements<USaveInterface>())
		ISaveInterface::Execute_GetSupportedSaveTypes(Object, Container);
	
	return Container;
}

void USaveSubSystem::SaveObject(UObject* Object, const FGameplayTag SaveType) const
{
	FGameplayTag SaveTag {};
	TSubclassOf<USaveGame> SaveClass = nullptr;
	bool bValidCustomData = GetSaveIDs(Object, SaveTag, SaveClass);

	FString SaveName = GetFullSaveName(SaveType, SaveTag);
	USaveGame* SaveObject = nullptr;
	if (bValidCustomData)
	{
		FString DebugString = FString::Printf(TEXT("Object will be saved under: %s"), *SaveName);
		DEBUG_SIMPLE(LogSaveSystem, Log, FColor::White, *DebugString, SaveTags::Name)

		//Loads in previous save if configured
		if (USaveSettings::Get()->bLoadDataBeforeSave)
			SaveObject = UGameplayStatics::LoadGameFromSlot(SaveName, 0);

		if (!SaveObject)
			SaveObject = UGameplayStatics::CreateSaveGameObject(SaveClass);
	}
	else
	{
		FString DebugString = FString::Printf(TEXT("Triggering Generic save on: %s"), *Object->GetName());
		DEBUG_SIMPLE(LogSaveSystem, Log, FColor::White, *DebugString, SaveTags::Name)
	}

	//Execute Interface
	ISaveInterface::Execute_OnSave(Object, SaveObject, LoadedSolos, SaveType);

	if (bValidCustomData)
	{
		UGameplayStatics::SaveGameToSlot(SaveObject, SaveName, 0);
	}

	//Save Solos if modified
	ReevaluateSolosForSave();
}

void USaveSubSystem::LoadObject(UObject* Object, const FGameplayTag SaveType) const
{
	FGameplayTag SaveTag {};
	TSubclassOf<USaveGame> SaveClass = nullptr;
	bool bValidCustomData = GetSaveIDs(Object, SaveTag, SaveClass);
	
	FString SaveName = GetFullSaveName(SaveType, SaveTag);	
	USaveGame* SaveObject = nullptr;

	if (bValidCustomData)
	{
		SaveObject = UGameplayStatics::LoadGameFromSlot(SaveName, 0);
		
		FString DebugString = FString::Printf(TEXT("Object loaded from: %s for %s"), *SaveName, *Object->GetName());
		DEBUG_SIMPLE(LogSaveSystem, Log, FColor::White, *DebugString, SaveTags::Name)
	}
	else
	{
		FString DebugString = FString::Printf(TEXT("Triggering Generic load on: %s"), *Object->GetName());
		DEBUG_SIMPLE(LogSaveSystem, Log, FColor::White, *DebugString, SaveTags::Name)
	}

	//Execute Interfaces
	bool bValid = SaveObject != nullptr;
	ISaveInterface::Execute_OnLoad(Object, SaveObject, LoadedSolos, SaveType, bValid);

	//Save Solos if Modified
	ReevaluateSolosForSave();
}

void USaveSubSystem::ClearObject(UObject* Object, const FGameplayTag SaveType) const
{
	FGameplayTag SaveTag {};
	TSubclassOf<USaveGame> SaveClass = nullptr;
	if (GetSaveIDs(Object, SaveTag, SaveClass))
	{
		FString SaveName = GetFullSaveName(SaveType, SaveTag);
		UGameplayStatics::DeleteGameInSlot(SaveName, 0);
		
		FString DebugString = FString::Printf(TEXT("Object cleared: %s"), *SaveName);
		DEBUG_SIMPLE(LogSaveSystem, Log, FColor::White, *DebugString, SaveTags::Name)
	}
}

USoloSaveGame* USaveSubSystem::LoadSolos()
{
	USaveGame* LoadedSave = UGameplayStatics::LoadGameFromSlot(GetSoloSaveName(), 0);
	USoloSaveGame* CastSave = Cast<USoloSaveGame>(LoadedSave);
	if (!CastSave)
	{
		CastSave = Cast<USoloSaveGame>(UGameplayStatics::CreateSaveGameObject(USoloSaveGame::StaticClass()));
	}

	return CastSave;
}

bool USaveSubSystem::SaveSolos(USoloSaveGame* SoloToSave)
{
	return UGameplayStatics::SaveGameToSlot(SoloToSave, GetSoloSaveName(), 0);
}

FString USaveSubSystem::GetSoloSaveName()
{
	return SoloSaveDirectory + SoloSaveName;
}

TArray<UConstantConfigs*> USaveSubSystem::GetAllConfigsOfType(const TSubclassOf<UConstantConfigs>& Class)
{
#if WITH_EDITOR
	LoadAllConstants();
#endif
	
	TArray<UConstantConfigs*> Configs {};
	for (auto Constant : Constants)
	{
		Configs.Append(Constant->GetAllConfigsOfType(Class));
	}
	return Configs;
}

void USaveSubSystem::LoadAllConstants()
{
	USaveSettings* SaveSettings = USaveSettings::Get();
	if (!SaveSettings)
		return;

	Constants.Empty();
	for (auto ConstantDefinitions : SaveSettings->ConstantDefinitions)
	{
		UConstantsDataAsset* Asset = ConstantDefinitions.LoadSynchronous();

		if (!Asset)
			continue;

		Constants.Add(Asset);
	}
}

void USaveSubSystem::VerifyAllConstants()
{
	USaveSettings* SaveSettings = USaveSettings::Get();
	if (!SaveSettings)
	{
		return;
	}
	
	TArray<UConstantsDataAsset*> DesiredAssets;
	DesiredAssets.Reserve(SaveSettings->ConstantDefinitions.Num());
	
	for (const TSoftObjectPtr<UConstantsDataAsset>& Definition : SaveSettings->ConstantDefinitions)
	{
		UConstantsDataAsset* LoadedAsset = nullptr;
		
		if (Definition.IsValid())
			LoadedAsset = Definition.Get();
		else
			LoadedAsset = Definition.LoadSynchronous();
		
		if (LoadedAsset)
			DesiredAssets.Add(LoadedAsset);
	}
	
	for (int32 Index = Constants.Num() - 1; Index >= 0; --Index)
	{
		UConstantsDataAsset* ExistingAsset = Constants[Index];
		if (!ExistingAsset || !DesiredAssets.Contains(ExistingAsset))
		{
			Constants.RemoveAt(Index);
		}
	}
	
	for (UConstantsDataAsset* Desired : DesiredAssets)
	{
		if (!Constants.Contains(Desired))
		{
			Constants.Add(Desired);
		}
	}
}

void USaveSubSystem::ReevaluateSolosForSave() const
{
	if (LoadedSolos->bDirty)
		SaveSolos(LoadedSolos);
}

void USaveSubSystem::ReevaluateLoadedSolos() const
{
	if (!LoadedSolos)
		LoadedSolos = LoadSolos();
}

void USaveSubSystem::VerifySoloDefaults() const
{
	USaveSettings* SaveSettings = USaveSettings::Get();
	check(SaveSettings);
	
	TMap<FGameplayTag, FString> StringSolos = SaveSettings->DefaultStringSolos;
	for (auto StringSolo : StringSolos)
	{
		FString Value;
		if (!LoadedSolos->LoadString(StringSolo.Key, Value))
			LoadedSolos->SaveString(StringSolo.Key, StringSolo.Value);
	}

	TMap<FGameplayTag, float> FloatSolos = SaveSettings->DefaultFloatSolos;
	for (auto StringSolo : FloatSolos)
	{
		float Value;
		if (!LoadedSolos->LoadFloat(StringSolo.Key, Value))
			LoadedSolos->SaveFloat(StringSolo.Key, StringSolo.Value);
	}

	ReevaluateSolosForSave();
}
