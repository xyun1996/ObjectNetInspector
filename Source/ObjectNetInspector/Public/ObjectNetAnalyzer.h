#pragma once

#include "CoreMinimal.h"
#include "ObjectNetQuery.h"
#include "ObjectNetTypes.h"

/**
 * Adapter for obtaining object-level network events from the active trace session.
 * TODO: Hook this into verified Insights/Trace APIs once endpoint names are finalized.
 */
class FObjectNetTraceReader
{
public:
    bool InitializeFromActiveSession();
    void Reset();
    void LoadMockDataForTesting();

    const TArray<FObjectNetEvent>& GetEvents() const;

private:
    TArray<FObjectNetEvent> Events;
};

/**
 * Normalizes and indexes network events for fast object-level queries.
 */
class FObjectNetAnalyzer
{
public:
    void Rebuild(const TArray<FObjectNetEvent>& InEvents);

    const TArray<FObjectNetEvent>* FindEventsByObjectId(uint64 ObjectId) const;
    TArray<uint64> FindMatchingObjectIds(const FObjectNetQuery& Query) const;

    const TArray<FObjectNetEvent>& GetEvents() const;
    const TMap<uint64, TArray<int32>>& GetEventIndicesByObject() const;

private:
    TArray<FObjectNetEvent> Events;
    TMap<uint64, TArray<int32>> EventIndicesByObject;
    TMap<uint32, TArray<int32>> EventIndicesByConnection;
    TMap<uint32, TArray<int32>> EventIndicesByPacket;
    TMap<uint64, TArray<FObjectNetEvent>> EventsByObject;
};
