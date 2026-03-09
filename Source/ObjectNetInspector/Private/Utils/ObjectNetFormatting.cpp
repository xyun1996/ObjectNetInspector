#include "ObjectNetFormatting.h"

namespace ObjectNetFormatting
{
FString FormatTimeSeconds(const double TimeSec)
{
    return FString::Printf(TEXT("%.3f s"), TimeSec);
}

FString FormatBitsAndBytes(const TOptional<uint64>& BitCount)
{
    if (!BitCount.IsSet())
    {
        return TEXT("N/A");
    }

    const uint64 Bits = BitCount.GetValue();
    const uint64 Bytes = (Bits + 7ull) / 8ull;
    return FString::Printf(TEXT("%llu bits / %llu bytes"), Bits, Bytes);
}

FString FormatKnownBitsToBytes(const uint64 KnownBits)
{
    const uint64 Bytes = (KnownBits + 7ull) / 8ull;
    return FString::Printf(TEXT("%llu bits / %llu bytes"), KnownBits, Bytes);
}
} // namespace ObjectNetFormatting
