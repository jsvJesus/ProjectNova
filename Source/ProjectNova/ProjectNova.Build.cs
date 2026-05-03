using UnrealBuildTool;

public class ProjectNova : ModuleRules
{
	public ProjectNova(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"EnhancedInput",
			"ProceduralMeshComponent"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
            "UnrealEd",
            "AssetRegistry",
            "MaterialEditor"
        });

    }
}
