#include "ObjectNetProvider.h"

#include "Misc/DefaultValueHelper.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SHorizontalBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

class SObjectNetToolbar : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SObjectNetToolbar)
    {
    }
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, const TSharedRef<FObjectNetProvider>& InProvider)
    {
        Provider = InProvider;
        CachedQuery = Provider->GetQuery();

        ChildSlot
        [
            SNew(SVerticalBox)

            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0.0f, 0.0f, 0.0f, 6.0f)
            [
                SNew(SHorizontalBox)

                + SHorizontalBox::Slot()
                .FillWidth(1.0f)
                [
                    SNew(SSearchBox)
                    .HintText(FText::FromString(TEXT("Search object/class/path/event")))
                    .OnTextCommitted(this, &SObjectNetToolbar::OnSearchCommitted)
                ]

                + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(8.0f, 0.0f, 0.0f, 0.0f)
                [
                    SNew(SButton)
                    .Text(FText::FromString(TEXT("Refresh")))
                    .OnClicked(this, &SObjectNetToolbar::OnRefreshClicked)
                ]
            ]

            + SVerticalBox::Slot()
            .AutoHeight()
            [
                SNew(SHorizontalBox)

                + SHorizontalBox::Slot()
                .AutoWidth()
                .VAlign(VAlign_Center)
                [
                    SNew(STextBlock).Text(FText::FromString(TEXT("Conn")))
                ]

                + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(6.0f, 0.0f)
                [
                    SNew(SBox)
                    .WidthOverride(70.0f)
                    [
                        SNew(SEditableTextBox)
                        .HintText(FText::FromString(TEXT("Any")))
                        .OnTextCommitted(this, &SObjectNetToolbar::OnConnectionCommitted)
                    ]
                ]

                + SHorizontalBox::Slot()
                .AutoWidth()
                .VAlign(VAlign_Center)
                [
                    SNew(STextBlock).Text(FText::FromString(TEXT("Time Start")))
                ]

                + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(6.0f, 0.0f)
                [
                    SNew(SBox)
                    .WidthOverride(90.0f)
                    [
                        SNew(SEditableTextBox)
                        .HintText(FText::FromString(TEXT("Any")))
                        .OnTextCommitted(this, &SObjectNetToolbar::OnTimeStartCommitted)
                    ]
                ]

                + SHorizontalBox::Slot()
                .AutoWidth()
                .VAlign(VAlign_Center)
                [
                    SNew(STextBlock).Text(FText::FromString(TEXT("Time End")))
                ]

                + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(6.0f, 0.0f)
                [
                    SNew(SBox)
                    .WidthOverride(90.0f)
                    [
                        SNew(SEditableTextBox)
                        .HintText(FText::FromString(TEXT("Any")))
                        .OnTextCommitted(this, &SObjectNetToolbar::OnTimeEndCommitted)
                    ]
                ]

                + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(12.0f, 0.0f, 0.0f, 0.0f)
                .VAlign(VAlign_Center)
                [
                    SNew(SCheckBox)
                    .IsChecked(this, &SObjectNetToolbar::GetRpcChecked)
                    .OnCheckStateChanged(this, &SObjectNetToolbar::OnRpcChanged)
                    [
                        SNew(STextBlock).Text(FText::FromString(TEXT("RPC")))
                    ]
                ]

                + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(8.0f, 0.0f, 0.0f, 0.0f)
                .VAlign(VAlign_Center)
                [
                    SNew(SCheckBox)
                    .IsChecked(this, &SObjectNetToolbar::GetPropertyChecked)
                    .OnCheckStateChanged(this, &SObjectNetToolbar::OnPropertyChanged)
                    [
                        SNew(STextBlock).Text(FText::FromString(TEXT("Property")))
                    ]
                ]

                + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(8.0f, 0.0f, 0.0f, 0.0f)
                .VAlign(VAlign_Center)
                [
                    SNew(SCheckBox)
                    .IsChecked(this, &SObjectNetToolbar::GetOutgoingOnlyChecked)
                    .OnCheckStateChanged(this, &SObjectNetToolbar::OnOutgoingOnlyChanged)
                    [
                        SNew(STextBlock).Text(FText::FromString(TEXT("Outgoing Only")))
                    ]
                ]
            ]
        ];
    }

