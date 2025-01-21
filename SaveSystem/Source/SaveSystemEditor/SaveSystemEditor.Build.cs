using UnrealBuildTool;

public class SaveSystemEditor : ModuleRules
{
    public SaveSystemEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "InputCore",
                "UnrealEd",
                "EditorSubsystem",
                "UMG",
                "UMGEditor",
                "Blutility", 
                "SaveSystem",
            }
        );
    }
}