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
                "InputCore",
                "Slate",
                "SlateCore",
                "TraceAnalysis",
                "TraceInsights",
                "TraceInsightsCore"
            });

        if (Target.bCompileAgainstEngine)
        {
            PrivateDependencyModuleNames.Add("Engine");
        }

        if (Target.Type == TargetType.Editor)
        {
            PrivateDependencyModuleNames.AddRange(
                new[]
                {
                    "EditorStyle",
                    "ToolMenus",
                    "WorkspaceMenuStructure"
                });
        }

        // TODO: If the target UE version moves specific Insights APIs across modules,
        // keep this list minimal and adjust only with verified module names.
    }
}
