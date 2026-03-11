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

static FString ExtractLeafToken(const FString& RawValue)
{
    int32 LastDot = INDEX_NONE;
    int32 LastColon = INDEX_NONE;
    int32 LastSlash = INDEX_NONE;
    RawValue.FindLastChar(TEXT('.'), LastDot);
    RawValue.FindLastChar(TEXT(':'), LastColon);
    RawValue.FindLastChar(TEXT('/'), LastSlash);

    const int32 SplitIndex = FMath::Max(LastDot, FMath::Max(LastColon, LastSlash));
    if (SplitIndex != INDEX_NONE && SplitIndex + 1 < RawValue.Len())
    {
        return RawValue.Mid(SplitIndex + 1);
    }

    return RawValue;
}

static FString StripOuterQuotes(const FString& RawValue)
{
    FString Value = RawValue;
    Value.TrimStartAndEndInline();
    if (Value.Len() >= 2)
    {
        const TCHAR First = Value[0];
        const TCHAR Last = Value[Value.Len() - 1];
        const bool bSingleQuoted = (First == TEXT('\'')) && (Last == TEXT('\''));
        const bool bDoubleQuoted = (First == TEXT('"')) && (Last == TEXT('"'));
        if (bSingleQuoted || bDoubleQuoted)
        {
            Value = Value.Mid(1, Value.Len() - 2);
        }
    }

    return Value;
}

static void StripKnownTransientClassPrefixes(FString& InOutClassName)
{
    static const TCHAR* Prefixes[] =
    {
        TEXT("REINST_"),
        TEXT("SKEL_"),
        TEXT("TRASHCLASS_")
    };

    for (const TCHAR* Prefix : Prefixes)
    {
        if (InOutClassName.StartsWith(Prefix))
        {
            InOutClassName.RightChopInline(FCString::Strlen(Prefix), EAllowShrinking::No);
            break;
        }
    }
}

static bool LooksLikeObjectPath(const FString& RawValue)
{
    return RawValue.Contains(TEXT("/"), ESearchCase::CaseSensitive) ||
        RawValue.Contains(TEXT(":"), ESearchCase::CaseSensitive) ||
        (RawValue.Contains(TEXT("."), ESearchCase::CaseSensitive) &&
            (RawValue.Contains(TEXT("_C"), ESearchCase::CaseSensitive) ||
                RawValue.Contains(TEXT("PersistentLevel"), ESearchCase::CaseSensitive)));
}
} // namespace

