#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "ObjectNetEventClassifier.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FObjectNetEventClassifierTest,
    "ObjectNetInspector.Classifier.KindInference",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FObjectNetEventClassifierTest::RunTest(const FString& Parameters)
{
    (void)Parameters;

    TestEqual(
        TEXT("RPC keyword should map to Rpc"),
        static_cast<uint8>(FObjectNetEventClassifier::InferKind(TEXT("ServerFireRpc"), 0, 0)),
        static_cast<uint8>(EObjectNetEventKind::Rpc));

    TestEqual(
        TEXT("Multicast should map to Rpc"),
        static_cast<uint8>(FObjectNetEventClassifier::InferKind(TEXT("NetMulticast_Explosion"), 0, 0)),
        static_cast<uint8>(EObjectNetEventKind::Rpc));

    TestEqual(
        TEXT("Property keyword should map to Property"),
        static_cast<uint8>(FObjectNetEventClassifier::InferKind(TEXT("ReplicatedMovementProperty"), 0, 0)),
        static_cast<uint8>(EObjectNetEventKind::Property));

    TestEqual(
        TEXT("Fast array payload should map to Property"),
        static_cast<uint8>(FObjectNetEventClassifier::InferKind(TEXT("FastArraySerializerDelta"), 0, 0)),
        static_cast<uint8>(EObjectNetEventKind::Property));

    TestEqual(
        TEXT("RepLayout signal should map to Property"),
        static_cast<uint8>(FObjectNetEventClassifier::InferKind(TEXT("RepLayoutPushModelState"), 0, 0)),
        static_cast<uint8>(EObjectNetEventKind::Property));

    TestEqual(
        TEXT("NetSerialize signal should map to Property"),
        static_cast<uint8>(FObjectNetEventClassifier::InferKind(TEXT("InventoryNetSerializeDelta"), 0, 0)),
        static_cast<uint8>(EObjectNetEventKind::Property));

    TestEqual(
        TEXT("Iris fragment signal should map to Property"),
        static_cast<uint8>(FObjectNetEventClassifier::InferKind(TEXT("IrisReplicationFragmentDescriptor"), 0, 0)),
        static_cast<uint8>(EObjectNetEventKind::Property));

    TestEqual(
        TEXT("Remote function signal should map to Rpc"),
        static_cast<uint8>(FObjectNetEventClassifier::InferKind(TEXT("CallRemoteFunction"), 0, 0)),
        static_cast<uint8>(EObjectNetEventKind::Rpc));

    TestEqual(
        TEXT("ProcessRemoteFunction should map to Rpc"),
        static_cast<uint8>(FObjectNetEventClassifier::InferKind(TEXT("ProcessRemoteFunctionForChannel"), 0, 0)),
        static_cast<uint8>(EObjectNetEventKind::Rpc));

    TestEqual(
        TEXT("Weak remote hint alone should stay Unknown"),
        static_cast<uint8>(FObjectNetEventClassifier::InferKind(TEXT("RemoteHandle"), 0, 0)),
        static_cast<uint8>(EObjectNetEventKind::Unknown));

    TestEqual(
        TEXT("No keyword but high level should map to Property"),
        static_cast<uint8>(FObjectNetEventClassifier::InferKind(TEXT("CustomPayload"), 2, 0)),
        static_cast<uint8>(EObjectNetEventKind::Property));

    TestEqual(
        TEXT("Server/client words alone should not force Rpc"),
        static_cast<uint8>(FObjectNetEventClassifier::InferKind(TEXT("ServerPredictionData"), 0, 0)),
        static_cast<uint8>(EObjectNetEventKind::Unknown));

    TestEqual(
        TEXT("PrepareData should not be misclassified by rep substring"),
        static_cast<uint8>(FObjectNetEventClassifier::InferKind(TEXT("PrepareData"), 0, 0)),
        static_cast<uint8>(EObjectNetEventKind::Unknown));

    TestEqual(
        TEXT("FunctionGraph editor context should stay Unknown"),
        static_cast<uint8>(FObjectNetEventClassifier::InferKind(TEXT("FunctionGraphCompilePass"), 0, 0)),
        static_cast<uint8>(EObjectNetEventKind::Unknown));

    TestEqual(
        TEXT("FunctionTable editor context should stay Unknown"),
        static_cast<uint8>(FObjectNetEventClassifier::InferKind(TEXT("BuildFunctionTable"), 0, 0)),
        static_cast<uint8>(EObjectNetEventKind::Unknown));

    TestEqual(
        TEXT("NetField lifecycle should map to Property"),
        static_cast<uint8>(FObjectNetEventClassifier::InferKind(TEXT("NetFieldRetirementState"), 0, 0)),
        static_cast<uint8>(EObjectNetEventKind::Property));

    TestEqual(
        TEXT("Packet-level bunch should map to PacketRef"),
        static_cast<uint8>(FObjectNetEventClassifier::InferKind(TEXT("ChannelBunchPacket"), 0, 0)),
        static_cast<uint8>(EObjectNetEventKind::PacketRef));

    TestEqual(
        TEXT("Ack packet update should map to PacketRef"),
        static_cast<uint8>(FObjectNetEventClassifier::InferKind(TEXT("NetPacketAck"), 0, 0)),
        static_cast<uint8>(EObjectNetEventKind::PacketRef));

    TestEqual(
        TEXT("Delivery status should map to PacketRef"),
        static_cast<uint8>(FObjectNetEventClassifier::InferKind(TEXT("PacketDeliveryStatus"), 0, 0)),
        static_cast<uint8>(EObjectNetEventKind::PacketRef));

    TestEqual(
        TEXT("StateBuffer should map to Property"),
        static_cast<uint8>(FObjectNetEventClassifier::InferKind(TEXT("IrisStateBufferChangeMask"), 0, 0)),
        static_cast<uint8>(EObjectNetEventKind::Property));

    TestEqual(
        TEXT("WriteObject should map to Property"),
        static_cast<uint8>(FObjectNetEventClassifier::InferKind(TEXT("WriteObjectNetField"), 0, 0)),
        static_cast<uint8>(EObjectNetEventKind::Property));

    TestEqual(
        TEXT("Property signal should win over packet hint"),
        static_cast<uint8>(FObjectNetEventClassifier::InferKind(TEXT("ReplicatedPropertyPacket"), 0, 0)),
        static_cast<uint8>(EObjectNetEventKind::Property));

    TestEqual(
        TEXT("Content block header should map to Property"),
        static_cast<uint8>(FObjectNetEventClassifier::InferKind(TEXT("ContentBlockHeader"), 0, 0)),
        static_cast<uint8>(EObjectNetEventKind::Property));

    TestEqual(
        TEXT("Property handle should map to Property"),
        static_cast<uint8>(FObjectNetEventClassifier::InferKind(TEXT("PropertyHandle"), 0, 0)),
        static_cast<uint8>(EObjectNetEventKind::Property));

    TestEqual(
        TEXT("Packet header metadata should map to PacketRef"),
        static_cast<uint8>(FObjectNetEventClassifier::InferKind(TEXT("PacketHeaderAndInfo"), 0, 0)),
        static_cast<uint8>(EObjectNetEventKind::PacketRef));

    TestEqual(
        TEXT("Ambiguous rpc+rep mix should stay Unknown"),
        static_cast<uint8>(FObjectNetEventClassifier::InferKind(TEXT("ServerRepFunction"), 0, 0)),
        static_cast<uint8>(EObjectNetEventKind::Unknown));

    TestEqual(
        TEXT("No signal should stay Unknown"),
        static_cast<uint8>(FObjectNetEventClassifier::InferKind(TEXT("CustomPayload"), 0, 0)),
        static_cast<uint8>(EObjectNetEventKind::Unknown));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FObjectNetEventClassifierQualityGuardTest,
    "ObjectNetInspector.Classifier.QualityGuard",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FObjectNetEventClassifierQualityGuardTest::RunTest(const FString& Parameters)
{
    (void)Parameters;

    struct FCase
    {
        const TCHAR* Name;
        EObjectNetEventKind Expected;
    };

    static const FCase Corpus[] =
    {
        { TEXT("ServerFireRpc"), EObjectNetEventKind::Rpc },
        { TEXT("ClientRpc_OnDamage"), EObjectNetEventKind::Rpc },
        { TEXT("ProcessRemoteFunctionForChannel"), EObjectNetEventKind::Rpc },
        { TEXT("ReplicatedMovementProperty"), EObjectNetEventKind::Property },
        { TEXT("FastArraySerializerDelta"), EObjectNetEventKind::Property },
        { TEXT("IrisStateBufferChangeMask"), EObjectNetEventKind::Property },
        { TEXT("WriteObjectNetField"), EObjectNetEventKind::Property },
        { TEXT("ContentBlockHeader"), EObjectNetEventKind::Property },
        { TEXT("PropertyHandle"), EObjectNetEventKind::Property },
        { TEXT("ChannelBunchPacket"), EObjectNetEventKind::PacketRef },
        { TEXT("PacketHeaderAndInfo"), EObjectNetEventKind::PacketRef },
        { TEXT("PacketDeliveryStatus"), EObjectNetEventKind::PacketRef },
        { TEXT("BuildFunctionTable"), EObjectNetEventKind::Unknown },
        { TEXT("RemoteHandle"), EObjectNetEventKind::Unknown },
        { TEXT("CustomPayload"), EObjectNetEventKind::Unknown }
    };

    int32 UnknownCount = 0;
    for (const FCase& Entry : Corpus)
    {
        const EObjectNetEventKind Actual = FObjectNetEventClassifier::InferKind(Entry.Name, 0, 0);
        if (Actual == EObjectNetEventKind::Unknown)
        {
            ++UnknownCount;
        }

        TestEqual(
            FString::Printf(TEXT("Classifier corpus expectation: %s"), Entry.Name),
            static_cast<uint8>(Actual),
            static_cast<uint8>(Entry.Expected));
    }

    const double UnknownRatio = static_cast<double>(UnknownCount) / static_cast<double>(UE_ARRAY_COUNT(Corpus));
    TestTrue(TEXT("Classifier corpus unknown ratio should stay <= 0.30"), UnknownRatio <= 0.30);

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
