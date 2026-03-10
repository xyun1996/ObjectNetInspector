#include "ObjectNetEventClassifier.h"

namespace
{
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
} // namespace

EObjectNetEventKind FObjectNetEventClassifier::InferKind(const FString& EventName, const uint16 EventTypeLevel, const uint8 ContentLevel)
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