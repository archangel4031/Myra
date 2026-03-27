using UnrealBuildTool;

public class MyraEditor : ModuleRules
{
	public MyraEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			  "Core",
			  "Myra"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			  "CoreUObject",
			  "Engine",
			  "Slate",
			  "SlateCore",
			  "UnrealEd",
			  "ToolMenus",
			  "PropertyEditor",
			  "AssetTools",
			  "DesktopPlatform",
			  "AppFramework",
			  "Projects"
		});
	}
}