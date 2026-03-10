#include "ObjectNetEventClassifier.h"

namespace
{
struct FWeightedTerm
{
    const TCHAR* Term;
    int32 Weight;
};

static int32 ComputeScore(const FString& Text, const FWeightedTerm* Terms, const int32 TermCount)
{
    const ESearchCase::Type SearchCase = ESearchCase::IgnoreCase;
    int32 Score = 0;

    for (int32 Index = 0; Index < TermCount; ++Index)
    {
        if (Text.Contains(Terms[Index].Term, SearchCase))
        {
            Score += Terms[Index].Weight;
        }
    }

    return Score;
}
} // namespace

EObjectNetEventKind FObjectNetEventClassifier::InferKind(const FString& EventName, const uint16 EventTypeLevel, const uint8 ContentLevel)
{
    static const FWeightedTerm RpcTerms[] =
    {
        { TEXT("rpc"), 4 },
        { TEXT("function"), 3 },
        { TEXT("remotefunction"), 3 },
        { TEXT("callremote"), 2 },
        { TEXT("sendrpc"), 3 },
        { TEXT("netmulticast"), 4 },
        { TEXT("multicast"), 3 },
        { TEXT("remote"), 1 }
    };

    static const FWeightedTerm PropertyTerms[] =
    {
        { TEXT("property"), 4 },
        { TEXT("replayout"), 3 },
        { TEXT("repstate"), 3 },
        { TEXT("replicated"), 3 },
        { TEXT("repnotify"), 3 },
        { TEXT("rep"), 2 },
        { TEXT("pushmodel"), 3 },
        { TEXT("changelist"), 2 },
        { TEXT("state"), 2 },
        { TEXT("delta"), 2 },
        { TEXT("array"), 2 },
        { TEXT("fastarray"), 3 },
        { TEXT("serializer"), 2 },
        { TEXT("quantized"), 1 }
    };

    const int32 RpcScore = ComputeScore(EventName, RpcTerms, UE_ARRAY_COUNT(RpcTerms));
    const int32 PropertyScore = ComputeScore(EventName, PropertyTerms, UE_ARRAY_COUNT(PropertyTerms));

    if (RpcScore > 0 && PropertyScore > 0)
    {
        const int32 Diff = FMath::Abs(RpcScore - PropertyScore);
        if (Diff <= 1)
        {
            return EObjectNetEventKind::Unknown;
        }

        if (RpcScore >= 3 || PropertyScore >= 3)
        {
            return RpcScore > PropertyScore ? EObjectNetEventKind::Rpc : EObjectNetEventKind::Property;
        }

        return EObjectNetEventKind::Unknown;
    }

    if (RpcScore >= 3)
    {
        return EObjectNetEventKind::Rpc;
    }

    if (PropertyScore >= 3)
    {
        return EObjectNetEventKind::Property;
    }

    if (RpcScore > 0)
    {
        return EObjectNetEventKind::Rpc;
    }

    if (PropertyScore > 0)
    {
        return EObjectNetEventKind::Property;
    }

    // Level-only fallback: bias to property payload events while preserving Unknown for weak/ambiguous names.
    if (EventTypeLevel >= 2 || ContentLevel >= 2)
    {
        return EObjectNetEventKind::Property;
    }

    return EObjectNetEventKind::Unknown;
}
