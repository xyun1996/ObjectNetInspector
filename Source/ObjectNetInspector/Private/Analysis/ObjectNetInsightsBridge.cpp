#include "ObjectNetInsightsBridge.h"

#include "Insights/IUnrealInsightsModule.h"
#include "Modules/ModuleManager.h"
#include "ObjectNetTypes.h"
#include "TraceServices/Model/AnalysisSession.h"
#include "TraceServices/Model/NetProfiler.h"

DEFINE_LOG_CATEGORY_STATIC(LogObjectNetInsightsBridge, Log, All);

namespace
{
struct FBridgeEventTypeInfo
{
    FString Name;
    uint16 Level = 0;
};

struct FBridgeObjectInfo
{
    uint64 NetObjectId = 0;
    uint64 TypeId = 0;
    FString Name;
};

static uint64 MakeObjectCacheKey(const uint32 GameInstanceIndex, const uint32 ObjectIndex)
{
    return (static_cast<uint64>(GameInstanceIndex) << 32ull) | static_cast<uint64>(ObjectIndex);
}

static EObjectNetDirection ToDirection(const TraceServices::ENetProfilerConnectionMode Mode)
{
    switch (Mode)
    {
    case TraceServices::Incoming:
        return EObjectNetDirection::Incoming;
    case TraceServices::Outgoing:
        return EObjectNetDirection::Outgoing;
    default:
        return EObjectNetDirection::Unknown;
    }
}

static bool ContainsAny(const FString& Text, const TCHAR* const* Terms, const int32 TermCount)
{
    const ESearchCase::Type SearchCase = ESearchCase::IgnoreCase;
    for (int32 Index = 0; Index < TermCount; ++Index)
    {
        if (Text.Contains(Terms[Index], SearchCase))
        {
            return true;
        }
    }

    return false;
}

static EObjectNetEventKind GuessEventKind(const FString& EventName, const uint16 EventTypeLevel, const uint8 ContentLevel)
{
    static const TCHAR* RpcTerms[] =
    {
        TEXT("rpc"),
        TEXT("function"),
        TEXT("netmulticast"),
        TEXT("server"),
        TEXT("client")
    };

    static const TCHAR* PropertyTerms[] =
    {
        TEXT("property"),
        TEXT("prop"),
        TEXT("rep"),
        TEXT("state"),
        TEXT("delta"),
        TEXT("array"),
        TEXT("serializer")
    };

    if (ContainsAny(EventName, RpcTerms, UE_ARRAY_COUNT(RpcTerms)))
    {
        return EObjectNetEventKind::Rpc;
    }

    if (ContainsAny(EventName, PropertyTerms, UE_ARRAY_COUNT(PropertyTerms)))
    {
        return EObjectNetEventKind::Property;
    }

    // Level-only fallback: bias to property payload events while preserving Unknown for unclassified names.
    if (EventTypeLevel >= 2 || ContentLevel >= 2)
    {
        return EObjectNetEventKind::Property;
    }

    return EObjectNetEventKind::Unknown;
}

static uint32 MakePacketId(const uint32 PacketSequence, const uint32 PacketIndex)
{
    const uint64 Candidate = static_cast<uint64>(PacketSequence) + 1ull;
    if (Candidate > static_cast<uint64>(TNumericLimits<uint32>::Max()))
    {
        return PacketIndex + 1u;
    }

    return static_cast<uint32>(Candidate);
}
} // namespace

