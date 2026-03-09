using UnrealBuildTool;

public class ObjectNetInspector : ModuleRules
{
    public ObjectNetInspector(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new[]
            {
                "Core",
                "CoreUObject",
                "TraceServices"
            });

        PrivateDependencyModuleNames.AddRange(
            new[]
            {
                "ApplicationCore",
                "Engine",
                "InputCore",
                "Slate",
                "SlateCore",
                "EditorStyle",
                "ToolMenus",
                "WorkspaceMenuStructure",
                "TraceAnalysis",
                "TraceInsights",
                "NetworkingInsights"
            });

        // TODO: If the target UE version moves specific Insights APIs across modules,
        // keep this list minimal and adjust only with verified module names.
    }
}
