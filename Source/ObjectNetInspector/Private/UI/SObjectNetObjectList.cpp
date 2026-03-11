#include "ObjectNetProvider.h"

#include "Widgets/SCompoundWidget.h"
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
} // namespace ObjectNetObjectList

class SObjectNetObjectListRow : public SMultiColumnTableRow<TSharedPtr<ObjectNetObjectList::FRowItem>>
{
public:
    SLATE_BEGIN_ARGS(SObjectNetObjectListRow)
    {
    }
        SLATE_ARGUMENT(TSharedPtr<ObjectNetObjectList::FRowItem>, Item)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& OwnerTable)
    {
        Item = InArgs._Item;
        SMultiColumnTableRow<TSharedPtr<ObjectNetObjectList::FRowItem>>::Construct(
            SMultiColumnTableRow<TSharedPtr<ObjectNetObjectList::FRowItem>>::FArguments(),
            OwnerTable);
    }

    virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override
    {
        if (!Item.IsValid())
        {
            return SNew(STextBlock).Text(FText::GetEmpty());
        }

        const FObjectNetAggregate& Aggregate = Item->Aggregate;
        const uint64 KnownBytes = (Aggregate.TotalKnownBits + 7ull) / 8ull;

        if (ColumnName == TEXT("Name"))
        {
            return SNew(STextBlock).Text(FText::FromString(Aggregate.ObjectName));
        }
        if (ColumnName == TEXT("Class"))
        {
            return SNew(STextBlock).Text(FText::FromString(Aggregate.ClassName));
        }
        if (ColumnName == TEXT("Events"))
        {
            return SNew(STextBlock).Justification(ETextJustify::Right).Text(FText::AsNumber(Aggregate.TotalEventCount));
        }
        if (ColumnName == TEXT("RPCs"))
        {
            return SNew(STextBlock).Justification(ETextJustify::Right).Text(FText::AsNumber(Aggregate.RpcCount));
        }
        if (ColumnName == TEXT("Props"))
        {
            return SNew(STextBlock).Justification(ETextJustify::Right).Text(FText::AsNumber(Aggregate.PropertyCount));
        }
        if (ColumnName == TEXT("KnownBytes"))
        {
            return SNew(STextBlock).Justification(ETextJustify::Right).Text(FText::AsNumber(KnownBytes));
        }

        return SNew(STextBlock).Text(FText::GetEmpty());
    }

private:
    TSharedPtr<ObjectNetObjectList::FRowItem> Item;
};

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
        CachedRevision = Provider->GetViewRevision();

        RebuildRows();

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
                .FillWidth(0.36f)
                .DefaultLabel(FText::FromString(TEXT("Name")))
                .SortMode(this, &SObjectNetObjectList::GetSortModeForName)
                .OnSort(this, &SObjectNetObjectList::OnSortRequested)

                + SHeaderRow::Column(TEXT("Class"))
                .FillWidth(0.26f)
                .DefaultLabel(FText::FromString(TEXT("Class")))
                .SortMode(this, &SObjectNetObjectList::GetSortModeForClass)
                .OnSort(this, &SObjectNetObjectList::OnSortRequested)

                + SHeaderRow::Column(TEXT("Events"))
                .FixedWidth(95.0f)
                .HAlignCell(HAlign_Right)
                .DefaultLabel(FText::FromString(TEXT("Events")))
                .SortMode(this, &SObjectNetObjectList::GetSortModeForEvents)
                .OnSort(this, &SObjectNetObjectList::OnSortRequested)

                + SHeaderRow::Column(TEXT("RPCs"))
                .FixedWidth(75.0f)
                .HAlignCell(HAlign_Right)
                .DefaultLabel(FText::FromString(TEXT("RPCs")))
                .SortMode(this, &SObjectNetObjectList::GetSortModeForRpcs)
                .OnSort(this, &SObjectNetObjectList::OnSortRequested)

                + SHeaderRow::Column(TEXT("Props"))
                .FixedWidth(90.0f)
                .HAlignCell(HAlign_Right)
                .DefaultLabel(FText::FromString(TEXT("Properties")))
                .SortMode(this, &SObjectNetObjectList::GetSortModeForProperties)
                .OnSort(this, &SObjectNetObjectList::OnSortRequested)

                + SHeaderRow::Column(TEXT("KnownBytes"))
                .FixedWidth(110.0f)
                .HAlignCell(HAlign_Right)
                .DefaultLabel(FText::FromString(TEXT("Known Bytes")))
                .SortMode(this, &SObjectNetObjectList::GetSortModeForKnownBytes)
                .OnSort(this, &SObjectNetObjectList::OnSortRequested)
            )
        ];
    }

    virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override
    {
        SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

        const uint64 NewRevision = Provider->GetViewRevision();
        if (NewRevision != CachedRevision)
        {
            CachedRevision = NewRevision;
            RebuildRows();
            ListView->RequestListRefresh();
        }
    }

private:
    void RebuildRows()
    {
        Rows.Reset();
        Rows.Reserve(Provider->GetCurrentAggregates().Num());

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
        return SNew(SObjectNetObjectListRow, OwnerTable).Item(Item);
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

    EColumnSortMode::Type GetSortModeForClass() const
    {
        return (SortColumn == ObjectNetObjectList::ESortColumn::Class) ? SortMode : EColumnSortMode::None;
    }

    EColumnSortMode::Type GetSortModeForEvents() const
    {
        return (SortColumn == ObjectNetObjectList::ESortColumn::Events) ? SortMode : EColumnSortMode::None;
    }

    EColumnSortMode::Type GetSortModeForRpcs() const
    {
        return (SortColumn == ObjectNetObjectList::ESortColumn::Rpcs) ? SortMode : EColumnSortMode::None;
    }

    EColumnSortMode::Type GetSortModeForProperties() const
    {
        return (SortColumn == ObjectNetObjectList::ESortColumn::Properties) ? SortMode : EColumnSortMode::None;
    }

    EColumnSortMode::Type GetSortModeForKnownBytes() const
    {
        return (SortColumn == ObjectNetObjectList::ESortColumn::KnownBytes) ? SortMode : EColumnSortMode::None;
    }

private:
    TSharedPtr<FObjectNetProvider> Provider;
    TArray<TSharedPtr<ObjectNetObjectList::FRowItem>> Rows;
    TSharedPtr<SListView<TSharedPtr<ObjectNetObjectList::FRowItem>>> ListView;
    ObjectNetObjectList::ESortColumn SortColumn;
    EColumnSortMode::Type SortMode;
    uint64 CachedRevision = 0;
};

TSharedRef<SWidget> MakeObjectNetObjectListWidget(const TSharedRef<FObjectNetProvider>& Provider)
{
    return SNew(SObjectNetObjectList, Provider);
}
