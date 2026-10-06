// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

using System;
using UnrealBuildTool;

public class SmartTablesTests : ModuleRules
{
	public SmartTablesTests(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		IWYUSupport = IWYUSupport.Full;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"Slate",
			"SlateCore",
			"UMG",
			"SmartTables",
		});

		if (!ShouldBuildTests(Target))
		{

			PublicDefinitions.Add("SMARTTABLES_WITH_TESTS=0");
			return;
		}

		PublicDefinitions.Add("SMARTTABLES_WITH_TESTS=1");

		PrivateIncludePaths.Add(System.IO.Path.Combine(ModuleDirectory, "..", "SmartTables", "Private"));

		PrivateDependencyModuleNames.Add("AITestSuite");
	}

	private static bool ShouldBuildTests(ReadOnlyTargetRules Target)
	{
		const bool bAlwaysBuildTests = false;

		bool bRequested = bAlwaysBuildTests || Environment.GetEnvironmentVariable("SMARTTABLES_BUILD_TESTS") == "1";

		return bRequested && Target.Configuration != UnrealTargetConfiguration.Shipping;
	}
}
