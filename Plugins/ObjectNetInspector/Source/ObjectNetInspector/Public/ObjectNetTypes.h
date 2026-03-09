#pragma once

#include "CoreMinimal.h"
#include "Misc/Optional.h"

/** Event category for object network analysis. */
enum class EObjectNetEventKind : uint8
{
    Rpc,
    Property,
    PacketRef,
    Unknown
};

/** Network direction from the perspective of this captured endpoint. */
enum class EObjectNetDirection : uint8
{
    Incoming,
    Outgoing,
    Unknown
};

/** Single network-related event that can be attributed to an object. */
struct FObjectNetEvent
{
    double TimeSec;
    uint32 ConnectionId;
    uint64 ObjectId;
    FString ObjectName;
    FString ObjectPath;
    FString ClassName;
    EObjectNetEventKind Kind;
    EObjectNetDirection Direction;
    FString EventName;
    uint32 PacketId;
    TOptional<uint64> BitCount;

    FObjectNetEvent()
        : TimeSec(0.0)
        , ConnectionId(0)
        , ObjectId(0)
        , Kind(EObjectNetEventKind::Unknown)
        , Direction(EObjectNetDirection::Unknown)
        , PacketId(0)
    {
    }

    bool HasKnownBits() const
    {
        return BitCount.IsSet();
    }

    uint64 GetKnownBytesRoundedUp() const
    {
        return BitCount.IsSet() ? ((BitCount.GetValue() + 7ull) / 8ull) : 0ull;
    }

    FString ToDebugString() const
    {
        const TCHAR* KindText = TEXT("Unknown");
        switch (Kind)
        {
        case EObjectNetEventKind::Rpc:
            KindText = TEXT("Rpc");
            break;
        case EObjectNetEventKind::Property:
            KindText = TEXT("Property");
            break;
        case EObjectNetEventKind::PacketRef:
            KindText = TEXT("PacketRef");
            break;
        default:
            break;
        }

        const TCHAR* DirectionText = TEXT("Unknown");
        switch (Direction)
        {
        case EObjectNetDirection::Incoming:
            DirectionText = TEXT("In");
            break;
        case EObjectNetDirection::Outgoing:
            DirectionText = TEXT("Out");
            break;
        default:
            break;
        }

        const FString BitsText = BitCount.IsSet() ? FString::Printf(TEXT("%llu"), BitCount.GetValue()) : TEXT("N/A");
        return FString::Printf(
            TEXT("T=%.3f Obj=%llu Conn=%u Kind=%s Dir=%s Event=%s Packet=%u Bits=%s"),
            TimeSec,
            ObjectId,
            ConnectionId,
            KindText,
            DirectionText,
            *EventName,
            PacketId,
            *BitsText);
    }
};

/** Aggregated network view for one object under a query scope. */
struct FObjectNetAggregate
{
    uint64 ObjectId;
    FString ObjectName;
    FString ObjectPath;
    FString ClassName;
    uint32 TotalEventCount;
    uint32 RpcCount;
    uint32 PropertyCount;
    uint32 PacketHitCount;
    uint64 TotalKnownBits;
    TMap<FString, uint32> RpcCounts;
    TMap<FString, uint32> PropertyCounts;
    TMap<uint32, uint64> BitsByConnection;
    double FirstSeenTime;
    double LastSeenTime;

    FObjectNetAggregate()
        : ObjectId(0)
        , TotalEventCount(0)
        , RpcCount(0)
        , PropertyCount(0)
        , PacketHitCount(0)
        , TotalKnownBits(0)
        , FirstSeenTime(TNumericLimits<double>::Max())
        , LastSeenTime(TNumericLimits<double>::Lowest())
    {
    }

    void AddEvent(const FObjectNetEvent& Event)
    {
        if (ObjectId == 0)
        {
            ObjectId = Event.ObjectId;
            ObjectName = Event.ObjectName;
            ObjectPath = Event.ObjectPath;
            ClassName = Event.ClassName;
        }

        ++TotalEventCount;

        if (Event.PacketId != 0)
        {
            ++PacketHitCount;
        }

        if (Event.Kind == EObjectNetEventKind::Rpc)
        {
            ++RpcCount;
            const FString& Key = Event.EventName.IsEmpty() ? TEXT("(UnnamedRpc)") : Event.EventName;
            RpcCounts.FindOrAdd(Key)++;
        }
        else if (Event.Kind == EObjectNetEventKind::Property)
        {
            ++PropertyCount;
            const FString& Key = Event.EventName.IsEmpty() ? TEXT("(UnnamedProperty)") : Event.EventName;
            PropertyCounts.FindOrAdd(Key)++;
        }

        if (Event.BitCount.IsSet())
        {
            const uint64 Bits = Event.BitCount.GetValue();
            TotalKnownBits += Bits;
            BitsByConnection.FindOrAdd(Event.ConnectionId) += Bits;
        }

        FirstSeenTime = FMath::Min(FirstSeenTime, Event.TimeSec);
        LastSeenTime = FMath::Max(LastSeenTime, Event.TimeSec);
    }

    bool HasKnownBits() const
    {
        return TotalKnownBits > 0;
    }
};
