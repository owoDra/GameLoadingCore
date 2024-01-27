// Copyright (C) 2024 owoDra

using UnrealBuildTool;

public class GCPreLoading : ModuleRules
{
	public GCPreLoading(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[]
            {
                ModuleDirectory,
                ModuleDirectory + "/GCPreLoading",
            }
        );


        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core", "CoreUObject", "Engine",
            }
        );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Slate", "SlateCore",

                "PreLoadScreen",
            }
        );
    }
}