void FObjectNetMetadataParser::ParseObjectNameAndPath(const FString& RawObjectName, FString& OutObjectName, FString& OutObjectPath)
{
    const FString SanitizedName = StripOuterQuotes(RawObjectName);

    OutObjectName = SanitizedName;
    OutObjectPath.Empty();

    if (SanitizedName.IsEmpty())
    {
        return;
    }

    const bool bLooksLikePath = LooksLikeObjectPath(SanitizedName);
    if (!bLooksLikePath)
    {
        return;
    }

    OutObjectPath = SanitizedName;
    OutObjectName = ExtractLeafToken(SanitizedName);
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

bool FObjectNetMetadataParser::TryInferClassNameFromEventName(const FString& RawEventName, FString& OutClassName)
{
    OutClassName.Empty();

    auto IsLikelyClassLikeToken = [](const FString& Token) -> bool
    {
        if (Token.IsEmpty())
        {
            return false;
        }

        const ESearchCase::Type SearchCase = ESearchCase::IgnoreCase;
        if (Token.Equals(TEXT("Event"), SearchCase) ||
            Token.Equals(TEXT("Payload"), SearchCase) ||
            Token.Equals(TEXT("Unknown"), SearchCase))
        {
            return false;
        }

        if (Token.EndsWith(TEXT("_C")) ||
            Token.Contains(TEXT("Component"), SearchCase) ||
            Token.Contains(TEXT("Actor"), SearchCase) ||
            Token.Contains(TEXT("Character"), SearchCase) ||
            Token.Contains(TEXT("Pawn"), SearchCase) ||
            Token.Contains(TEXT("Controller"), SearchCase))
        {
            return true;
        }

        // Scoped/path-based candidates are likely class-bearing tokens.
        return Token.Contains(TEXT("::"), ESearchCase::CaseSensitive) ||
            Token.Contains(TEXT("/"), ESearchCase::CaseSensitive) ||
            Token.Contains(TEXT("."), ESearchCase::CaseSensitive);
    };

    auto TryFromCandidate = [](const FString& RawCandidate, FString& OutValue)
    {
        FString Candidate = StripOuterQuotes(RawCandidate);
        Candidate.TrimStartAndEndInline();
        if (Candidate.IsEmpty())
        {
            return false;
        }
        if (Candidate.Contains(TEXT(" "), ESearchCase::CaseSensitive))
        {
            return false;
        }

        int32 ScopeIndex = INDEX_NONE;
        if (Candidate.FindLastChar(TEXT(':'), ScopeIndex) && ScopeIndex > 0)
        {
            // Handle "Class::Function" and similar event-name scopes.
            if (ScopeIndex > 0 && Candidate[ScopeIndex - 1] == TEXT(':'))
            {
                Candidate = Candidate.Left(ScopeIndex - 1);
            }
        }

        Candidate = NormalizeClassName(Candidate);
        if (Candidate.IsEmpty())
        {
            return false;
        }

        OutValue = Candidate;
        return true;
    };

    if (TryFromCandidate(RawEventName, OutClassName))
    {
        return true;
    }

    TArray<FString> Tokens;
    RawEventName.ParseIntoArrayWS(Tokens);
    for (const FString& Token : Tokens)
    {
        if (!IsLikelyClassLikeToken(Token))
        {
            continue;
        }

        if (TryFromCandidate(Token, OutClassName))
        {
            return true;
        }
    }

    for (const FString& Token : Tokens)
    {
        FString Normalized = NormalizeClassName(Token);
        if (!Normalized.IsEmpty() && (Normalized.EndsWith(TEXT("_C")) || Normalized.Contains(TEXT("Component"), ESearchCase::IgnoreCase)))
        {
            OutClassName = MoveTemp(Normalized);
            return true;
        }
    }

    if (RawEventName.Contains(TEXT("::"), ESearchCase::CaseSensitive))
    {
        // Fallback for uncommon strings where ParseIntoArrayWS doesn't isolate a valid scope token.
        FString Compact = RawEventName;
        Compact.ReplaceInline(TEXT(" "), TEXT(""));
        if (TryFromCandidate(Compact, OutClassName))
        {
            return true;
        }
    }

    return false;
}

FString FObjectNetMetadataParser::NormalizeClassName(const FString& RawClassName)
{
    FString Candidate = RawClassName;
    Candidate.TrimStartAndEndInline();
    if (Candidate.IsEmpty())
    {
        return FString();
    }

    int32 FirstQuoteIndex = INDEX_NONE;
    int32 LastQuoteIndex = INDEX_NONE;
    Candidate.FindChar(TEXT('\''), FirstQuoteIndex);
    Candidate.FindLastChar(TEXT('\''), LastQuoteIndex);
    if (FirstQuoteIndex != INDEX_NONE && LastQuoteIndex != INDEX_NONE && LastQuoteIndex > FirstQuoteIndex)
    {
        Candidate = Candidate.Mid(FirstQuoteIndex + 1, LastQuoteIndex - FirstQuoteIndex - 1);
    }

    Candidate = ExtractLeafToken(Candidate);

    if (Candidate.StartsWith(TEXT("Default__")))
    {
        Candidate.RightChopInline(9, EAllowShrinking::No);
    }
    StripKnownTransientClassPrefixes(Candidate);

    return Candidate;
}
