#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "ObjectNetMetadataParser.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FObjectNetMetadataParserTest,
    "ObjectNetInspector.MetadataParser.ObjectNamePath",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FObjectNetMetadataParserTest::RunTest(const FString& Parameters)
{
    (void)Parameters;

    {
        FString ObjectName;
        FString ObjectPath;
        FObjectNetMetadataParser::ParseObjectNameAndPath(
            TEXT("/Game/Maps/Arena.Arena:PersistentLevel.BP_PlayerCharacter_C_0"),
            ObjectName,
            ObjectPath);

        TestEqual(TEXT("Path-like name should extract short object name"), ObjectName, FString(TEXT("BP_PlayerCharacter_C_0")));
        TestEqual(TEXT("Path-like name should preserve full object path"), ObjectPath, FString(TEXT("/Game/Maps/Arena.Arena:PersistentLevel.BP_PlayerCharacter_C_0")));
    }

    {
        FString ObjectName;
        FString ObjectPath;
        FObjectNetMetadataParser::ParseObjectNameAndPath(TEXT("BP_Rifle_C_3"), ObjectName, ObjectPath);

        TestEqual(TEXT("Simple name should remain unchanged"), ObjectName, FString(TEXT("BP_Rifle_C_3")));
        TestTrue(TEXT("Simple name should not produce object path"), ObjectPath.IsEmpty());
    }

    {
        FString ClassName;
        const bool bInferred = FObjectNetMetadataParser::TryInferClassName(
            TEXT("/Game/Maps/Arena.Arena:PersistentLevel.BP_PlayerCharacter_C_0"),
            ClassName);

        TestTrue(TEXT("Path-like object should infer class name"), bInferred);
        TestEqual(TEXT("Path-like class should strip instance suffix"), ClassName, FString(TEXT("BP_PlayerCharacter_C")));
    }

    {
        FString ClassName;
        const bool bInferred = FObjectNetMetadataParser::TryInferClassName(TEXT("Default__BP_Weapon_C"), ClassName);

        TestTrue(TEXT("Default object should infer class name"), bInferred);
        TestEqual(TEXT("Default object should strip prefix"), ClassName, FString(TEXT("BP_Weapon_C")));
    }

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
