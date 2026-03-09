#include "ObjectNetInsightsBridge.h"

#include "Modules/ModuleManager.h"
#include "ObjectNetTypes.h"

DEFINE_LOG_CATEGORY_STATIC(LogObjectNetInsightsBridge, Log, All);

bool FObjectNetInsightsBridge::TryReadActiveSession(TArray<FObjectNetEvent>& OutEvents)
{
    OutEvents.Reset();

    const bool bTraceInsightsLoaded = FModuleManager::Get().IsModuleLoaded(TEXT("TraceInsights"));
    const bool bNetworkingInsightsLoaded = FModuleManager::Get().IsModuleLoaded(TEXT("NetworkingInsights"));

    // TODO: Replace this stub with verified APIs to resolve active session + net trace providers
    // and translate RPC/property events into FObjectNetEvent.
    UE_LOG(
        LogObjectNetInsightsBridge,
        Verbose,
        TEXT("Bridge stub called. TraceInsightsLoaded=%s NetworkingInsightsLoaded=%s"),
        bTraceInsightsLoaded ? TEXT("true") : TEXT("false"),
        bNetworkingInsightsLoaded ? TEXT("true") : TEXT("false"));

    return false;
}
