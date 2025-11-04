// Copyright Henet LLC 2025
// Build.cs file for HenetSwitchControl plugin

using UnrealBuildTool;

public class HenetSwitchControl : ModuleRules
{
    public HenetSwitchControl(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[] {
                // ... add public include paths here if needed
            }
            );
                
        
        PrivateIncludePaths.AddRange(
            new string[] {
                // ... add other private include paths here
            }
            );
            
        
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                // ... add other public dependencies here
            }
            );
            
        
        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "Projects" // Needed for IPluginManager
                // ... add private dependencies here
            }
            );
        
        // Add necessary definitions and libraries for Windows serial communication
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PrivateIncludePaths.Add(System.IO.Path.Combine(ModuleDirectory, "Private/Windows"));

            // Add definitions to conditionally compile Windows-specific code
            PublicDefinitions.Add("HENET_WINDOWS_SERIAL=1");

            // Add necessary system libraries
            PublicSystemLibraries.Add("kernel32.lib");
            PublicSystemLibraries.Add("setupapi.lib");
        }
        
        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
                // ... add any dynamically loaded modules here
            }
            );
    }
}

