#include "ObjectNetAggregator.h"

#include "ObjectNetAnalyzer.h"

TArray<FObjectNetAggregate> FObjectNetAggregator::BuildAggregates(const FObjectNetAnalyzer& Analyzer, const FObjectNetQuery& Query) const
{
    TArray<FObjectNetAggregate> Aggregates;
    const TArray<FObjectNetEvent>& Events = Analyzer.GetEvents();

    for (const TPair<uint64, TArray<int32>>& Pair : Analyzer.GetEventIndicesByObject())
    {
        FObjectNetAggregate Aggregate;

        for (const int32 EventIndex : Pair.Value)
        {
            if (!Events.IsValidIndex(EventIndex))
            {
                continue;
            }

            const FObjectNetEvent& Event = Events[EventIndex];
            if (!Query.PassesEvent(Event))
            {
                continue;
            }

            Aggregate.AddEvent(Event);
        }

        if (Aggregate.TotalEventCount > 0 && Query.PassesAggregate(Aggregate))
        {
            Aggregates.Add(MoveTemp(Aggregate));
        }
    }

    SortAggregates(Aggregates);
    return Aggregates;
}

TOptional<FObjectNetAggregate> FObjectNetAggregator::BuildAggregateForObject(
    const uint64 ObjectId,
    const FObjectNetAnalyzer& Analyzer,
    const FObjectNetQuery& Query) const
{
    const TArray<FObjectNetEvent>* Events = Analyzer.FindEventsByObjectId(ObjectId);
    if (Events == nullptr)
    {
        return TOptional<FObjectNetAggregate>();
    }

    FObjectNetAggregate Aggregate;
    for (const FObjectNetEvent& Event : *Events)
    {
        if (Query.PassesEvent(Event))
        {
            Aggregate.AddEvent(Event);
        }
    }

    if (Aggregate.TotalEventCount == 0 || !Query.PassesAggregate(Aggregate))
    {
        return TOptional<FObjectNetAggregate>();
    }

    return TOptional<FObjectNetAggregate>(MoveTemp(Aggregate));
}

void FObjectNetAggregator::SortAggregates(TArray<FObjectNetAggregate>& Aggregates)
{
    Aggregates.Sort([](const FObjectNetAggregate& A, const FObjectNetAggregate& B)
    {
        if (A.TotalKnownBits != B.TotalKnownBits)
        {
            return A.TotalKnownBits > B.TotalKnownBits;
        }

        if (A.TotalEventCount != B.TotalEventCount)
        {
            return A.TotalEventCount > B.TotalEventCount;
        }

        return A.ObjectName < B.ObjectName;
    });
}
