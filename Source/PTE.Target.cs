// © 2020 Seokjin Lee <seokjin.dev@gmail.com>

using UnrealBuildTool;
using System.Collections.Generic;

public class PTETarget : TargetRules
{
	public PTETarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V2;

		ExtraModuleNames.AddRange( new string[] { "PTE" } );
	}
}
