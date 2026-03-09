#include "ObjectNetProvider.h"

FObjectNetProvider::FObjectNetProvider()
{
}

void FObjectNetProvider::Refresh()
{
    if (TraceReader.GetEvents().Num() == 0)
    {
        const bool bInitializedFromSession = TraceReader.InitializeFromActiveSession();
        if (!bInitializedFromSession)
        {
            TraceReader.LoadMockDataForTesting();
        }
    }

    Analyzer.Rebuild(TraceReader.GetEvents());
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

FObjectNetTraceReader& FObjectNetProvider::GetReader()
{
    return TraceReader;
}

const FObjectNetTraceReader& FObjectNetProvider::GetReader() const
{
    return TraceReader;
}
