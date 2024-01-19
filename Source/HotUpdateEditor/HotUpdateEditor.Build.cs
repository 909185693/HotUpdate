// Some copyright should be here...

using System;
using System.IO;
using System.Diagnostics;
#if UE_5_0_OR_LATER
using EpicGames.Core;
#else
using Tools.DotNETCommon;
#endif
using UnrealBuildTool;

public class HotUpdateEditor : ModuleRules
{
    public HotUpdateEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        OptimizeCode = CodeOptimization.InShippingBuildsOnly;

        PublicIncludePaths.AddRange(
            new string[]
            {

            }
        );

        PrivateIncludePaths.AddRange(
            new string[]
            {

			}
        );

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
				"HotUpdate"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "EditorStyle",
                "Projects"
            }
        );

        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {

            }
        );
    }
}
