#pragma once

#include "CoreMinimal.h"

class FObjectNetMetadataParser
{
public:
    static void ParseObjectNameAndPath(const FString& RawObjectName, FString& OutObjectName, FString& OutObjectPath);
    static bool TryInferClassName(const FString& RawObjectName, FString& OutClassName);
    static bool TryInferClassNameFromEventName(const FString& RawEventName, FString& OutClassName);
    static FString NormalizeClassName(const FString& RawClassName);
};
