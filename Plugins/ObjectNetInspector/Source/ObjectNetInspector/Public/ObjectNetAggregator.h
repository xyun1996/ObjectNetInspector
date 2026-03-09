#pragma once

#include "CoreMinimal.h"
#include "Misc/Optional.h"
#include "ObjectNetQuery.h"
#include "ObjectNetTypes.h"

class FObjectNetAnalyzer;

/** Builds aggregate rows from indexed events under a given query filter. */
class FObjectNetAggregator
{
public:
    TArray<FObjectNetAggregate> BuildAggregates(const FObjectNetAnalyzer& Analyzer, const FObjectNetQuery& Query) const;

    TOptional<FObjectNetAggregate> BuildAggregateForObject(
        uint64 ObjectId,
        const FObjectNetAnalyzer& Analyzer,
        const FObjectNetQuery& Query) const;

private:
    static void SortAggregates(TArray<FObjectNetAggregate>& Aggregates);
};
