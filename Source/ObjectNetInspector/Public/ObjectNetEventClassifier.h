#pragma once

#include "CoreMinimal.h"
#include "ObjectNetTypes.h"

class FObjectNetEventClassifier
{
public:
    static EObjectNetEventKind InferKind(const FString& EventName, uint16 EventTypeLevel, uint8 ContentLevel);
};