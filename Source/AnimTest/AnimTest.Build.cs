// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class AnimTest : ModuleRules
{
	public AnimTest(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "GameplayTags", "AnimGraph", "AnimGraphRunTime", "BlueprintGraph" });

        PrivateDependencyModuleNames.AddRange(new string[] { "AnimGraph", "AnimGraphRunTime", "BlueprintGraph" });
    }
}
