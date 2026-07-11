using UnrealBuildTool;

// Primary game module: Unreal gameplay framework, networking, UI shells, sessions, pawns, actors.
// May depend on DubitoCore and UE runtime modules. Required V1 systems are wired here so their
// linkage is proven at bootstrap (Phase 1.4). No gameplay logic yet.
public class Dubito : ModuleRules
{
	public Dubito(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"DubitoCore",
			// Required V1 systems (Phase 1.4)
			"UMG",
			"EnhancedInput",
			"CommonUI",
			"OnlineSubsystem",
			"OnlineSubsystemUtils"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });
	}
}
