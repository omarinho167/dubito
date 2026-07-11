using UnrealBuildTool;

// Automation test module for DubitoCore contracts. Test-only: built for editor/automation,
// excluded from shipping. Depends on DubitoCore plus Core/CoreUObject for the automation framework.
public class DubitoTests : ModuleRules
{
	public DubitoTests(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"DubitoCore"
		});
	}
}
