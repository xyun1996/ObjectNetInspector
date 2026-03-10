#include "ObjectNetProvider.h"

#include "Widgets/SCompoundWidget.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"

namespace ObjectNetObjectList
{
struct FRowItem
{
    FObjectNetAggregate Aggregate;
};

enum class ESortColumn : uint8
{
    Name,
    Class,
    Events,
    Rpcs,
    Properties,
    KnownBytes
};
}

class SObjectNetObjectList : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SObjectNetObjectList)
    {
    }
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, const TSharedRef<FObjectNetProvider>& InProvider)
    {
        Provider = InProvider;
        SortColumn = ObjectNetObjectList::ESortColumn::KnownBytes;
        SortMode = EColumnSortMode::Descending;

        RebuildRows();
        CachedFingerprint = BuildFingerprint();

        ChildSlot
        [
            SAssignNew(ListView, SListView<TSharedPtr<ObjectNetObjectList::FRowItem>>)
            .ListItemsSource(&Rows)
            .SelectionMode(ESelectionMode::Single)
            .OnGenerateRow(this, &SObjectNetObjectList::OnGenerateRow)
            .OnSelectionChanged(this, &SObjectNetObjectList::OnSelectionChanged)
            .HeaderRow
            (
                SNew(SHeaderRow)

                + SHeaderRow::Column(TEXT("Name"))
                .FillWidth(0.32f)
                .DefaultLabel(FText::FromString(TEXT("Name")))
                .SortMode(this, &SObjectNetObjectList::GetSortModeForName)
                .OnSort(this, &SObjectNetObjectList::OnSortRequested)

                + SHeaderRow::Column(TEXT("Class"))
                .FillWidth(0.24f)
                .DefaultLabel(FText::FromString(TEXT("Class")))

                + SHeaderRow::Column(TEXT("Events"))
                .FixedWidth(65.0f)
                .HAlignCell(HAlign_Right)
                .DefaultLabel(FText::FromString(TEXT("Events")))

                + SHeaderRow::Column(TEXT("RPCs"))
                .FixedWidth(65.0f)
                .HAlignCell(HAlign_Right)
                .DefaultLabel(FText::FromString(TEXT("RPCs")))

                + SHeaderRow::Column(TEXT("Props"))
                .FixedWidth(85.0f)
                .HAlignCell(HAlign_Right)
                .DefaultLabel(FText::FromString(TEXT("Properties")))

                + SHeaderRow::Column(TEXT("KnownBytes"))
                .FixedWidth(95.0f)
                .HAlignCell(HAlign_Right)
                .DefaultLabel(FText::FromString(TEXT("Known Bytes")))
            )
        ];
    }

    virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override
    {
        SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

        const uint32 NewFingerprint = BuildFingerprint();
        if (NewFingerprint != CachedFingerprint)
        {
            CachedFingerprint = NewFingerprint;
            RebuildRows();
            ListView->RequestListRefresh();
        }
    }

