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
        FString ObjectName;
        FString ObjectPath;
        FObjectNetMetadataParser::ParseObjectNameAndPath(
            TEXT("BP_Weapon.BP_Weapon_C_1"),
            ObjectName,
            ObjectPath);

        TestEqual(TEXT("Dot-only path should extract short object name"), ObjectName, FString(TEXT("BP_Weapon_C_1")));
        TestEqual(TEXT("Dot-only path should preserve full object path"), ObjectPath, FString(TEXT("BP_Weapon.BP_Weapon_C_1")));
    }

    {
        FString ObjectName;
        FString ObjectPath;
        FObjectNetMetadataParser::ParseObjectNameAndPath(
            TEXT("  '/Game/Maps/Arena.Arena:PersistentLevel.BP_Pawn_C_2'  "),
            ObjectName,
            ObjectPath);

        TestEqual(TEXT("Quoted path should extract short object name"), ObjectName, FString(TEXT("BP_Pawn_C_2")));
        TestEqual(TEXT("Quoted path should be sanitized and preserved"), ObjectPath, FString(TEXT("/Game/Maps/Arena.Arena:PersistentLevel.BP_Pawn_C_2")));
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

    {
        const FString ClassName = FObjectNetMetadataParser::NormalizeClassName(TEXT("Class'/Script/LyraGame.BP_PlayerCharacter_C'"));
        TestEqual(TEXT("Quoted type path should normalize to short class"), ClassName, FString(TEXT("BP_PlayerCharacter_C")));
    }

    {
        const FString ClassName = FObjectNetMetadataParser::NormalizeClassName(TEXT("/Script/Engine.Character"));
        TestEqual(TEXT("Script type path should normalize to leaf class"), ClassName, FString(TEXT("Character")));
    }

    {
        const FString ClassName = FObjectNetMetadataParser::NormalizeClassName(TEXT("Class'/Script/LyraGame.REINST_BP_Rifle_C_123'"));
        TestEqual(TEXT("Reinst prefix should be stripped from class name"), ClassName, FString(TEXT("BP_Rifle_C_123")));
    }

    {
        const FString ClassName = FObjectNetMetadataParser::NormalizeClassName(TEXT("SKEL_BP_PlayerCharacter_C"));
        TestEqual(TEXT("Skel prefix should be stripped from class name"), ClassName, FString(TEXT("BP_PlayerCharacter_C")));
    }

    {
        const FString ClassName = FObjectNetMetadataParser::NormalizeClassName(TEXT("TRASHCLASS_BP_DebugPawn_C"));
        TestEqual(TEXT("TrashClass prefix should be stripped from class name"), ClassName, FString(TEXT("BP_DebugPawn_C")));
    }

    {
        FString ClassName;
        const bool bInferred = FObjectNetMetadataParser::TryInferClassNameFromEventName(
            TEXT("/Script/GameplayAbilities.AbilitySystemComponent::ServerTryActivateAbility"),
            ClassName);
        TestTrue(TEXT("Scoped event name should infer class name"), bInferred);
        TestEqual(TEXT("Scoped event name should infer normalized class"), ClassName, FString(TEXT("AbilitySystemComponent")));
    }

    {
        FString ClassName;
        const bool bInferred = FObjectNetMetadataParser::TryInferClassNameFromEventName(
            TEXT("BP_Weapon_C::ClientOnFireConfirmed"),
            ClassName);
        TestTrue(TEXT("Blueprint scoped event should infer class name"), bInferred);
        TestEqual(TEXT("Blueprint scoped event should keep BP class token"), ClassName, FString(TEXT("BP_Weapon_C")));
    }

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
