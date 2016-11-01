// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UDPServer : ModuleRules
{
	public UDPServer(TargetInfo Target)
	{
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore","Sockets" , "Networking" });
        PrivateDependencyModuleNames.AddRange(
     new string[] {
         "Networking",
         "Sockets",
     }
 );
    }
}
