// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SSL : ModuleRules
{
    public SSL(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDefinitions.Add("SSL_PACKAGE=1");

		bool bShouldUseModule = false;

		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"Core",
			}
		);

		if (bShouldUseModule)
		{
			PublicDefinitions.Add("WITH_SSL=1");

			PrivateIncludePaths.AddRange(
				new string[] {
					"Runtime/Online/SSL/Private",
				}
			);

			AddEngineThirdPartyPrivateStaticDependencies(Target, "OpenSSL");

			if (Target.Platform == UnrealTargetPlatform.Win32 ||
				Target.Platform == UnrealTargetPlatform.Win64)
			{
				PublicAdditionalLibraries.Add("crypt32.lib");
			}
		}
		else
		{
			PublicDefinitions.Add("WITH_SSL=0");
		}
    }
}
