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
        TEXT("No keyword but high level should map to Property"),
        static_cast<uint8>(FObjectNetEventClassifier::InferKind(TEXT("CustomPayload"), 2, 0)),
        static_cast<uint8>(EObjectNetEventKind::Property));

    TestEqual(
        TEXT("Server/client words alone should not force Rpc"),
        static_cast<uint8>(FObjectNetEventClassifier::InferKind(TEXT("ServerPredictionData"), 0, 0)),
        static_cast<uint8>(EObjectNetEventKind::Unknown));

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

#endif // WITH_DEV_AUTOMATION_TESTS