private:
    uint32 BuildFingerprint() const
    {
        uint32 Fingerprint = 0;
        for (const FObjectNetAggregate& Aggregate : Provider->GetCurrentAggregates())
        {
            Fingerprint = HashCombine(Fingerprint, GetTypeHash(Aggregate.ObjectId));
            Fingerprint = HashCombine(Fingerprint, GetTypeHash(Aggregate.TotalEventCount));
            Fingerprint = HashCombine(Fingerprint, GetTypeHash(Aggregate.TotalKnownBits));
        }
        return Fingerprint;
    }

    void RebuildRows()
    {
        Rows.Reset();

        for (const FObjectNetAggregate& Aggregate : Provider->GetCurrentAggregates())
        {
            TSharedPtr<ObjectNetObjectList::FRowItem> Item = MakeShared<ObjectNetObjectList::FRowItem>();
            Item->Aggregate = Aggregate;
            Rows.Add(Item);
        }

        SortRows();
    }

    void SortRows()
    {
        Rows.Sort([this](const TSharedPtr<ObjectNetObjectList::FRowItem>& A, const TSharedPtr<ObjectNetObjectList::FRowItem>& B)
        {
            const FObjectNetAggregate& L = A->Aggregate;
            const FObjectNetAggregate& R = B->Aggregate;

            auto CompareNumeric = [this](const uint64 LV, const uint64 RV)
            {
                return (SortMode == EColumnSortMode::Ascending) ? (LV < RV) : (LV > RV);
            };

            switch (SortColumn)
            {
            case ObjectNetObjectList::ESortColumn::Name:
                return (SortMode == EColumnSortMode::Ascending) ? (L.ObjectName < R.ObjectName) : (L.ObjectName > R.ObjectName);
            case ObjectNetObjectList::ESortColumn::Class:
                return (SortMode == EColumnSortMode::Ascending) ? (L.ClassName < R.ClassName) : (L.ClassName > R.ClassName);
            case ObjectNetObjectList::ESortColumn::Events:
                return CompareNumeric(L.TotalEventCount, R.TotalEventCount);
            case ObjectNetObjectList::ESortColumn::Rpcs:
                return CompareNumeric(L.RpcCount, R.RpcCount);
            case ObjectNetObjectList::ESortColumn::Properties:
                return CompareNumeric(L.PropertyCount, R.PropertyCount);
            case ObjectNetObjectList::ESortColumn::KnownBytes:
            default:
                return CompareNumeric((L.TotalKnownBits + 7ull) / 8ull, (R.TotalKnownBits + 7ull) / 8ull);
            }
        });
    }

    TSharedRef<ITableRow> OnGenerateRow(TSharedPtr<ObjectNetObjectList::FRowItem> Item, const TSharedRef<STableViewBase>& OwnerTable) const
    {
        const uint64 KnownBytes = (Item->Aggregate.TotalKnownBits + 7ull) / 8ull;

        return SNew(STableRow<TSharedPtr<ObjectNetObjectList::FRowItem>>, OwnerTable)
            [
                SNew(SHorizontalBox)

                + SHorizontalBox::Slot().FillWidth(0.32f)
                [
                    SNew(STextBlock).Text(FText::FromString(Item->Aggregate.ObjectName))
                ]

                + SHorizontalBox::Slot().FillWidth(0.24f)
                [
                    SNew(STextBlock).Text(FText::FromString(Item->Aggregate.ClassName))
                ]

                + SHorizontalBox::Slot().FillWidth(0.11f)
                [
                    SNew(STextBlock).Text(FText::AsNumber(Item->Aggregate.TotalEventCount))
                ]

                + SHorizontalBox::Slot().FillWidth(0.11f)
                [
                    SNew(STextBlock).Text(FText::AsNumber(Item->Aggregate.RpcCount))
                ]

                + SHorizontalBox::Slot().FillWidth(0.12f)
                [
                    SNew(STextBlock).Text(FText::AsNumber(Item->Aggregate.PropertyCount))
                ]

                + SHorizontalBox::Slot().FillWidth(0.12f)
                [
                    SNew(STextBlock).Text(FText::AsNumber(KnownBytes))
                ]
            ];
    }

    void OnSelectionChanged(TSharedPtr<ObjectNetObjectList::FRowItem> Item, ESelectInfo::Type)
    {
        if (Item.IsValid())
        {
            Provider->SetSelectedObjectId(Item->Aggregate.ObjectId);
        }
        else
        {
            Provider->SetSelectedObjectId(TOptional<uint64>());
        }
    }

    void OnSortRequested(const EColumnSortPriority::Type, const FName& ColumnId, const EColumnSortMode::Type NewSortMode)
    {
        SortMode = NewSortMode;

        if (ColumnId == TEXT("Name"))
        {
            SortColumn = ObjectNetObjectList::ESortColumn::Name;
        }
        else if (ColumnId == TEXT("Class"))
        {
            SortColumn = ObjectNetObjectList::ESortColumn::Class;
        }
        else if (ColumnId == TEXT("Events"))
        {
            SortColumn = ObjectNetObjectList::ESortColumn::Events;
        }
        else if (ColumnId == TEXT("RPCs"))
        {
            SortColumn = ObjectNetObjectList::ESortColumn::Rpcs;
        }
        else if (ColumnId == TEXT("Props"))
        {
            SortColumn = ObjectNetObjectList::ESortColumn::Properties;
        }
        else
        {
            SortColumn = ObjectNetObjectList::ESortColumn::KnownBytes;
        }

        SortRows();
        ListView->RequestListRefresh();
    }

    EColumnSortMode::Type GetSortModeForName() const
    {
        return (SortColumn == ObjectNetObjectList::ESortColumn::Name) ? SortMode : EColumnSortMode::None;
    }

private:
    TSharedPtr<FObjectNetProvider> Provider;
    TArray<TSharedPtr<ObjectNetObjectList::FRowItem>> Rows;
    TSharedPtr<SListView<TSharedPtr<ObjectNetObjectList::FRowItem>>> ListView;
    ObjectNetObjectList::ESortColumn SortColumn;
    EColumnSortMode::Type SortMode;
    uint32 CachedFingerprint = 0;
};

TSharedRef<SWidget> MakeObjectNetObjectListWidget(const TSharedRef<FObjectNetProvider>& Provider)
{
    return SNew(SObjectNetObjectList, Provider);
}
