// (C) 2020 Seokjin Lee <seokjin.dev@gmail.com>

using UnrealBuildTool;

public class CP0Target : TargetRules
{
	public CP0Target(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V2;

		ExtraModuleNames.AddRange(new[] {"CP0"});
	}
}