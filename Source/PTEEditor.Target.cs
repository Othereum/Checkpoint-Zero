// © 2020 Seokjin Lee <seokjin.dev@gmail.com>

using UnrealBuildTool;
using System.Collections.Generic;

public class PTEEditorTarget : TargetRules
{
	public PTEEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;

		ExtraModuleNames.AddRange( new string[] { "PTE" } );
	}
}
