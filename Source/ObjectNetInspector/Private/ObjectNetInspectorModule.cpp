#include "ObjectNetInspectorTab.h"

#include "Framework/Docking/TabManager.h"
#include "Modules/ModuleManager.h"
#if WITH_EDITOR
#include "ToolMenus.h"
#endif

#if __has_include("WorkspaceMenuStructure.h") && __has_include("WorkspaceMenuStructureModule.h")
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#define OBJECTNET_HAS_WORKSPACE_MENU 1
#else
#define OBJECTNET_HAS_WORKSPACE_MENU 0
#endif

DEFINE_LOG_CATEGORY_STATIC(LogObjectNetInspector, Log, All);

static const FName ObjectNetInspectorTabId(TEXT("ObjectNetInspector.MainTab"));

class FObjectNetInspectorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
        auto& Spawner = FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
                ObjectNetInspectorTabId,
                FOnSpawnTab::CreateStatic(&SpawnObjectNetInspectorTab))
            .SetDisplayName(FText::FromString(TEXT("Object Net Inspector")))
            .SetTooltipText(FText::FromString(TEXT("Object-level network analysis panel for Unreal Insights traces.")));

#if OBJECTNET_HAS_WORKSPACE_MENU
        const TSharedRef<FWorkspaceItem> ProfilingCategory =
            WorkspaceMenu::GetMenuStructure().GetDeveloperToolsProfilingCategory();
        Spawner.SetGroup(ProfilingCategory);
#else
        UE_LOG(LogObjectNetInspector, Verbose, TEXT("Workspace menu API unavailable; tab remains Nomad-only."));
#endif

        UE_LOG(LogObjectNetInspector, Log, TEXT("ObjectNetInspector module started and tab spawner registered."));

#if WITH_EDITOR
        UToolMenus::RegisterStartupCallback(
            FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FObjectNetInspectorModule::RegisterMenus));
#else
        // UnrealInsights program path does not expose LevelEditor menu extensions.
        // Auto-open once so users can verify the panel without relying on menu wiring.
        const TSharedPtr<SDockTab> OpenedTab = FGlobalTabmanager::Get()->TryInvokeTab(ObjectNetInspectorTabId);
        if (OpenedTab.IsValid())
        {
            UE_LOG(LogObjectNetInspector, Log, TEXT("ObjectNetInspector tab auto-opened for UnrealInsights."));
        }
        else
        {
            UE_LOG(LogObjectNetInspector, Warning, TEXT("ObjectNetInspector tab failed to auto-open in UnrealInsights."));
        }
#endif
    }

    virtual void ShutdownModule() override
    {
#if WITH_EDITOR
        if (UToolMenus* ToolMenus = UToolMenus::TryGet())
        {
            ToolMenus->UnregisterOwner(this);
        }
#endif

        if (FModuleManager::Get().IsModuleLoaded(TEXT("Slate")))
        {
            FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(ObjectNetInspectorTabId);
        }

        UE_LOG(LogObjectNetInspector, Log, TEXT("ObjectNetInspector module shutdown."));
    }

private:
#if WITH_EDITOR
    void RegisterMenus()
    {
        UToolMenu* WindowMenu = UToolMenus::Get()->ExtendMenu(TEXT("LevelEditor.MainMenu.Window"));
        if (WindowMenu == nullptr)
        {
            UE_LOG(LogObjectNetInspector, Verbose, TEXT("Failed to extend LevelEditor.MainMenu.Window."));
            return;
        }

        FToolMenuSection& Section = WindowMenu->FindOrAddSection(TEXT("ObjectNetInspector"));
        Section.AddMenuEntry(
            TEXT("OpenObjectNetInspectorTab"),
            FText::FromString(TEXT("Object Net Inspector")),
            FText::FromString(TEXT("Open Object Net Inspector analysis tab.")),
            FSlateIcon(),
            FUIAction(
                FExecuteAction::CreateLambda([]()
                {
                    FGlobalTabmanager::Get()->TryInvokeTab(ObjectNetInspectorTabId);
                })));
    }
#endif
};

IMPLEMENT_MODULE(FObjectNetInspectorModule, ObjectNetInspector)
