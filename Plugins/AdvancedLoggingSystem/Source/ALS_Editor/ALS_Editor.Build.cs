//Copyright © 2025 RTerofer. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class ALS_Editor : ModuleRules
{
    public ALS_Editor(ReadOnlyTargetRules Target) : base(Target)
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
            "ALS"
        });

        PrivateDependencyModuleNames.AddRange(new[]
        {
            "Slate", 
            "SlateCore", 
            "UMG", 
            "UMGEditor", 
            "EditorStyle",
            "EditorFramework", 
            "AppFramework", 
            "Kismet", 
            "KismetCompiler",
            "BlueprintGraph", 
            "GraphEditor",
            "UnrealEd", 
            "ToolMenus", 
            "Blutility"
        });
    }
}
