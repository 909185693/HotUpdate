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

public class HotUpdate : ModuleRules
{
    public HotUpdate(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        OptimizeCode = CodeOptimization.InShippingBuildsOnly;

#if !__MonoCS__
        var ProjectDir = Target.ProjectFile.Directory;
        var ConfigFilePath = ProjectDir + "/Config/DefaultHotUpdate.ini";
        var ConfigFileReference = new FileReference(ConfigFilePath);
        var ConfigFile = FileReference.Exists(ConfigFileReference) ? new ConfigFile(ConfigFileReference) : new ConfigFile();
        var Config = new ConfigHierarchy(new[] { ConfigFile });
        const string Section = "/Script/HotUpdate.HotUpdateSettings";

        int MajorVersion = 0;
        Config.GetInt32(Section, "MajorVersion", out MajorVersion);

        int MinorVersion = 0;
        Config.GetInt32(Section, "MinorVersion", out MinorVersion);

        string Arguments = MajorVersion + " " + MinorVersion + " ";

        string VersionController;
        Config.GetString(Section, "VersionController", out VersionController);

        string BatchDirectory = null;
        if (VersionController == "Git")
        {
            BatchDirectory = Path.Combine(PluginDirectory, "Batch/Git");
        }
        else if (VersionController == "Perforce")
        {
            BatchDirectory = Path.Combine(PluginDirectory, "Batch/Perforce");

            string P4Server;
            Config.GetString(Section, "P4Server", out P4Server);

            string P4Username;
            Config.GetString(Section, "P4Username", out P4Username);

            string P4Password;
            Config.GetString(Section, "P4Password", out P4Password);

            string P4Workspaces;
            Config.GetString(Section, "P4Workspaces", out P4Workspaces);

            Arguments += P4Server + " " + P4Username + " " + P4Password + " " + P4Workspaces + " ";
        }
        else if (VersionController == "Subversion")
        {
            BatchDirectory = Path.Combine(PluginDirectory, "Batch/Subversion");
        }

        if (BatchDirectory != null)
        {
            System.Console.WriteLine("-------------------- HotUpdate Config ---------------------");
            Process Proc = new Process();
            Proc.StartInfo.WorkingDirectory = BatchDirectory;
            Proc.StartInfo.FileName = Path.Combine(BatchDirectory, "Version.bat");
            Proc.StartInfo.Arguments = Arguments;
            Proc.StartInfo.WindowStyle = ProcessWindowStyle.Hidden;
            Proc.StartInfo.RedirectStandardOutput = true;
            Proc.StartInfo.RedirectStandardError = true;
            Proc.StartInfo.UseShellExecute = false;
            Proc.OutputDataReceived += new DataReceivedEventHandler((SendingProcess, OutLine) =>
            {
                System.Console.WriteLine(OutLine.Data);
            });
            Proc.Start();
            Proc.BeginOutputReadLine();
            Proc.BeginErrorReadLine();
            Proc.WaitForExit();

            System.Console.WriteLine("Auto write version file by " + VersionController);
            System.Console.WriteLine("---------------------------------------------------------------");
        }
#endif

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
                "HTTP",
                "Json",
                "JsonUtilities"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "PakFile"
            }
        );

        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {

            }
        );
    }
}
