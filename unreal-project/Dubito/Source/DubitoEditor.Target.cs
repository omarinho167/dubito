using UnrealBuildTool;
using System.Collections.Generic;

public class DubitoEditorTarget : TargetRules
{
	public DubitoEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

		// DubitoTests is a developer-tool module: it is built for editor/automation only.
		ExtraModuleNames.AddRange(new string[] { "Dubito", "DubitoTests" });
	}
}
