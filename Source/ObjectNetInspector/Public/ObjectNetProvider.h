#pragma once

#include "CoreMinimal.h"
#include "Misc/Optional.h"
#include "ObjectNetAggregator.h"
#include "ObjectNetAnalyzer.h"
#include "ObjectNetQuery.h"

enum class EObjectNetDataSourceKind : uint8
{
    Unknown,
    Session,
    Mock
};

/** Thin UI-facing coordinator around reader, analyzer, and aggregator. */
class FObjectNetProvider
{
public:
    FObjectNetProvider();

    void Refresh();

    void SetQuery(const FObjectNetQuery& InQuery);
    const FObjectNetQuery& GetQuery() const;

    void SetSelectedObjectId(TOptional<uint64> InObjectId);
    TOptional<uint64> GetSelectedObjectId() const;

    const TArray<FObjectNetAggregate>& GetCurrentAggregates() const;
    TOptional<FObjectNetAggregate> GetSelectedAggregate() const;
    TArray<FObjectNetEvent> GetSelectedObjectEvents() const;

    EObjectNetDataSourceKind GetLastDataSourceKind() const;
    FString GetLastDataSourceLabel() const;
    int32 GetLastEventCount() const;
    int32 GetLastUnknownEventCount() const;
    double GetLastUnknownRatio() const;
    int32 GetLastPacketRefEventCount() const;
    double GetLastPacketRefRatio() const;
    uint64 GetViewRevision() const;

    FObjectNetTraceReader& GetReader();
    const FObjectNetTraceReader& GetReader() const;

private:
    void RebuildViewState();
    void InvalidateSelectionCaches();

    FObjectNetTraceReader TraceReader;
    FObjectNetAnalyzer Analyzer;
    FObjectNetAggregator Aggregator;

    FObjectNetQuery CurrentQuery;
    TOptional<uint64> SelectedObjectId;
    TArray<FObjectNetAggregate> CurrentAggregates;
    mutable TOptional<FObjectNetAggregate> CachedSelectedAggregate;
    mutable TArray<FObjectNetEvent> CachedSelectedEvents;
    mutable bool bSelectedAggregateCacheValid;
    mutable bool bSelectedEventsCacheValid;
    EObjectNetDataSourceKind LastDataSourceKind;
    int32 LastEventCount;
    int32 LastUnknownEventCount;
    int32 LastPacketRefEventCount;
    uint64 ViewRevision;
};
