#include "ObjectNetProvider.h"
#include "ObjectNetFormatting.h"

#include "Algo/Sort.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Text/STextBlock.h"

namespace ObjectNetDetailView
{
static FString BuildTopSummary(const TMap<FString, uint32>& Counts)
{
    TArray<TPair<FString, uint32>> Pairs;
    for (const TPair<FString, uint32>& It : Counts)
    {
        Pairs.Add(It);
    }

    Pairs.Sort([](const TPair<FString, uint32>& A, const TPair<FString, uint32>& B)
    {
        if (A.Value != B.Value)
        {
            return A.Value > B.Value;
        }

        return A.Key < B.Key;
    });

    FString Out;
    const int32 Limit = FMath::Min(3, Pairs.Num());
    for (int32 Index = 0; Index < Limit; ++Index)
    {
        Out += FString::Printf(TEXT("%s (%u)"), *Pairs[Index].Key, Pairs[Index].Value);
        if (Index < Limit - 1)
        {
            Out += TEXT("\n");
        }
    }

    return Out.IsEmpty() ? TEXT("N/A") : Out;
}
} // namespace ObjectNetDetailView

class SObjectNetDetailView : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SObjectNetDetailView)
    {
    }
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, const TSharedRef<FObjectNetProvider>& InProvider)
    {
        Provider = InProvider;

        ChildSlot
        [
            SNew(SScrollBox)

            + SScrollBox::Slot()
            [
                BuildSummaryWidget()
            ]
        ];
    }

private:
    TSharedRef<SWidget> BuildSummaryWidget()
    {
        return SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 6.0f)
            [
                SNew(STextBlock)
                .Text(this, &SObjectNetDetailView::GetTitleText)
            ]
            + SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(this, &SObjectNetDetailView::GetPathText)]
            + SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(this, &SObjectNetDetailView::GetClassText)]
            + SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(this, &SObjectNetDetailView::GetCountText)]
            + SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(this, &SObjectNetDetailView::GetBitsText)]
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 8.0f, 0.0f, 0.0f)[SNew(STextBlock).Text(this, &SObjectNetDetailView::GetTopRpcText)]
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f, 0.0f, 0.0f)[SNew(STextBlock).Text(this, &SObjectNetDetailView::GetTopPropertyText)];
    }

    TOptional<FObjectNetAggregate> GetAggregate() const
    {
        return Provider->GetSelectedAggregate();
    }

    FText GetTitleText() const
    {
        const TOptional<FObjectNetAggregate> Agg = GetAggregate();
        if (!Agg.IsSet())
        {
            return FText::FromString(TEXT("No object selected"));
        }

        return FText::FromString(FString::Printf(TEXT("Object: %s"), *Agg->ObjectName));
    }

    FText GetPathText() const
    {
        const TOptional<FObjectNetAggregate> Agg = GetAggregate();
        return Agg.IsSet() ? FText::FromString(FString::Printf(TEXT("Path: %s"), *Agg->ObjectPath)) : FText::GetEmpty();
    }

    FText GetClassText() const
    {
        const TOptional<FObjectNetAggregate> Agg = GetAggregate();
        return Agg.IsSet() ? FText::FromString(FString::Printf(TEXT("Class: %s"), *Agg->ClassName)) : FText::GetEmpty();
    }

    FText GetCountText() const
    {
        const TOptional<FObjectNetAggregate> Agg = GetAggregate();
        if (!Agg.IsSet())
        {
            return FText::GetEmpty();
        }

        return FText::FromString(FString::Printf(
            TEXT("Events: %u  RPCs: %u  Properties: %u  PacketHits: %u"),
            Agg->TotalEventCount,
            Agg->RpcCount,
            Agg->PropertyCount,
            Agg->PacketHitCount));
    }

    FText GetBitsText() const
    {
        const TOptional<FObjectNetAggregate> Agg = GetAggregate();
        if (!Agg.IsSet())
        {
            return FText::GetEmpty();
        }

        return FText::FromString(FString::Printf(TEXT("Known: %s"), *ObjectNetFormatting::FormatKnownBitsToBytes(Agg->TotalKnownBits)));
    }

    FText GetTopRpcText() const
    {
        const TOptional<FObjectNetAggregate> Agg = GetAggregate();
        if (!Agg.IsSet())
        {
            return FText::GetEmpty();
        }

        return FText::FromString(FString::Printf(TEXT("Top RPCs:\n%s"), *ObjectNetDetailView::BuildTopSummary(Agg->RpcCounts)));
    }

    FText GetTopPropertyText() const
    {
        const TOptional<FObjectNetAggregate> Agg = GetAggregate();
        if (!Agg.IsSet())
        {
            return FText::GetEmpty();
        }

        return FText::FromString(FString::Printf(TEXT("Top Properties:\n%s"), *ObjectNetDetailView::BuildTopSummary(Agg->PropertyCounts)));
    }

private:
    TSharedPtr<FObjectNetProvider> Provider;
};

TSharedRef<SWidget> MakeObjectNetDetailViewWidget(const TSharedRef<FObjectNetProvider>& Provider)
{
    return SNew(SObjectNetDetailView, Provider);
}
