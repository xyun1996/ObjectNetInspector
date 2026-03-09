#pragma once

#include "CoreMinimal.h"

class SDockTab;
struct FSpawnTabArgs;

TSharedRef<SDockTab> SpawnObjectNetInspectorTab(const FSpawnTabArgs& SpawnTabArgs);
