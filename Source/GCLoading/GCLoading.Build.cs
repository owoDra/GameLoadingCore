// Copyright (C) 2024 owoDra

using UnrealBuildTool;

public class GCLoading : ModuleRules
{
	public GCLoading(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[]
            {
                ModuleDirectory,
                ModuleDirectory + "/GCLoading",
            }
        );


        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core", "CoreUObject", "Engine",

                "Slate", "SlateCore", "UMG",

                "GameplayTags",
            }
        );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "DeveloperSettings",

                "RenderCore", "ApplicationCore", "InputCore",

                "PreLoadScreen",
            }
        );
    }
}
