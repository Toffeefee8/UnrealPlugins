// Copyright Phoenix Dawn Development LLC. All Rights Reserved.


#include "SaveObjects/SoloSaveGame.h"

void USoloSaveGame::SaveFloat(FGameplayTag Tag, float Value)
{
	if (!Tag.IsValid())
		return;
	
	FloatValues.Add(Tag, Value);
	bDirty = true;
}

bool USoloSaveGame::LoadFloat(FGameplayTag Tag, float& Value) const
{
	if (!Tag.IsValid())
		return false;
	
	if (const float* FoundValue = FloatValues.Find(Tag))
	{
		Value = *FoundValue;
		return true;
	}
	
	Value = 0.0f;
	return false;
}

void USoloSaveGame::ClearFloat(FGameplayTag Tag)
{
	if (!Tag.IsValid())
		return;
	
	FloatValues.Remove(Tag);
	bDirty = true;
}

void USoloSaveGame::SaveString(FGameplayTag Tag, FString Value)
{
	if (!Tag.IsValid())
		return;
	
	StringValues.Add(Tag, Value);
	bDirty = true;
}

bool USoloSaveGame::LoadString(FGameplayTag Tag, FString& Value) const
{
	if (!Tag.IsValid())
		return false;
	
	if (const FString* FoundValue = StringValues.Find(Tag))
	{
		Value = *FoundValue;
		return true;
	}
	
	Value = "";
	return false;
}

void USoloSaveGame::ClearString(FGameplayTag Tag)
{
	if (!Tag.IsValid())
		return;
	
	StringValues.Remove(Tag);
	bDirty = true;
}
