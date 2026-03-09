#include "Modules/ModuleManager.h"
#include "ObjectNetProvider.h"

DEFINE_LOG_CATEGORY_STATIC(LogObjectNetInspector, Log, All);

class FObjectNetInspectorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
        Provider = MakeUnique<FObjectNetProvider>();
        Provider->Refresh();

        // TODO: Register an Insights tab spawner and wire it to SObjectNetInspectorTab.
        // Keep registration in this module once exact Insights extension points are verified.
        UE_LOG(LogObjectNetInspector, Log, TEXT("ObjectNetInspector module started (mock-ready)."));
    }

    virtual void ShutdownModule() override
    {
        // TODO: Unregister Insights tab spawner when registration is implemented.
        Provider.Reset();
        UE_LOG(LogObjectNetInspector, Log, TEXT("ObjectNetInspector module shutdown."));
    }

private:
    TUniquePtr<FObjectNetProvider> Provider;
};

IMPLEMENT_MODULE(FObjectNetInspectorModule, ObjectNetInspector)
