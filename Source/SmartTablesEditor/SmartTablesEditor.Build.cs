// Smart Tables. See LICENSE at the root of this repository for the terms of use.
//
// A portfolio copy with the comments stripped. The fully documented source, UHT tooltips and all, is
// on FAB: https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd

using UnrealBuildTool;

public class SmartTablesEditor : ModuleRules
{
	public SmartTablesEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"Slate",
			"SlateCore",
			"UMG",
			"SmartTables",
			"PropertyEditor",
			"Projects",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"DetailCustomizations",
		});
	}
}
