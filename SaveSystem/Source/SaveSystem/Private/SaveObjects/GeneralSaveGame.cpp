// Copyright Phoenix Dawn Development LLC. All Rights Reserved.

#include "SaveObjects/GeneralSaveGame.h"

#include "GameplayTagContainer.h"
#include "Save/ObjectSerializationLibrary.h"

void UGeneralSaveGame::AddData(FGameplayTag Tag, FObjectData Data)
{
	TagDataPairs.Add(Tag, Data);
}

bool UGeneralSaveGame::RemoveData(FGameplayTag Tag)
{
	int Index = TagDataPairs.Remove(Tag);
	return Index >= 0;
}

bool UGeneralSaveGame::GetData(FGameplayTag Tag, FObjectData& Data)
{
	if (FObjectData* FoundData = TagDataPairs.Find(Tag))
	{
		Data = *FoundData;
		return true;
	}
	
	Data = FObjectData();
	return false;
}

TMap<FGameplayTag, FObjectData> UGeneralSaveGame::GetAllData()
{
	return TagDataPairs;
}
