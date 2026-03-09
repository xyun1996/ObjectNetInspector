#pragma once

#include "CoreMinimal.h"

struct FObjectNetEvent;

/**
 * Thin adapter boundary for reading object-level events from the active Insights session.
 * Keep all uncertain engine-version-specific API usage inside this bridge.
 */
class FObjectNetInsightsBridge
{
public:
    static bool TryReadActiveSession(TArray<FObjectNetEvent>& OutEvents);
};
