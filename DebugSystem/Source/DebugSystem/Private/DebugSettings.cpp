// Copyright Phoenix Dawn Development LLC. All Rights Reserved.


#include "DebugSettings.h"

bool UDebugSettings::ShouldDebug(FGameplayTag DebugTagIn, EDebugDisplayType DebugTypeIn) const
{
	if(bSuppressAllDebugs)
		return false;

	if (!ShouldShowDebugType(DebugTypeIn))
		return false;

	for (auto DebugConfig : SuppressedDebugConfigs)
	{
		if (DebugTagIn.MatchesTag(DebugConfig.Key))
		{
			if (DebugConfig.Value.AllowsDebugType(DebugTypeIn))
				return false;
		}
	}

	for (auto DebugConfig : ShownDebugConfigs)
	{
		if (DebugTagIn.MatchesTag(DebugConfig.Key))
		{
			if (DebugConfig.Value.AllowsDebugType(DebugTypeIn))
				return true;
		}
	}
	
	return false;
}

float UDebugSettings::GetDebugDuration(FGameplayTag DebugTagIn, EDebugDisplayType DebugTypeIn) const
{
	FDebugConfig Config = GetShownDebugConfig(DebugTagIn);
	switch (DebugTypeIn) {
		case EDebugDisplayType::Log:
			return Config.LogDuration;
		case EDebugDisplayType::Print:
			return Config.PrintDuration;
		case EDebugDisplayType::Visual:
			return Config.VisualDuration;
		case EDebugDisplayType::Sound:
			return Config.SoundVolume;
	}
	return 1;
}

FDebugConfig UDebugSettings::GetShownDebugConfig(FGameplayTag DebugTag) const
{
	for (auto DebugConfig : ShownDebugConfigs)
	{
		if (DebugTag.MatchesTag(DebugConfig.Key))
		{
			return DebugConfig.Value;
		}
	}
	return FDebugConfig();
}

FDebugConfig UDebugSettings::GetSuppressedDebugConfig(FGameplayTag DebugTag) const
{
	for (auto DebugConfig : SuppressedDebugConfigs)
	{
		if (DebugTag.MatchesTag(DebugConfig.Key))
		{
			return DebugConfig.Value;
		}
	}
	return FDebugConfig();
}

bool UDebugSettings::SuppressAllDebugs() const
{
	return bSuppressAllDebugs;
}

UDebugSettings* UDebugSettings::Get()
{
	return StaticClass()->GetDefaultObject<UDebugSettings>();
}

FName UDebugSettings::GetCategoryName() const
{
	return FApp::GetProjectName();
}

bool UDebugSettings::ShouldShowDebugType(EDebugDisplayType DebugTypeIn) const
{
	switch (DebugTypeIn) {
		case EDebugDisplayType::Log:
			return bAllowLog;
		case EDebugDisplayType::Print:
			return bAllowPrint;
		case EDebugDisplayType::Visual:
			return bAllowVisual;
		case EDebugDisplayType::Sound:
			return bAllowSound;
	}
	return false;
}
