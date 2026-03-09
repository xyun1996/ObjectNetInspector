#include "ObjectNetAnalyzer.h"

namespace ObjectNetTraceReader
{
static FObjectNetEvent MakeEvent(
    const double TimeSec,
    const uint32 ConnectionId,
    const uint64 ObjectId,
    const TCHAR* ObjectName,
    const TCHAR* ObjectPath,
    const TCHAR* ClassName,
    const EObjectNetEventKind Kind,
    const EObjectNetDirection Direction,
    const TCHAR* EventName,
    const uint32 PacketId,
    const TOptional<uint64> BitCount)
{
    FObjectNetEvent Event;
    Event.TimeSec = TimeSec;
    Event.ConnectionId = ConnectionId;
    Event.ObjectId = ObjectId;
    Event.ObjectName = ObjectName;
    Event.ObjectPath = ObjectPath;
    Event.ClassName = ClassName;
    Event.Kind = Kind;
    Event.Direction = Direction;
    Event.EventName = EventName;
    Event.PacketId = PacketId;
    Event.BitCount = BitCount;
    return Event;
}
} // namespace ObjectNetTraceReader

bool FObjectNetTraceReader::InitializeFromActiveSession()
{
    Reset();

    // TODO: Replace this stub with real extraction from the active Unreal Insights session.
    // Keep this returning false until verified Trace/Networking Insights APIs are integrated.
    return false;
}

void FObjectNetTraceReader::Reset()
{
    Events.Reset();
}

