// Copyright Epic Games, Inc. All Rights Reserved.

#include "SaveTags.h"
#include "Engine/EngineTypes.h"

namespace SaveTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Name, "Save", "Parent Tag for all save related tags.");

	namespace Type
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Name, "Save.Type", "Parent Tag for all save Types.");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(World, "Save.Type.World", "Tag for all saves related to the world and server content. (Used for server saving)");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Player, "Save.Type.Player", "Tag for all saves related to the player and client content. (Used for client saving)");
	}
	
	namespace IDs
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Name, "Save.IDs", "Parent Tag for all unique save identifiers.");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Debug, "Save.IDs.Debug", "Tag for testing and debugging.");
	}

	namespace Constants
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Name, "Save.Constants", "Parent Tag for all Constants.");

		namespace Worlds
		{
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Name, "Save.Constants.Worlds", "Parent Tag for World Constants.");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(MainMenu, "Save.Constants.Worlds.MainMenu", "Defined Main Menu Level.");
			UE_DEFINE_GAMEPLAY_TAG_COMMENT(Game, "Save.Constants.Worlds.Game", "Defined Game Level.");
		}
	}
}
