#include "ObjectNetInspectorTab.h"

#include "Framework/Docking/TabManager.h"
#include "Modules/ModuleManager.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"

DEFINE_LOG_CATEGORY_STATIC(LogObjectNetInspector, Log, All);

static const FName ObjectNetInspectorTabId(TEXT("ObjectNetInspector.MainTab"));

class FObjectNetInspectorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
        IWorkspaceMenuStructureModule& WorkspaceMenuStructureModule =
            FModuleManager::LoadModuleChecked<IWorkspaceMenuStructureModule>(TEXT("WorkspaceMenuStructure"));

        const TSharedRef<FWorkspaceItem> ProfilingCategory =
            WorkspaceMenuStructureModule.GetWorkspaceMenuStructure().GetDeveloperToolsProfilingCategory();

        FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
                ObjectNetInspectorTabId,
                FOnSpawnTab::CreateStatic(&SpawnObjectNetInspectorTab))
            .SetDisplayName(FText::FromString(TEXT("Object Net Inspector")))
            .SetTooltipText(FText::FromString(TEXT("Object-level network analysis panel for Unreal Insights traces.")))
            .SetGroup(ProfilingCategory);

        UE_LOG(LogObjectNetInspector, Log, TEXT("ObjectNetInspector module started and tab spawner registered."));
    }

    virtual void ShutdownModule() override
    {
        if (FModuleManager::Get().IsModuleLoaded(TEXT("Slate")))
        {
            FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(ObjectNetInspectorTabId);
        }

        UE_LOG(LogObjectNetInspector, Log, TEXT("ObjectNetInspector module shutdown."));
    }
};

IMPLEMENT_MODULE(FObjectNetInspectorModule, ObjectNetInspector)