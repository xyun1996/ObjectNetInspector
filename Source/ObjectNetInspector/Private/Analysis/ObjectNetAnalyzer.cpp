#include "ObjectNetAnalyzer.h"

void FObjectNetAnalyzer::Rebuild(const TArray<FObjectNetEvent>& InEvents)
{
    Events = InEvents;
    Events.Sort([](const FObjectNetEvent& A, const FObjectNetEvent& B)
    {
        return A.TimeSec < B.TimeSec;
    });

    EventIndicesByObject.Reset();
    EventIndicesByConnection.Reset();
    EventIndicesByPacket.Reset();
    EventsByObject.Reset();

    for (int32 Index = 0; Index < Events.Num(); ++Index)
    {
        const FObjectNetEvent& Event = Events[Index];

        EventIndicesByObject.FindOrAdd(Event.ObjectId).Add(Index);
        EventIndicesByConnection.FindOrAdd(Event.ConnectionId).Add(Index);

        if (Event.PacketId != 0)
        {
            EventIndicesByPacket.FindOrAdd(Event.PacketId).Add(Index);
        }

        EventsByObject.FindOrAdd(Event.ObjectId).Add(Event);
    }
}

const TArray<FObjectNetEvent>* FObjectNetAnalyzer::FindEventsByObjectId(const uint64 ObjectId) const
{
    return EventsByObject.Find(ObjectId);
}

TArray<uint64> FObjectNetAnalyzer::FindMatchingObjectIds(const FObjectNetQuery& Query) const
{
    TArray<uint64> Result;
    Result.Reserve(EventsByObject.Num());

    for (const TPair<uint64, TArray<FObjectNetEvent>>& Pair : EventsByObject)
    {
        const uint64 ObjectId = Pair.Key;
        const TArray<FObjectNetEvent>& ObjectEvents = Pair.Value;

        bool bAnyMatch = false;
        for (const FObjectNetEvent& Event : ObjectEvents)
        {
            if (Query.PassesEvent(Event))
            {
                bAnyMatch = true;
                break;
            }
        }

        if (bAnyMatch)
        {
            Result.Add(ObjectId);
        }
    }

    return Result;
}

const TArray<FObjectNetEvent>& FObjectNetAnalyzer::GetEvents() const
{
    return Events;
}

const TMap<uint64, TArray<int32>>& FObjectNetAnalyzer::GetEventIndicesByObject() const
{
    return EventIndicesByObject;
}
