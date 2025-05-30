﻿// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class RegionSystem : ModuleRules
{
	public RegionSystem(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core", 
				"ModularGameplay", 
				"ObjectExtensions", 
				"AIModule",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore", 
				"DebugSystem", 
				"GameplayAbilities",
				"GameplayTags",
				"NavigationSystem",
				"DeveloperSettings",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

		if (Target.bBuildEditor)
		{
			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					
				}
			);
			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"GameplayTagsEditor",
					"UnrealEd",
				}
			);
		}
	}
}
