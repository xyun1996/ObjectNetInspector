#include "ObjectNetProvider.h"
#include "ObjectNetFormatting.h"

#include "Widgets/SCompoundWidget.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"

namespace ObjectNetEventTable
{
struct FRowItem
{
    FObjectNetEvent Event;
};

enum class ESortColumn : uint8
{
    Time,
    Connection,
    Name,
    Bits
};

static FString DirectionToString(const EObjectNetDirection Direction)
{
    switch (Direction)
    {
    case EObjectNetDirection::Incoming:
        return TEXT("Incoming");
    case EObjectNetDirection::Outgoing:
        return TEXT("Outgoing");
    default:
        return TEXT("Unknown");
    }
}

static FString KindToString(const EObjectNetEventKind Kind)
{
    switch (Kind)
    {
    case EObjectNetEventKind::Rpc:
        return TEXT("RPC");
    case EObjectNetEventKind::Property:
        return TEXT("Property");
    case EObjectNetEventKind::PacketRef:
        return TEXT("PacketRef");
    default:
        return TEXT("Unknown");
    }
}
} // namespace ObjectNetEventTable

class SObjectNetEventTable : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SObjectNetEventTable)
    {
    }
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, const TSharedRef<FObjectNetProvider>& InProvider)
    {
        Provider = InProvider;
        SortColumn = ObjectNetEventTable::ESortColumn::Time;
        SortMode = EColumnSortMode::Ascending;

        RebuildRows();
        CachedFingerprint = BuildFingerprint();

        ChildSlot
        [
            SAssignNew(ListView, SListView<TSharedPtr<ObjectNetEventTable::FRowItem>>)
            .ListItemsSource(&Rows)
            .SelectionMode(ESelectionMode::None)
            .OnGenerateRow(this, &SObjectNetEventTable::OnGenerateRow)
            .HeaderRow
            (
                SNew(SHeaderRow)

                + SHeaderRow::Column(TEXT("Time"))
                .FixedWidth(90.0f)
                .DefaultLabel(FText::FromString(TEXT("Time")))
                .SortMode(this, &SObjectNetEventTable::GetSortModeForTime)
                .OnSort(this, &SObjectNetEventTable::OnSortRequested)

                + SHeaderRow::Column(TEXT("Direction"))
                .FixedWidth(85.0f)
                .DefaultLabel(FText::FromString(TEXT("Direction")))

                + SHeaderRow::Column(TEXT("Conn"))
                .FixedWidth(70.0f)
                .DefaultLabel(FText::FromString(TEXT("Connection")))

                + SHeaderRow::Column(TEXT("Type"))
                .FixedWidth(85.0f)
                .DefaultLabel(FText::FromString(TEXT("Type")))

                + SHeaderRow::Column(TEXT("Name"))
                .FillWidth(0.46f)
                .DefaultLabel(FText::FromString(TEXT("Name")))

                + SHeaderRow::Column(TEXT("Packet"))
                .FixedWidth(65.0f)
                .DefaultLabel(FText::FromString(TEXT("Packet")))

                + SHeaderRow::Column(TEXT("BitsBytes"))
                .FixedWidth(150.0f)
                .DefaultLabel(FText::FromString(TEXT("Bits/Bytes")))
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
        const TOptional<uint64> SelectedObjectId = Provider->GetSelectedObjectId();
        uint32 Fingerprint = GetTypeHash(SelectedObjectId.IsSet() ? SelectedObjectId.GetValue() : 0ull);

        const TArray<FObjectNetEvent> Events = Provider->GetSelectedObjectEvents();
        for (const FObjectNetEvent& Event : Events)
        {
            Fingerprint = HashCombine(Fingerprint, GetTypeHash(Event.TimeSec));
            Fingerprint = HashCombine(Fingerprint, GetTypeHash(Event.ConnectionId));
            Fingerprint = HashCombine(Fingerprint, GetTypeHash(Event.PacketId));
            Fingerprint = HashCombine(Fingerprint, GetTypeHash(Event.EventName));
            Fingerprint = HashCombine(Fingerprint, GetTypeHash(Event.BitCount.IsSet() ? Event.BitCount.GetValue() : 0ull));
        }

        return Fingerprint;
    }

    void RebuildRows()
    {
        Rows.Reset();

        const TArray<FObjectNetEvent> Events = Provider->GetSelectedObjectEvents();
        Rows.Reserve(Events.Num());

        for (const FObjectNetEvent& Event : Events)
        {
            TSharedPtr<ObjectNetEventTable::FRowItem> Item = MakeShared<ObjectNetEventTable::FRowItem>();
            Item->Event = Event;
            Rows.Add(Item);
        }

        SortRows();
    }

    void SortRows()
    {
        Rows.Sort([this](const TSharedPtr<ObjectNetEventTable::FRowItem>& A, const TSharedPtr<ObjectNetEventTable::FRowItem>& B)
        {
            const FObjectNetEvent& L = A->Event;
            const FObjectNetEvent& R = B->Event;

            auto CompareU64 = [this](const uint64 LV, const uint64 RV)
            {
                return (SortMode == EColumnSortMode::Ascending) ? (LV < RV) : (LV > RV);
            };

            switch (SortColumn)
            {
            case ObjectNetEventTable::ESortColumn::Connection:
                return CompareU64(L.ConnectionId, R.ConnectionId);
            case ObjectNetEventTable::ESortColumn::Name:
                return (SortMode == EColumnSortMode::Ascending) ? (L.EventName < R.EventName) : (L.EventName > R.EventName);
            case ObjectNetEventTable::ESortColumn::Bits:
            {
                const uint64 LBits = L.BitCount.IsSet() ? L.BitCount.GetValue() : 0ull;
                const uint64 RBits = R.BitCount.IsSet() ? R.BitCount.GetValue() : 0ull;
                return CompareU64(LBits, RBits);
            }
            case ObjectNetEventTable::ESortColumn::Time:
            default:
                return (SortMode == EColumnSortMode::Ascending) ? (L.TimeSec < R.TimeSec) : (L.TimeSec > R.TimeSec);
            }
        });
    }

    TSharedRef<ITableRow> OnGenerateRow(TSharedPtr<ObjectNetEventTable::FRowItem> Item, const TSharedRef<STableViewBase>& OwnerTable) const
    {
        return SNew(STableRow<TSharedPtr<ObjectNetEventTable::FRowItem>>, OwnerTable)
            [
                SNew(SHorizontalBox)

                + SHorizontalBox::Slot().FillWidth(0.12f)
                [
                    SNew(STextBlock).Text(FText::FromString(ObjectNetFormatting::FormatTimeSeconds(Item->Event.TimeSec)))
                ]

                + SHorizontalBox::Slot().FillWidth(0.14f)
                [
                    SNew(STextBlock).Text(FText::FromString(ObjectNetEventTable::DirectionToString(Item->Event.Direction)))
                ]

                + SHorizontalBox::Slot().FillWidth(0.12f)
                [
                    SNew(STextBlock).Text(FText::AsNumber(Item->Event.ConnectionId))
                ]

                + SHorizontalBox::Slot().FillWidth(0.14f)
                [
                    SNew(STextBlock).Text(FText::FromString(ObjectNetEventTable::KindToString(Item->Event.Kind)))
                ]

                + SHorizontalBox::Slot().FillWidth(0.26f)
                [
                    SNew(STextBlock).Text(FText::FromString(Item->Event.EventName))
                ]

                + SHorizontalBox::Slot().FillWidth(0.10f)
                [
                    SNew(STextBlock).Text(FText::AsNumber(Item->Event.PacketId))
                ]

                + SHorizontalBox::Slot().FillWidth(0.12f)
                [
                    SNew(STextBlock).Text(FText::FromString(ObjectNetFormatting::FormatBitsAndBytes(Item->Event.BitCount)))
                ]
            ];
    }

    void OnSortRequested(const EColumnSortPriority::Type, const FName ColumnId, const EColumnSortMode::Type NewSortMode)
    {
        SortMode = NewSortMode;

        if (ColumnId == TEXT("Conn"))
        {
            SortColumn = ObjectNetEventTable::ESortColumn::Connection;
        }
        else if (ColumnId == TEXT("Name"))
        {
            SortColumn = ObjectNetEventTable::ESortColumn::Name;
        }
        else if (ColumnId == TEXT("BitsBytes"))
        {
            SortColumn = ObjectNetEventTable::ESortColumn::Bits;
        }
        else
        {
            SortColumn = ObjectNetEventTable::ESortColumn::Time;
        }

        SortRows();
        ListView->RequestListRefresh();
    }

    EColumnSortMode::Type GetSortModeForTime() const
    {
        return (SortColumn == ObjectNetEventTable::ESortColumn::Time) ? SortMode : EColumnSortMode::None;
    }

private:
    TSharedPtr<FObjectNetProvider> Provider;
    TArray<TSharedPtr<ObjectNetEventTable::FRowItem>> Rows;
    TSharedPtr<SListView<TSharedPtr<ObjectNetEventTable::FRowItem>>> ListView;
    ObjectNetEventTable::ESortColumn SortColumn;
    EColumnSortMode::Type SortMode;
    uint32 CachedFingerprint = 0;
};

TSharedRef<SWidget> MakeObjectNetEventTableWidget(const TSharedRef<FObjectNetProvider>& Provider)
{
    return SNew(SObjectNetEventTable, Provider);
}
