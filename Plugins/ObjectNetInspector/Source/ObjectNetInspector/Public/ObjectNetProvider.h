#pragma once

#include "CoreMinimal.h"
#include "Misc/Optional.h"
#include "ObjectNetAggregator.h"
#include "ObjectNetAnalyzer.h"
#include "ObjectNetQuery.h"

/** Thin UI-facing coordinator around reader, analyzer, and aggregator. */
class FObjectNetProvider
{
public:
    FObjectNetProvider();

    void Refresh();

    void SetQuery(const FObjectNetQuery& InQuery);
    const FObjectNetQuery& GetQuery() const;

    void SetSelectedObjectId(TOptional<uint64> InObjectId);
    TOptional<uint64> GetSelectedObjectId() const;

    const TArray<FObjectNetAggregate>& GetCurrentAggregates() const;
    TOptional<FObjectNetAggregate> GetSelectedAggregate() const;
    TArray<FObjectNetEvent> GetSelectedObjectEvents() const;

    FObjectNetTraceReader& GetReader();
    const FObjectNetTraceReader& GetReader() const;

private:
    FObjectNetTraceReader TraceReader;
    FObjectNetAnalyzer Analyzer;
    FObjectNetAggregator Aggregator;

    FObjectNetQuery CurrentQuery;
    TOptional<uint64> SelectedObjectId;
    TArray<FObjectNetAggregate> CurrentAggregates;
};
