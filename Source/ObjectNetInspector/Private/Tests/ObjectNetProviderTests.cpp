#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "ObjectNetProvider.h"

namespace
{
static FObjectNetEvent MakeEvent(
    const double TimeSec,
    const uint32 ConnectionId,
    const uint64 ObjectId,
    const TCHAR* ObjectName,
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
    Event.ObjectPath = TEXT("/Game/Test");
    Event.ClassName = TEXT("BP_Test_C");
    Event.Kind = Kind;
    Event.Direction = Direction;
    Event.EventName = EventName;
    Event.PacketId = PacketId;
    Event.BitCount = BitCount;
    return Event;
}
} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FObjectNetProviderFilteringAndAggregationTest,
    "ObjectNetInspector.Provider.FilteringAndAggregation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FObjectNetProviderFilteringAndAggregationTest::RunTest(const FString& Parameters)
{
    (void)Parameters;

    FObjectNetProvider Provider;

    Provider.GetReader().SetSessionReader(
        [](TArray<FObjectNetEvent>& OutEvents)
        {
            OutEvents.Reset();
            OutEvents.Add(MakeEvent(1.0, 1, 1001, TEXT("ObjA"), EObjectNetEventKind::Rpc, EObjectNetDirection::Outgoing, TEXT("ServerDoThingRpc"), 101, 128ull));
            OutEvents.Add(MakeEvent(1.1, 1, 1001, TEXT("ObjA"), EObjectNetEventKind::Property, EObjectNetDirection::Outgoing, TEXT("HealthProperty"), 101, TOptional<uint64>()));
            OutEvents.Add(MakeEvent(1.2, 2, 2002, TEXT("ObjB"), EObjectNetEventKind::Property, EObjectNetDirection::Outgoing, TEXT("AmmoProperty"), 202, 64ull));
            OutEvents.Add(MakeEvent(1.3, 2, 2002, TEXT("ObjB"), EObjectNetEventKind::Unknown, EObjectNetDirection::Outgoing, TEXT("CustomPayload"), 203, TOptional<uint64>()));
            OutEvents.Add(MakeEvent(1.4, 2, 2002, TEXT("ObjB"), EObjectNetEventKind::PacketRef, EObjectNetDirection::Outgoing, TEXT("ChannelBunchPacket"), 204, TOptional<uint64>()));
            return true;
        });

    Provider.Refresh();

    TestEqual(TEXT("Session reader data source should be Session"), static_cast<uint8>(Provider.GetLastDataSourceKind()), static_cast<uint8>(EObjectNetDataSourceKind::Session));
    TestEqual(TEXT("Session reader event count"), Provider.GetLastEventCount(), 5);
    TestEqual(TEXT("Session reader unknown event count"), Provider.GetLastUnknownEventCount(), 1);
    TestTrue(TEXT("Session reader unknown ratio should be 0.2"), FMath::IsNearlyEqual(Provider.GetLastUnknownRatio(), 0.2, KINDA_SMALL_NUMBER));
    TestEqual(TEXT("Session reader packet-ref event count"), Provider.GetLastPacketRefEventCount(), 1);
    TestTrue(TEXT("Session reader packet-ref ratio should be 0.2"), FMath::IsNearlyEqual(Provider.GetLastPacketRefRatio(), 0.2, KINDA_SMALL_NUMBER));

    FObjectNetQuery Query;
    Query.ConnectionFilter = 1u;
    Provider.SetQuery(Query);

    const TArray<FObjectNetAggregate>& Aggregates = Provider.GetCurrentAggregates();
    TestEqual(TEXT("Connection=1 aggregate rows"), Aggregates.Num(), 1);

    if (Aggregates.Num() == 1)
    {
        const FObjectNetAggregate& Aggregate = Aggregates[0];
        TestEqual(TEXT("ObjA event count"), Aggregate.TotalEventCount, 2u);
        TestEqual(TEXT("ObjA rpc count"), Aggregate.RpcCount, 1u);
        TestEqual(TEXT("ObjA property count"), Aggregate.PropertyCount, 1u);
        TestEqual(TEXT("ObjA known bits should ignore N/A event"), Aggregate.TotalKnownBits, 128ull);
    }

    Provider.SetSelectedObjectId(1001ull);
    const TArray<FObjectNetEvent> SelectedEvents = Provider.GetSelectedObjectEvents();
    TestEqual(TEXT("Selected ObjA events"), SelectedEvents.Num(), 2);

    int32 UnknownBitsEventCount = 0;
    for (const FObjectNetEvent& Event : SelectedEvents)
    {
        if (!Event.HasKnownBits())
        {
            ++UnknownBitsEventCount;
        }
    }
    TestEqual(TEXT("Selected ObjA N/A bit events"), UnknownBitsEventCount, 1);

    FObjectNetQuery PropertyOnlyQuery;
    PropertyOnlyQuery.ConnectionFilter = 1u;
    PropertyOnlyQuery.bIncludeRpc = false;
    PropertyOnlyQuery.bIncludeProperties = true;
    Provider.SetQuery(PropertyOnlyQuery);

    const TArray<FObjectNetAggregate>& PropertyOnlyAggregates = Provider.GetCurrentAggregates();
    TestEqual(TEXT("Connection=1 property-only rows"), PropertyOnlyAggregates.Num(), 1);
    if (PropertyOnlyAggregates.Num() == 1)
    {
        const FObjectNetAggregate& Aggregate = PropertyOnlyAggregates[0];
        TestEqual(TEXT("Property-only event count"), Aggregate.TotalEventCount, 1u);
        TestEqual(TEXT("Property-only known bits should stay 0 for N/A"), Aggregate.TotalKnownBits, 0ull);
    }

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FObjectNetProviderSearchFieldsTest,
    "ObjectNetInspector.Provider.SearchFields",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FObjectNetProviderSearchFieldsTest::RunTest(const FString& Parameters)
{
    (void)Parameters;

    FObjectNetProvider Provider;

    Provider.GetReader().SetSessionReader(
        [](TArray<FObjectNetEvent>& OutEvents)
        {
            OutEvents.Reset();

            FObjectNetEvent PlayerEvent = MakeEvent(
                1.0,
                1,
                1001,
                TEXT("BP_PlayerCharacter_C_0"),
                EObjectNetEventKind::Property,
                EObjectNetDirection::Outgoing,
                TEXT("ReplicatedMovementProperty"),
                101,
                128ull);
            PlayerEvent.ObjectPath = TEXT("/Game/Maps/Arena.Arena:PersistentLevel.BP_PlayerCharacter_C_0");
            PlayerEvent.ClassName = TEXT("BP_PlayerCharacter_C");
            OutEvents.Add(MoveTemp(PlayerEvent));

            FObjectNetEvent WeaponEvent = MakeEvent(
                1.2,
                1,
                2002,
                TEXT("BP_Rifle_C_3"),
                EObjectNetEventKind::Rpc,
                EObjectNetDirection::Outgoing,
                TEXT("ServerFireRpc"),
                102,
                64ull);
            WeaponEvent.ObjectPath = TEXT("/Game/Weapons/BP_Rifle.BP_Rifle_C_3");
            WeaponEvent.ClassName = TEXT("BP_Rifle_C");
            OutEvents.Add(MoveTemp(WeaponEvent));

            return true;
        });

    Provider.Refresh();

    FObjectNetQuery PathQuery;
    PathQuery.SearchText = TEXT("PersistentLevel");
    Provider.SetQuery(PathQuery);
    TestEqual(TEXT("Path search should match one aggregate"), Provider.GetCurrentAggregates().Num(), 1);
    if (Provider.GetCurrentAggregates().Num() == 1)
    {
        TestEqual(TEXT("Path search should match player object"), Provider.GetCurrentAggregates()[0].ObjectId, 1001ull);
    }

    FObjectNetQuery ClassQuery;
    ClassQuery.SearchText = TEXT("BP_Rifle_C");
    Provider.SetQuery(ClassQuery);
    TestEqual(TEXT("Class search should match one aggregate"), Provider.GetCurrentAggregates().Num(), 1);
    if (Provider.GetCurrentAggregates().Num() == 1)
    {
        TestEqual(TEXT("Class search should match rifle object"), Provider.GetCurrentAggregates()[0].ObjectId, 2002ull);
    }

    FObjectNetQuery EventQuery;
    EventQuery.SearchText = TEXT("ServerFireRpc");
    Provider.SetQuery(EventQuery);
    TestEqual(TEXT("Event search should match one aggregate"), Provider.GetCurrentAggregates().Num(), 1);
    if (Provider.GetCurrentAggregates().Num() == 1)
    {
        TestEqual(TEXT("Event search should match rifle object"), Provider.GetCurrentAggregates()[0].ObjectId, 2002ull);
    }

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FObjectNetProviderFilterEdgeCasesTest,
    "ObjectNetInspector.Provider.FilterEdgeCases",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FObjectNetProviderFilterEdgeCasesTest::RunTest(const FString& Parameters)
{
    (void)Parameters;

    FObjectNetProvider Provider;

    Provider.GetReader().SetSessionReader(
        [](TArray<FObjectNetEvent>& OutEvents)
        {
            OutEvents.Reset();
            OutEvents.Add(MakeEvent(1.0, 1, 1001, TEXT("ObjA"), EObjectNetEventKind::Rpc, EObjectNetDirection::Outgoing, TEXT("ServerDoThingRpc"), 101, 128ull));
            OutEvents.Add(MakeEvent(2.0, 1, 1001, TEXT("ObjA"), EObjectNetEventKind::Property, EObjectNetDirection::Incoming, TEXT("HealthProperty"), 102, 64ull));
            OutEvents.Add(MakeEvent(3.0, 2, 2002, TEXT("ObjB"), EObjectNetEventKind::Property, EObjectNetDirection::Outgoing, TEXT("AmmoProperty"), 201, 32ull));
            return true;
        });

    Provider.Refresh();

    TestEqual(TEXT("Session reader event count should stay non-zero"), Provider.GetLastEventCount(), 3);
    TestEqual(TEXT("Default query should include two objects"), Provider.GetCurrentAggregates().Num(), 2);

    FObjectNetQuery OutgoingOnly;
    OutgoingOnly.DirectionFilter = EObjectNetDirection::Outgoing;
    Provider.SetQuery(OutgoingOnly);
    TestEqual(TEXT("Outgoing-only should keep two objects"), Provider.GetCurrentAggregates().Num(), 2);

    FObjectNetQuery TimeWindow;
    TimeWindow.TimeStartSec = 1.5;
    TimeWindow.TimeEndSec = 2.5;
    Provider.SetQuery(TimeWindow);
    TestEqual(TEXT("Time-window [1.5,2.5] should keep one object"), Provider.GetCurrentAggregates().Num(), 1);
    if (Provider.GetCurrentAggregates().Num() == 1)
    {
        const FObjectNetAggregate& Aggregate = Provider.GetCurrentAggregates()[0];
        TestEqual(TEXT("Time-window object id"), Aggregate.ObjectId, 1001ull);
        TestEqual(TEXT("Time-window event count"), Aggregate.TotalEventCount, 1u);
    }

    FObjectNetQuery DisabledKinds;
    DisabledKinds.bIncludeRpc = false;
    DisabledKinds.bIncludeProperties = false;
    Provider.SetQuery(DisabledKinds);
    TestEqual(TEXT("Both kind toggles off should return zero rows"), Provider.GetCurrentAggregates().Num(), 0);
    TestEqual(TEXT("Total source event count should remain unchanged"), Provider.GetLastEventCount(), 3);

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