void FObjectNetTraceReader::LoadMockDataForTesting()
{
    Reset();
    Events.Reserve(18);

    // Object A: character with mixed RPC/property updates on multiple connections.
    Events.Add(ObjectNetTraceReader::MakeEvent(1.000, 1, 1001, TEXT("BP_PlayerCharacter_C_0"), TEXT("/Game/Maps/Arena.Arena:PersistentLevel.BP_PlayerCharacter_C_0"), TEXT("BP_PlayerCharacter_C"), EObjectNetEventKind::Rpc, EObjectNetDirection::Outgoing, TEXT("ServerMove"), 3001, 128ull));
    Events.Add(ObjectNetTraceReader::MakeEvent(1.040, 1, 1001, TEXT("BP_PlayerCharacter_C_0"), TEXT("/Game/Maps/Arena.Arena:PersistentLevel.BP_PlayerCharacter_C_0"), TEXT("BP_PlayerCharacter_C"), EObjectNetEventKind::Property, EObjectNetDirection::Outgoing, TEXT("ReplicatedMovement"), 3001, 256ull));
    Events.Add(ObjectNetTraceReader::MakeEvent(1.060, 2, 1001, TEXT("BP_PlayerCharacter_C_0"), TEXT("/Game/Maps/Arena.Arena:PersistentLevel.BP_PlayerCharacter_C_0"), TEXT("BP_PlayerCharacter_C"), EObjectNetEventKind::Property, EObjectNetDirection::Outgoing, TEXT("Health"), 3002, TOptional<uint64>()));
    Events.Add(ObjectNetTraceReader::MakeEvent(1.200, 2, 1001, TEXT("BP_PlayerCharacter_C_0"), TEXT("/Game/Maps/Arena.Arena:PersistentLevel.BP_PlayerCharacter_C_0"), TEXT("BP_PlayerCharacter_C"), EObjectNetEventKind::Rpc, EObjectNetDirection::Incoming, TEXT("ClientPlayHitReact"), 3005, 96ull));
    Events.Add(ObjectNetTraceReader::MakeEvent(1.260, 1, 1001, TEXT("BP_PlayerCharacter_C_0"), TEXT("/Game/Maps/Arena.Arena:PersistentLevel.BP_PlayerCharacter_C_0"), TEXT("BP_PlayerCharacter_C"), EObjectNetEventKind::Property, EObjectNetDirection::Outgoing, TEXT("Ammo"), 3006, 40ull));

    // Object B: weapon actor with sparse bit availability.
    Events.Add(ObjectNetTraceReader::MakeEvent(1.010, 1, 1002, TEXT("BP_Rifle_C_3"), TEXT("/Game/Maps/Arena.Arena:PersistentLevel.BP_Rifle_C_3"), TEXT("BP_Rifle_C"), EObjectNetEventKind::Rpc, EObjectNetDirection::Outgoing, TEXT("ServerFire"), 3001, 72ull));
    Events.Add(ObjectNetTraceReader::MakeEvent(1.050, 1, 1002, TEXT("BP_Rifle_C_3"), TEXT("/Game/Maps/Arena.Arena:PersistentLevel.BP_Rifle_C_3"), TEXT("BP_Rifle_C"), EObjectNetEventKind::Property, EObjectNetDirection::Outgoing, TEXT("CurrentSpread"), 3001, TOptional<uint64>()));
    Events.Add(ObjectNetTraceReader::MakeEvent(1.120, 2, 1002, TEXT("BP_Rifle_C_3"), TEXT("/Game/Maps/Arena.Arena:PersistentLevel.BP_Rifle_C_3"), TEXT("BP_Rifle_C"), EObjectNetEventKind::Property, EObjectNetDirection::Outgoing, TEXT("BurstCounter"), 3004, 24ull));
    Events.Add(ObjectNetTraceReader::MakeEvent(1.240, 2, 1002, TEXT("BP_Rifle_C_3"), TEXT("/Game/Maps/Arena.Arena:PersistentLevel.BP_Rifle_C_3"), TEXT("BP_Rifle_C"), EObjectNetEventKind::Rpc, EObjectNetDirection::Incoming, TEXT("ClientConfirmHit"), 3005, TOptional<uint64>()));
    Events.Add(ObjectNetTraceReader::MakeEvent(1.310, 1, 1002, TEXT("BP_Rifle_C_3"), TEXT("/Game/Maps/Arena.Arena:PersistentLevel.BP_Rifle_C_3"), TEXT("BP_Rifle_C"), EObjectNetEventKind::Property, EObjectNetDirection::Outgoing, TEXT("Durability"), 3007, 16ull));

    // Object C: game state object, frequent properties and one packet reference.
    Events.Add(ObjectNetTraceReader::MakeEvent(0.980, 1, 1003, TEXT("BP_GameState_C_0"), TEXT("/Game/Maps/Arena.Arena:PersistentLevel.BP_GameState_C_0"), TEXT("BP_GameState_C"), EObjectNetEventKind::Property, EObjectNetDirection::Outgoing, TEXT("MatchTimeRemaining"), 3000, 32ull));
    Events.Add(ObjectNetTraceReader::MakeEvent(1.030, 1, 1003, TEXT("BP_GameState_C_0"), TEXT("/Game/Maps/Arena.Arena:PersistentLevel.BP_GameState_C_0"), TEXT("BP_GameState_C"), EObjectNetEventKind::Property, EObjectNetDirection::Outgoing, TEXT("TeamScores"), 3001, 80ull));
    Events.Add(ObjectNetTraceReader::MakeEvent(1.140, 2, 1003, TEXT("BP_GameState_C_0"), TEXT("/Game/Maps/Arena.Arena:PersistentLevel.BP_GameState_C_0"), TEXT("BP_GameState_C"), EObjectNetEventKind::Property, EObjectNetDirection::Outgoing, TEXT("TeamScores"), 3004, 80ull));
    Events.Add(ObjectNetTraceReader::MakeEvent(1.180, 2, 1003, TEXT("BP_GameState_C_0"), TEXT("/Game/Maps/Arena.Arena:PersistentLevel.BP_GameState_C_0"), TEXT("BP_GameState_C"), EObjectNetEventKind::Rpc, EObjectNetDirection::Outgoing, TEXT("MulticastRoundState"), 3004, 88ull));
    Events.Add(ObjectNetTraceReader::MakeEvent(1.260, 1, 1003, TEXT("BP_GameState_C_0"), TEXT("/Game/Maps/Arena.Arena:PersistentLevel.BP_GameState_C_0"), TEXT("BP_GameState_C"), EObjectNetEventKind::PacketRef, EObjectNetDirection::Outgoing, TEXT("PacketRefOnly"), 3006, TOptional<uint64>()));
}

const TArray<FObjectNetEvent>& FObjectNetTraceReader::GetEvents() const
{
    return Events;
}
