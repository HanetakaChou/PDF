// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class XMPP : ModuleRules
{
	public XMPP(ReadOnlyTargetRules Target) : base(Target)
	{
		PublicDefinitions.Add("XMPP_PACKAGE=1");

		PrivateIncludePaths.AddRange(
			new string[] 
			{
				"Runtime/Online/XMPP/Private"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[] 
			{ 
				"Core",
				"Json"
			}
		);
		
		bool TargetPlatformSupportsJingle = false;
		bool TargetPlatformSupportsStrophe = false;

		if (TargetPlatformSupportsJingle)
		{
			AddEngineThirdPartyPrivateStaticDependencies(Target, "WebRTC");
			PrivateDefinitions.Add("WITH_XMPP_JINGLE=1");
		}
		else
		{
			PrivateDefinitions.Add("WITH_XMPP_JINGLE=0");
		}
		
		if (TargetPlatformSupportsStrophe)
		{
			AddEngineThirdPartyPrivateStaticDependencies(Target, "libstrophe");
			PrivateDependencyModuleNames.Add("WebSockets");
			PrivateDefinitions.Add("WITH_XMPP_STROPHE=1");
		}
		else
		{
			PrivateDefinitions.Add("WITH_XMPP_STROPHE=0");
		}

		if (Target.IsInPlatformGroup(UnrealPlatformGroup.Unix))
		{
			AddEngineThirdPartyPrivateStaticDependencies(Target, "libcurl");
		}
	}
}
