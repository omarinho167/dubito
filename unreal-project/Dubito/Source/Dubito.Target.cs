using UnrealBuildTool;
using System.Collections.Generic;

public class DubitoTarget : TargetRules
{
	public DubitoTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

		ExtraModuleNames.Add("Dubito");
	}
}
