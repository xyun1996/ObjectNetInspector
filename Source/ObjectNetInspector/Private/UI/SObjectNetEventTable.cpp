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
    Direction,
    Connection,
    Type,
    Name,
    Packet,
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
        CachedRevision = Provider->GetViewRevision();
        CachedSelectionRevision = Provider->GetSelectionRevision();

        RebuildRows();

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
                .SortMode(this, &SObjectNetEventTable::GetSortModeForDirection)
                .OnSort(this, &SObjectNetEventTable::OnSortRequested)

                + SHeaderRow::Column(TEXT("Conn"))
                .FixedWidth(70.0f)
                .DefaultLabel(FText::FromString(TEXT("Connection")))
                .SortMode(this, &SObjectNetEventTable::GetSortModeForConnection)
                .OnSort(this, &SObjectNetEventTable::OnSortRequested)

                + SHeaderRow::Column(TEXT("Type"))
                .FixedWidth(85.0f)
                .DefaultLabel(FText::FromString(TEXT("Type")))
                .SortMode(this, &SObjectNetEventTable::GetSortModeForType)
                .OnSort(this, &SObjectNetEventTable::OnSortRequested)

                + SHeaderRow::Column(TEXT("Name"))
                .FillWidth(0.46f)
                .DefaultLabel(FText::FromString(TEXT("Name")))
                .SortMode(this, &SObjectNetEventTable::GetSortModeForName)
                .OnSort(this, &SObjectNetEventTable::OnSortRequested)

                + SHeaderRow::Column(TEXT("Packet"))
                .FixedWidth(65.0f)
                .DefaultLabel(FText::FromString(TEXT("Packet")))
                .SortMode(this, &SObjectNetEventTable::GetSortModeForPacket)
                .OnSort(this, &SObjectNetEventTable::OnSortRequested)

                + SHeaderRow::Column(TEXT("BitsBytes"))
                .FixedWidth(150.0f)
                .DefaultLabel(FText::FromString(TEXT("Bits/Bytes")))
                .SortMode(this, &SObjectNetEventTable::GetSortModeForBits)
                .OnSort(this, &SObjectNetEventTable::OnSortRequested)
            )
        ];
    }

    virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override
    {
        SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

        const uint64 NewRevision = Provider->GetViewRevision();
        const uint64 NewSelectionRevision = Provider->GetSelectionRevision();
        if (NewRevision != CachedRevision || NewSelectionRevision != CachedSelectionRevision)
        {
            CachedRevision = NewRevision;
            CachedSelectionRevision = NewSelectionRevision;
            RebuildRows();
            ListView->RequestListRefresh();
        }
    }

private:
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
            case ObjectNetEventTable::ESortColumn::Direction:
                return CompareU64(static_cast<uint8>(L.Direction), static_cast<uint8>(R.Direction));
            case ObjectNetEventTable::ESortColumn::Connection:
                return CompareU64(L.ConnectionId, R.ConnectionId);
            case ObjectNetEventTable::ESortColumn::Type:
                return CompareU64(static_cast<uint8>(L.Kind), static_cast<uint8>(R.Kind));
            case ObjectNetEventTable::ESortColumn::Name:
                return (SortMode == EColumnSortMode::Ascending) ? (L.EventName < R.EventName) : (L.EventName > R.EventName);
            case ObjectNetEventTable::ESortColumn::Packet:
                return CompareU64(L.PacketId, R.PacketId);
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

    void OnSortRequested(const EColumnSortPriority::Type, const FName& ColumnId, const EColumnSortMode::Type NewSortMode)
    {
        SortMode = NewSortMode;

        if (ColumnId == TEXT("Conn"))
        {
            SortColumn = ObjectNetEventTable::ESortColumn::Connection;
        }
        else if (ColumnId == TEXT("Direction"))
        {
            SortColumn = ObjectNetEventTable::ESortColumn::Direction;
        }
        else if (ColumnId == TEXT("Type"))
        {
            SortColumn = ObjectNetEventTable::ESortColumn::Type;
        }
        else if (ColumnId == TEXT("Name"))
        {
            SortColumn = ObjectNetEventTable::ESortColumn::Name;
        }
        else if (ColumnId == TEXT("Packet"))
        {
            SortColumn = ObjectNetEventTable::ESortColumn::Packet;
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

    EColumnSortMode::Type GetSortModeForDirection() const
    {
        return (SortColumn == ObjectNetEventTable::ESortColumn::Direction) ? SortMode : EColumnSortMode::None;
    }

    EColumnSortMode::Type GetSortModeForConnection() const
    {
        return (SortColumn == ObjectNetEventTable::ESortColumn::Connection) ? SortMode : EColumnSortMode::None;
    }

    EColumnSortMode::Type GetSortModeForType() const
    {
        return (SortColumn == ObjectNetEventTable::ESortColumn::Type) ? SortMode : EColumnSortMode::None;
    }

    EColumnSortMode::Type GetSortModeForName() const
    {
        return (SortColumn == ObjectNetEventTable::ESortColumn::Name) ? SortMode : EColumnSortMode::None;
    }

    EColumnSortMode::Type GetSortModeForPacket() const
    {
        return (SortColumn == ObjectNetEventTable::ESortColumn::Packet) ? SortMode : EColumnSortMode::None;
    }

    EColumnSortMode::Type GetSortModeForBits() const
    {
        return (SortColumn == ObjectNetEventTable::ESortColumn::Bits) ? SortMode : EColumnSortMode::None;
    }

private:
    TSharedPtr<FObjectNetProvider> Provider;
    TArray<TSharedPtr<ObjectNetEventTable::FRowItem>> Rows;
    TSharedPtr<SListView<TSharedPtr<ObjectNetEventTable::FRowItem>>> ListView;
    ObjectNetEventTable::ESortColumn SortColumn;
    EColumnSortMode::Type SortMode;
    uint64 CachedRevision = 0;
    uint64 CachedSelectionRevision = 0;
};

TSharedRef<SWidget> MakeObjectNetEventTableWidget(const TSharedRef<FObjectNetProvider>& Provider)
{
    return SNew(SObjectNetEventTable, Provider);
}
