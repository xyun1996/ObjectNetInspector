#include "ObjectNetProvider.h"

#include "Widgets/SCompoundWidget.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSplitter.h"

TSharedRef<SWidget> MakeObjectNetToolbarWidget(const TSharedRef<FObjectNetProvider>& Provider);
TSharedRef<SWidget> MakeObjectNetObjectListWidget(const TSharedRef<FObjectNetProvider>& Provider);
TSharedRef<SWidget> MakeObjectNetDetailViewWidget(const TSharedRef<FObjectNetProvider>& Provider);
TSharedRef<SWidget> MakeObjectNetEventTableWidget(const TSharedRef<FObjectNetProvider>& Provider);

class SObjectNetInspectorTab : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SObjectNetInspectorTab)
    {
    }
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs)
    {
        Provider = MakeShared<FObjectNetProvider>();
        Provider->Refresh();

        ChildSlot
        [
            SNew(SSplitter)
            .Orientation(Orient_Vertical)

            + SSplitter::Slot()
            .Value(0.16f)
            [
                SNew(SBorder)
                .Padding(6.0f)
                [
                    MakeObjectNetToolbarWidget(Provider.ToSharedRef())
                ]
            ]

            + SSplitter::Slot()
            .Value(0.84f)
            [
                SNew(SSplitter)
                .Orientation(Orient_Horizontal)

                + SSplitter::Slot()
                .Value(0.42f)
                [
                    SNew(SBorder)
                    .Padding(4.0f)
                    [
                        MakeObjectNetObjectListWidget(Provider.ToSharedRef())
                    ]
                ]

                + SSplitter::Slot()
                .Value(0.58f)
                [
                    SNew(SSplitter)
                    .Orientation(Orient_Vertical)

                    + SSplitter::Slot()
                    .Value(0.42f)
                    [
                        SNew(SBorder)
                        .Padding(4.0f)
                        [
                            MakeObjectNetDetailViewWidget(Provider.ToSharedRef())
                        ]
                    ]

                    + SSplitter::Slot()
                    .Value(0.58f)
                    [
                        SNew(SBorder)
                        .Padding(4.0f)
                        [
                            MakeObjectNetEventTableWidget(Provider.ToSharedRef())
                        ]
                    ]
                ]
            ]
        ];
    }

private:
    TSharedPtr<FObjectNetProvider> Provider;
};
