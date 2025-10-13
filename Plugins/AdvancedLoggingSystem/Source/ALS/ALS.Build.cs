//Copyright © 2025 RTerofer. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class ALS : ModuleRules
{
    public ALS(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
		PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));

        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core", 
            "CoreUObject", 
            "Engine", 
            "InputCore", 
            "Projects",
            "UMG",
            "Slate",
            "SlateCore",
            "GameplayTags",
            "AIModule",
            "DeveloperSettings"
        });

        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new[]
            {
                "UMGEditor",
                "UnrealEd",
                "DeveloperToolSettings"
            });
        }
    }
}
