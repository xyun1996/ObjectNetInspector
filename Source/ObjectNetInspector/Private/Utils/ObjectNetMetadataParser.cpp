#include "ObjectNetMetadataParser.h"

namespace
{
static bool TryExtractTrailingNumericSuffix(const FString& Value, int32& OutUnderscoreIndex)
{
    OutUnderscoreIndex = INDEX_NONE;
    if (Value.IsEmpty())
    {
        return false;
    }

    int32 LastUnderscoreIndex = INDEX_NONE;
    Value.FindLastChar(TEXT('_'), LastUnderscoreIndex);
    if (LastUnderscoreIndex == INDEX_NONE || LastUnderscoreIndex + 1 >= Value.Len())
    {
        return false;
    }

    const FString NumericPart = Value.Mid(LastUnderscoreIndex + 1);
    for (int32 Index = 0; Index < NumericPart.Len(); ++Index)
    {
        if (!FChar::IsDigit(NumericPart[Index]))
        {
            return false;
        }
    }

    OutUnderscoreIndex = LastUnderscoreIndex;
    return true;
}
} // namespace

void FObjectNetMetadataParser::ParseObjectNameAndPath(const FString& RawObjectName, FString& OutObjectName, FString& OutObjectPath)
{
    OutObjectName = RawObjectName;
    OutObjectPath.Empty();

    if (RawObjectName.IsEmpty())
    {
        return;
    }

    const bool bLooksLikePath =
        RawObjectName.Contains(TEXT("/"), ESearchCase::CaseSensitive) ||
        RawObjectName.Contains(TEXT(":"), ESearchCase::CaseSensitive);
    if (!bLooksLikePath)
    {
        return;
    }

    OutObjectPath = RawObjectName;

    int32 LastDot = INDEX_NONE;
    int32 LastColon = INDEX_NONE;
    int32 LastSlash = INDEX_NONE;
    RawObjectName.FindLastChar(TEXT('.'), LastDot);
    RawObjectName.FindLastChar(TEXT(':'), LastColon);
    RawObjectName.FindLastChar(TEXT('/'), LastSlash);

    const int32 SplitIndex = FMath::Max(LastDot, FMath::Max(LastColon, LastSlash));
    if (SplitIndex != INDEX_NONE && SplitIndex + 1 < RawObjectName.Len())
    {
        OutObjectName = RawObjectName.Mid(SplitIndex + 1);
    }
}

bool FObjectNetMetadataParser::TryInferClassName(const FString& RawObjectName, FString& OutClassName)
{
    OutClassName.Empty();

    FString ParsedObjectName;
    FString ParsedObjectPath;
    ParseObjectNameAndPath(RawObjectName, ParsedObjectName, ParsedObjectPath);
    if (ParsedObjectName.IsEmpty())
    {
        return false;
    }

    FString Candidate = ParsedObjectName;
    if (Candidate.StartsWith(TEXT("Default__")))
    {
        Candidate.RightChopInline(9, EAllowShrinking::No);
    }

    int32 TrailingNumberUnderscoreIndex = INDEX_NONE;
    if (TryExtractTrailingNumericSuffix(Candidate, TrailingNumberUnderscoreIndex))
    {
        Candidate = Candidate.Left(TrailingNumberUnderscoreIndex);
    }

    if (Candidate.IsEmpty())
    {
        return false;
    }

    OutClassName = Candidate;
    return true;
}
