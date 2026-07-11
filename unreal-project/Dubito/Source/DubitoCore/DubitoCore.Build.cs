using UnrealBuildTool;

// Pure rules module: cards, deck, hands, claims, state transitions, rule validation, constants.
// MUST NOT depend on the Engine module. Core value types (Core/CoreUObject) are allowed for
// reflection/replication contracts, but rule logic must stay plain, deterministic, and testable
// without a level.
public class DubitoCore : ModuleRules
{
	public DubitoCore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject"
		});
	}
}
