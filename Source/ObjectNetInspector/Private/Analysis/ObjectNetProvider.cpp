#include "ObjectNetProvider.h"

FObjectNetProvider::FObjectNetProvider()
    : LastDataSourceKind(EObjectNetDataSourceKind::Unknown)
    , LastEventCount(0)
    , LastUnknownEventCount(0)
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
    LastEventCount = Analyzer.GetEvents().Num();
    LastUnknownEventCount = 0;
    for (const FObjectNetEvent& Event : Analyzer.GetEvents())
    {
        if (Event.Kind == EObjectNetEventKind::Unknown)
        {
            ++LastUnknownEventCount;
        }
    }
    CurrentAggregates = Aggregator.BuildAggregates(Analyzer, CurrentQuery);

    if (SelectedObjectId.IsSet())
    {
        const TOptional<FObjectNetAggregate> SelectedAggregate = Aggregator.BuildAggregateForObject(
            SelectedObjectId.GetValue(),
            Analyzer,
            CurrentQuery);

        if (!SelectedAggregate.IsSet())
        {
            SelectedObjectId.Reset();
        }
    }
}

void FObjectNetProvider::SetQuery(const FObjectNetQuery& InQuery)
{
    CurrentQuery = InQuery;
    Refresh();
}

const FObjectNetQuery& FObjectNetProvider::GetQuery() const
{
    return CurrentQuery;
}

void FObjectNetProvider::SetSelectedObjectId(const TOptional<uint64> InObjectId)
{
    SelectedObjectId = InObjectId;
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

    return Aggregator.BuildAggregateForObject(SelectedObjectId.GetValue(), Analyzer, CurrentQuery);
}

TArray<FObjectNetEvent> FObjectNetProvider::GetSelectedObjectEvents() const
{
    TArray<FObjectNetEvent> FilteredEvents;
    if (!SelectedObjectId.IsSet())
    {
        return FilteredEvents;
    }

    const TArray<FObjectNetEvent>* SourceEvents = Analyzer.FindEventsByObjectId(SelectedObjectId.GetValue());
    if (SourceEvents == nullptr)
    {
        return FilteredEvents;
    }

    FilteredEvents.Reserve(SourceEvents->Num());
    for (const FObjectNetEvent& Event : *SourceEvents)
    {
        if (CurrentQuery.PassesEvent(Event))
        {
            FilteredEvents.Add(Event);
        }
    }

    FilteredEvents.Sort([](const FObjectNetEvent& A, const FObjectNetEvent& B)
    {
        return A.TimeSec < B.TimeSec;
    });

    return FilteredEvents;
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

FObjectNetTraceReader& FObjectNetProvider::GetReader()
{
    return TraceReader;
}

const FObjectNetTraceReader& FObjectNetProvider::GetReader() const
{
    return TraceReader;
}