bool FObjectNetInsightsBridge::TryReadActiveSession(TArray<FObjectNetEvent>& OutEvents)
{
    OutEvents.Reset();

    IUnrealInsightsModule* UnrealInsightsModule = FModuleManager::GetModulePtr<IUnrealInsightsModule>(TEXT("TraceInsights"));
    if (UnrealInsightsModule == nullptr)
    {
        UnrealInsightsModule = FModuleManager::LoadModulePtr<IUnrealInsightsModule>(TEXT("TraceInsights"));
    }

    if (UnrealInsightsModule == nullptr)
    {
        UE_LOG(LogObjectNetInsightsBridge, Verbose, TEXT("TraceInsights module unavailable."));
        return false;
    }

    const TSharedPtr<const TraceServices::IAnalysisSession> AnalysisSession = UnrealInsightsModule->GetAnalysisSession();
    if (!AnalysisSession.IsValid())
    {
        UE_LOG(LogObjectNetInsightsBridge, Verbose, TEXT("No active analysis session."));
        return false;
    }

    double DurationSec = 0.0;
    double BaseDateTimeUnixSec = 0.0;
    uint32 NetTraceVersion = 0;
    uint32 GameInstanceCount = 0;
    uint32 TotalConnectionCount = 0;
    uint32 TotalObjectCount = 0;
    uint32 MappedContentEventCount = 0;
    uint32 RpcMappedCount = 0;
    uint32 PropertyMappedCount = 0;
    uint32 UnknownMappedCount = 0;

    {
        TraceServices::FAnalysisSessionReadScope SessionReadScope(*AnalysisSession);

        DurationSec = AnalysisSession->GetDurationSeconds();
        BaseDateTimeUnixSec = AnalysisSession->GetBaseDateTime();

        const TraceServices::INetProfilerProvider* NetProvider = TraceServices::ReadNetProfilerProvider(*AnalysisSession.Get());
        if (NetProvider != nullptr)
        {
            NetTraceVersion = NetProvider->GetNetTraceVersion();
            GameInstanceCount = NetProvider->GetGameInstanceCount();

            TMap<uint32, FString> NameByIndex;
            NetProvider->ReadNames(
                [&NameByIndex](const TraceServices::FNetProfilerName* Names, const uint64 NameCount)
                {
                    if (Names == nullptr)
                    {
                        return;
                    }

                    for (uint64 NameArrayIndex = 0; NameArrayIndex < NameCount; ++NameArrayIndex)
                    {
                        const TraceServices::FNetProfilerName& NameEntry = Names[NameArrayIndex];
                        const FString NameText = NameEntry.Name != nullptr ? FString(NameEntry.Name) : FString();
                        if (!NameText.IsEmpty())
                        {
                            NameByIndex.Add(NameEntry.NameIndex, NameText);
                        }
                    }
                });

            TMap<uint32, FBridgeEventTypeInfo> EventTypeByIndex;
            NetProvider->ReadEventTypes(
                [&EventTypeByIndex, &NameByIndex](const TraceServices::FNetProfilerEventType* EventTypes, const uint64 EventTypeCount)
                {
                    if (EventTypes == nullptr)
                    {
                        return;
                    }

                    for (uint64 EventTypeArrayIndex = 0; EventTypeArrayIndex < EventTypeCount; ++EventTypeArrayIndex)
                    {
                        const TraceServices::FNetProfilerEventType& EventType = EventTypes[EventTypeArrayIndex];

                        FBridgeEventTypeInfo Info;
                        if (const FString* Name = NameByIndex.Find(EventType.NameIndex))
                        {
                            Info.Name = *Name;
                        }
                        else if (EventType.Name != nullptr)
                        {
                            Info.Name = EventType.Name;
                        }
                        Info.Level = EventType.Level;

                        EventTypeByIndex.Add(EventType.EventTypeIndex, MoveTemp(Info));
                    }
                });

            TMap<uint64, FBridgeObjectInfo> ObjectByKey;
            for (uint32 GameInstanceIndex = 0; GameInstanceIndex < GameInstanceCount; ++GameInstanceIndex)
            {
                const uint32 ObjectCountForInstance = NetProvider->GetObjectCount(GameInstanceIndex);
                TotalObjectCount += ObjectCountForInstance;

                NetProvider->ReadObjects(
                    GameInstanceIndex,
                    [&ObjectByKey, &NameByIndex, GameInstanceIndex](const TraceServices::FNetProfilerObjectInstance& Object)
                    {
                        FBridgeObjectInfo Info;
                        Info.NetObjectId = Object.NetObjectId;
                        Info.TypeId = Object.TypeId;

                        if (const FString* Name = NameByIndex.Find(Object.NameIndex))
                        {
                            Info.Name = *Name;
                        }

                        ObjectByKey.Add(MakeObjectCacheKey(GameInstanceIndex, Object.ObjectIndex), MoveTemp(Info));
                    });

                NetProvider->ReadConnections(
                    GameInstanceIndex,
                    [
                        NetProvider,
                        &OutEvents,
                        &MappedContentEventCount,
                        &RpcMappedCount,
                        &PropertyMappedCount,
                        &UnknownMappedCount,
                        &NameByIndex,
                        &EventTypeByIndex,
                        &ObjectByKey,
                        &TotalConnectionCount,
                        GameInstanceIndex](const TraceServices::FNetProfilerConnection& Connection)
                    {
                        ++TotalConnectionCount;

                        const TraceServices::ENetProfilerConnectionMode Modes[] =
                        {
                            TraceServices::Outgoing,
                            TraceServices::Incoming
                        };

                        for (const TraceServices::ENetProfilerConnectionMode Mode : Modes)
                        {
                            const bool bHasModeData =
                                (Mode == TraceServices::Outgoing && Connection.bHasOutgoingData != 0) ||
                                (Mode == TraceServices::Incoming && Connection.bHasIncomingData != 0);
                            if (!bHasModeData)
                            {
                                continue;
                            }

                            const uint32 PacketCount = NetProvider->GetPacketCount(Connection.ConnectionIndex, Mode);
                            if (PacketCount == 0)
                            {
                                continue;
                            }

                            uint32 PacketIndex = 0;
                            NetProvider->EnumeratePackets(
                                Connection.ConnectionIndex,
                                Mode,
                                0,
                                PacketCount - 1,
                                [
                                    NetProvider,
                                    &OutEvents,
                                    &MappedContentEventCount,
                                    &RpcMappedCount,
                                    &PropertyMappedCount,
                                    &UnknownMappedCount,
                                    &NameByIndex,
                                    &EventTypeByIndex,
                                    &ObjectByKey,
                                    GameInstanceIndex,
                                    Connection,
                                    Mode,
                                    &PacketIndex](const TraceServices::FNetProfilerPacket& Packet)
                                {
                                    const uint32 ThisPacketIndex = PacketIndex++;

                                    if (Packet.EventCount == 0)
                                    {
                                        return;
                                    }

                                    const double PacketTimeSec = Packet.TimeStamp;
                                    const uint32 PacketEventStartIndex = Packet.StartEventIndex;
                                    const uint32 PacketEventEndIndex = Packet.StartEventIndex + Packet.EventCount - 1u;
                                    const uint32 PacketId = MakePacketId(Packet.SequenceNumber, ThisPacketIndex);

                                    NetProvider->EnumeratePacketContentEventsByIndex(
                                        Connection.ConnectionIndex,
                                        Mode,
                                        PacketEventStartIndex,
                                        PacketEventEndIndex,
                                        [
                                            &OutEvents,
                                            &MappedContentEventCount,
                                            &RpcMappedCount,
                                            &PropertyMappedCount,
                                            &UnknownMappedCount,
                                            &NameByIndex,
                                            &EventTypeByIndex,
                                            &ObjectByKey,
                                            GameInstanceIndex,
                                            Connection,
                                            Mode,
                                            PacketTimeSec,
                                            PacketId](const TraceServices::FNetProfilerContentEvent& ContentEvent)
                                        {
                                            const uint64 ObjectKey = MakeObjectCacheKey(GameInstanceIndex, ContentEvent.ObjectInstanceIndex);
                                            const FBridgeObjectInfo* ObjectInfo = ObjectByKey.Find(ObjectKey);
                                            if (ObjectInfo == nullptr)
                                            {
                                                return;
                                            }

                                            const FBridgeEventTypeInfo* EventTypeInfo = EventTypeByIndex.Find(ContentEvent.EventTypeIndex);

                                            FString EventName;
                                            if (EventTypeInfo != nullptr && !EventTypeInfo->Name.IsEmpty())
                                            {
                                                EventName = EventTypeInfo->Name;
                                            }
                                            else if (const FString* ContentName = NameByIndex.Find(ContentEvent.NameIndex))
                                            {
                                                EventName = *ContentName;
                                            }
                                            else
                                            {
                                                EventName = TEXT("(UnknownEvent)");
                                            }

                                            const uint16 EventLevel = EventTypeInfo != nullptr ? EventTypeInfo->Level : 0;
                                            const EObjectNetEventKind Kind = GuessEventKind(EventName, EventLevel, static_cast<uint8>(ContentEvent.Level));

                                            FObjectNetEvent Event;
                                            Event.TimeSec = PacketTimeSec;
                                            Event.ConnectionId = Connection.ConnectionId;
                                            Event.ObjectId = ObjectInfo->NetObjectId != 0ull
                                                ? ObjectInfo->NetObjectId
                                                : MakeObjectCacheKey(GameInstanceIndex, ContentEvent.ObjectInstanceIndex);
                                            Event.ObjectName = !ObjectInfo->Name.IsEmpty()
                                                ? ObjectInfo->Name
                                                : FString::Printf(TEXT("Object_%u"), ContentEvent.ObjectInstanceIndex);
                                            Event.ObjectPath = FString();
                                            Event.ClassName = ObjectInfo->TypeId != 0ull
                                                ? FString::Printf(TEXT("TypeId_0x%llX"), ObjectInfo->TypeId)
                                                : FString();
                                            Event.Kind = Kind;
                                            Event.Direction = ToDirection(Mode);
                                            Event.EventName = MoveTemp(EventName);
                                            Event.PacketId = PacketId;

                                            if (ContentEvent.EndPos > ContentEvent.StartPos)
                                            {
                                                Event.BitCount = static_cast<uint64>(ContentEvent.EndPos - ContentEvent.StartPos);
                                            }

                                            switch (Kind)
                                            {
                                            case EObjectNetEventKind::Rpc:
                                                ++RpcMappedCount;
                                                break;
                                            case EObjectNetEventKind::Property:
                                                ++PropertyMappedCount;
                                                break;
                                            default:
                                                ++UnknownMappedCount;
                                                break;
                                            }

                                            OutEvents.Add(MoveTemp(Event));
                                            ++MappedContentEventCount;
                                        });
                                });
                        }
                    });
            }
        }
    }

    const bool bNetworkingInsightsLoaded = FModuleManager::Get().IsModuleLoaded(TEXT("NetworkingInsights"));

    UE_LOG(
        LogObjectNetInsightsBridge,
        Log,
        TEXT("Active session detected. Duration=%.3fs BaseUnix=%.3f NetTraceVersion=%u GameInstances=%u Connections=%u Objects=%u MappedEvents=%u (Rpc=%u Property=%u Unknown=%u) NetworkingInsightsLoaded=%s"),
        DurationSec,
        BaseDateTimeUnixSec,
        NetTraceVersion,
        GameInstanceCount,
        TotalConnectionCount,
        TotalObjectCount,
        MappedContentEventCount,
        RpcMappedCount,
        PropertyMappedCount,
        UnknownMappedCount,
        bNetworkingInsightsLoaded ? TEXT("true") : TEXT("false"));

    return true;
}