private:
    void ApplyQuery()
    {
        Provider->SetQuery(CachedQuery);
    }

    void OnSearchCommitted(const FText& InText, ETextCommit::Type)
    {
        CachedQuery.SearchText = InText.ToString();
        ApplyQuery();
    }

    void OnConnectionCommitted(const FText& InText, ETextCommit::Type)
    {
        const FString Value = InText.ToString().TrimStartAndEnd();
        if (Value.IsEmpty())
        {
            CachedQuery.ConnectionFilter.Reset();
        }
        else
        {
            int32 Parsed = 0;
            if (FDefaultValueHelper::ParseInt(Value, Parsed) && Parsed >= 0)
            {
                CachedQuery.ConnectionFilter = static_cast<uint32>(Parsed);
            }
        }
        ApplyQuery();
    }

    void OnTimeStartCommitted(const FText& InText, ETextCommit::Type)
    {
        const FString Value = InText.ToString().TrimStartAndEnd();
        if (Value.IsEmpty())
        {
            CachedQuery.TimeStartSec.Reset();
        }
        else
        {
            double Parsed = 0.0;
            if (FDefaultValueHelper::ParseDouble(Value, Parsed))
            {
                CachedQuery.TimeStartSec = Parsed;
            }
        }
        ApplyQuery();
    }

    void OnTimeEndCommitted(const FText& InText, ETextCommit::Type)
    {
        const FString Value = InText.ToString().TrimStartAndEnd();
        if (Value.IsEmpty())
        {
            CachedQuery.TimeEndSec.Reset();
        }
        else
        {
            double Parsed = 0.0;
            if (FDefaultValueHelper::ParseDouble(Value, Parsed))
            {
                CachedQuery.TimeEndSec = Parsed;
            }
        }
        ApplyQuery();
    }

    void OnRpcChanged(const ECheckBoxState InState)
    {
        CachedQuery.bIncludeRpc = (InState == ECheckBoxState::Checked);
        ApplyQuery();
    }

    void OnPropertyChanged(const ECheckBoxState InState)
    {
        CachedQuery.bIncludeProperties = (InState == ECheckBoxState::Checked);
        ApplyQuery();
    }

    void OnOutgoingOnlyChanged(const ECheckBoxState InState)
    {
        if (InState == ECheckBoxState::Checked)
        {
            CachedQuery.DirectionFilter = EObjectNetDirection::Outgoing;
        }
        else
        {
            CachedQuery.DirectionFilter.Reset();
        }
        ApplyQuery();
    }

    ECheckBoxState GetRpcChecked() const
    {
        return CachedQuery.bIncludeRpc ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
    }

    ECheckBoxState GetPropertyChecked() const
    {
        return CachedQuery.bIncludeProperties ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
    }

    ECheckBoxState GetOutgoingOnlyChecked() const
    {
        return (CachedQuery.DirectionFilter.IsSet() && CachedQuery.DirectionFilter.GetValue() == EObjectNetDirection::Outgoing)
            ? ECheckBoxState::Checked
            : ECheckBoxState::Unchecked;
    }

    FReply OnRefreshClicked()
    {
        Provider->Refresh();
        return FReply::Handled();
    }

private:
    TSharedPtr<FObjectNetProvider> Provider;
    FObjectNetQuery CachedQuery;
};

TSharedRef<SWidget> MakeObjectNetToolbarWidget(const TSharedRef<FObjectNetProvider>& Provider)
{
    return SNew(SObjectNetToolbar, Provider);
}
