using UnrealBuildTool;

public class RegionSystemEditor : ModuleRules
{
    public RegionSystemEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core", 
                "GameplayTags",
                "InputCore",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "ObjectExtensionsEditor", 
                "RegionSystem",
                "UnrealEd",
                "ObjectExtensions",
            }
        );
    }
}