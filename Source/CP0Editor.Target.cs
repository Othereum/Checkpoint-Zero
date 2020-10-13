// © 2020 Seokjin Lee <seokjin.dev@gmail.com>

using UnrealBuildTool;
using System.Collections.Generic;

public class CP0EditorTarget : TargetRules
{
	public CP0EditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;

		ExtraModuleNames.AddRange( new string[] { "CP0" } );
	}
}
