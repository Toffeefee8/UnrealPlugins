// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "NativeGameplayTags.h"

namespace SaveTags
{
	SAVESYSTEM_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Name);

	namespace Type
	{
		SAVESYSTEM_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Name);
		SAVESYSTEM_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(World);
		SAVESYSTEM_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player);
	}

	namespace IDs
	{
		SAVESYSTEM_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Name);
		SAVESYSTEM_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Debug);
	}
	namespace Constants
	{
		SAVESYSTEM_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Name);

		namespace Worlds
		{
			SAVESYSTEM_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Name);
			SAVESYSTEM_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(MainMenu);
			SAVESYSTEM_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Game);
		}
	}
}
