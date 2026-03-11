#include "ObjectNetProvider.h"

FObjectNetProvider::FObjectNetProvider()
    : bSelectedAggregateCacheValid(false)
    , bSelectedEventsCacheValid(false)
    , LastDataSourceKind(EObjectNetDataSourceKind::Unknown)
    , LastEventCount(0)
    , LastUnknownEventCount(0)
    , LastPacketRefEventCount(0)
    , ViewRevision(0)
{
}

void FObjectNetProvider::Refresh()
{
    const bool bInitializedFromSession = TraceReader.InitializeFromActiveSession();
    if (!bInitializedFromSession)
    {
        TraceReader.LoadMockDataForTesting();
        LastDataSourceKind = EObjectNetDataSourceKind::Mock;
    }
    else
    {
        LastDataSourceKind = EObjectNetDataSourceKind::Session;
    }

    Analyzer.Rebuild(TraceReader.GetEvents());
    InvalidateSelectionCaches();

    LastEventCount = Analyzer.GetEvents().Num();
    LastUnknownEventCount = 0;
    LastPacketRefEventCount = 0;
    for (const FObjectNetEvent& Event : Analyzer.GetEvents())
    {
        if (Event.Kind == EObjectNetEventKind::Unknown)
        {
            ++LastUnknownEventCount;
        }
        else if (Event.Kind == EObjectNetEventKind::PacketRef)
        {
            ++LastPacketRefEventCount;
        }
    }

    RebuildViewState();
}

void FObjectNetProvider::SetQuery(const FObjectNetQuery& InQuery)
{
    CurrentQuery = InQuery;
    RebuildViewState();
}

const FObjectNetQuery& FObjectNetProvider::GetQuery() const
{
    return CurrentQuery;
}

void FObjectNetProvider::SetSelectedObjectId(const TOptional<uint64> InObjectId)
{
    if (SelectedObjectId == InObjectId)
    {
        return;
    }

    SelectedObjectId = InObjectId;
    InvalidateSelectionCaches();
    ++ViewRevision;
}

TOptional<uint64> FObjectNetProvider::GetSelectedObjectId() const
{
    return SelectedObjectId;
}

const TArray<FObjectNetAggregate>& FObjectNetProvider::GetCurrentAggregates() const
{
    return CurrentAggregates;
}

TOptional<FObjectNetAggregate> FObjectNetProvider::GetSelectedAggregate() const
{
    if (!SelectedObjectId.IsSet())
    {
        return TOptional<FObjectNetAggregate>();
    }

    if (!bSelectedAggregateCacheValid)
    {
        CachedSelectedAggregate = Aggregator.BuildAggregateForObject(SelectedObjectId.GetValue(), Analyzer, CurrentQuery);
        bSelectedAggregateCacheValid = true;
    }

    return CachedSelectedAggregate;
}

TArray<FObjectNetEvent> FObjectNetProvider::GetSelectedObjectEvents() const
{
    if (!SelectedObjectId.IsSet())
    {
        return TArray<FObjectNetEvent>();
    }

    if (!bSelectedEventsCacheValid)
    {
        CachedSelectedEvents.Reset();

        const TArray<FObjectNetEvent>* SourceEvents = Analyzer.FindEventsByObjectId(SelectedObjectId.GetValue());
        if (SourceEvents != nullptr)
        {
            CachedSelectedEvents.Reserve(SourceEvents->Num());
            for (const FObjectNetEvent& Event : *SourceEvents)
            {
                if (CurrentQuery.PassesEvent(Event))
                {
                    CachedSelectedEvents.Add(Event);
                }
            }
        }

        CachedSelectedEvents.Sort([](const FObjectNetEvent& A, const FObjectNetEvent& B)
        {
            return A.TimeSec < B.TimeSec;
        });

        bSelectedEventsCacheValid = true;
    }

    return CachedSelectedEvents;
}

EObjectNetDataSourceKind FObjectNetProvider::GetLastDataSourceKind() const
{
    return LastDataSourceKind;
}

FString FObjectNetProvider::GetLastDataSourceLabel() const
{
    switch (LastDataSourceKind)
    {
    case EObjectNetDataSourceKind::Session:
        return TEXT("Session");
    case EObjectNetDataSourceKind::Mock:
        return TEXT("Mock");
    default:
        return TEXT("Unknown");
    }
}

int32 FObjectNetProvider::GetLastEventCount() const
{
    return LastEventCount;
}

int32 FObjectNetProvider::GetLastUnknownEventCount() const
{
    return LastUnknownEventCount;
}

double FObjectNetProvider::GetLastUnknownRatio() const
{
    if (LastEventCount <= 0)
    {
        return 0.0;
    }

    return static_cast<double>(LastUnknownEventCount) / static_cast<double>(LastEventCount);
}

int32 FObjectNetProvider::GetLastPacketRefEventCount() const
{
    return LastPacketRefEventCount;
}

double FObjectNetProvider::GetLastPacketRefRatio() const
{
    if (LastEventCount <= 0)
    {
        return 0.0;
    }

    return static_cast<double>(LastPacketRefEventCount) / static_cast<double>(LastEventCount);
}

uint64 FObjectNetProvider::GetViewRevision() const
{
    return ViewRevision;
}

FObjectNetTraceReader& FObjectNetProvider::GetReader()
{
    return TraceReader;
}

const FObjectNetTraceReader& FObjectNetProvider::GetReader() const
{
    return TraceReader;
}

void FObjectNetProvider::RebuildViewState()
{
    CurrentAggregates = Aggregator.BuildAggregates(Analyzer, CurrentQuery);
    InvalidateSelectionCaches();

    if (SelectedObjectId.IsSet())
    {
        const TOptional<FObjectNetAggregate> SelectedAggregate = Aggregator.BuildAggregateForObject(
            SelectedObjectId.GetValue(),
            Analyzer,
            CurrentQuery);

        if (!SelectedAggregate.IsSet())
        {
            SelectedObjectId.Reset();
            InvalidateSelectionCaches();
        }
    }

    ++ViewRevision;
}

void FObjectNetProvider::InvalidateSelectionCaches()
{
    CachedSelectedAggregate.Reset();
    CachedSelectedEvents.Reset();
    bSelectedAggregateCacheValid = false;
    bSelectedEventsCacheValid = false;
}
