using UnrealBuildTool;

// Automation test module for Dubito contracts. Test-only: built for editor/automation,
// excluded from shipping. Core tests stay under Dubito.Core; Unreal bridge tests use Engine/Dubito.
public class DubitoTests : ModuleRules
{
	public DubitoTests(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"DubitoCore",
			"Dubito"
		});
	}
}
