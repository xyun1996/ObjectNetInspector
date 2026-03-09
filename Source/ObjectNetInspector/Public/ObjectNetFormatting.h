#pragma once

#include "CoreMinimal.h"
#include "Misc/Optional.h"

namespace ObjectNetFormatting
{
FString FormatTimeSeconds(double TimeSec);
FString FormatBitsAndBytes(const TOptional<uint64>& BitCount);
FString FormatKnownBitsToBytes(uint64 KnownBits);
} // namespace ObjectNetFormatting
