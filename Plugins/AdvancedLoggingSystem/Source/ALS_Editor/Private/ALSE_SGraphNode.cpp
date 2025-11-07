//Copyright © 2025 RTerofer. All Rights Reserved.

#include "ALSE_SGraphNode.h"
#include "ALSE_Node.h"
#include "ALSE_Style.h"
#include "ALSE_SGraphPin.h"
#include "ALSE_SGraphPinExec.h"
#include "EdGraphSchema_K2.h"
#include "ScopedTransaction.h"
#include "KismetPins/SGraphPinExec.h"
#include "Widgets/Input/SComboBox.h"
#include "SGraphPanel.h"

void UALS_SGraphNode::Construct(const FArguments& InArgs, UEdGraphNode* InNode)
{
    GraphNode = InNode;

    if (UALS_Node* Node = Cast<UALS_Node>(GraphNode))
    {
        GraphNode->AdvancedPinDisplay = Node->SavedAdvancedPinState;
    }

    SetCursor(EMouseCursor::CardinalCross);
    UpdateGraphNode();
}

void UALS_SGraphNode::CreatePinWidgets()
{
    if (UALS_Node* Node = Cast<UALS_Node>(GraphNode))
    {
        for (UEdGraphPin* CurPin : GraphNode->Pins)
        {
            if (CurPin->PinId == Node->TextLocationPinId && !Node->bPrintToWorld) continue;
            if (CurPin->PinId == Node->WorldContextId && !Node->IsNonContextBP()) continue;

            if (TSharedPtr<SGraphPin> NewPin = CreatePinWidget(CurPin))
            {
                if (CurPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Exec)
                {
                    NewPin->SetPadding(FMargin(1.0f, -1.0f, 1.0f, -1.0f));
                }

                AddPin(NewPin.ToSharedRef());
            }
        }
    }
}

TSharedRef<SWidget> UALS_SGraphNode::MakePresetRow(TSharedPtr<FPresetItem> InItem) const
{
    FText Label = FText::FromString("Unknown");
    if (InItem.IsValid())
    {
        switch (InItem->Mode)
        {
        case EPrintPreset::PrintInfo:
            Label = FText::FromString("PrintInfo");
            break;
        case EPrintPreset::PrintWarn:
            Label = FText::FromString("PrintWarn");
            break;
        case EPrintPreset::PrintError:
            Label = FText::FromString("PrintError");
            break;
        case EPrintPreset::LogInfo:
            Label = FText::FromString("LogInfo");
            break;
        case EPrintPreset::LogWarn:
            Label = FText::FromString("LogWarn");
            break;
        case EPrintPreset::LogError:
            Label = FText::FromString("LogError");
            break;
        case EPrintPreset::Print3D:
            Label = FText::FromString("Print3D");
            break;
        }
    }
    return SNew(STextBlock).Text(Label);
}

FText UALS_SGraphNode::GetPresetText() const
{
    if (UALS_Node* Node = Cast<UALS_Node>(GraphNode))
    {
        switch (Node->PrintPreset)
        {
        case EPrintPreset::PrintInfo:
            return FText::FromString("PI");
        case EPrintPreset::PrintWarn:
            return FText::FromString("PW");
        case EPrintPreset::PrintError:
            return FText::FromString("PE");
        case EPrintPreset::LogInfo:
            return FText::FromString("LI");
        case EPrintPreset::LogWarn:
            return FText::FromString("LW");
        case EPrintPreset::LogError:
            return FText::FromString("LE");
        case EPrintPreset::Print3D:
            return FText::FromString("P3D");
        }
    }

    return FText();
}

void UALS_SGraphNode::OnActiveStateChanged(ECheckBoxState NewState)
{
    if (UALS_Node* Node = Cast<UALS_Node>(GraphNode))
    {
        const FScopedTransaction Transaction(NSLOCTEXT("ALS", "UndoText", "NodeActiveState"));
        Node->Modify();
        Node->GetGraph()->Modify();

        Node->bToggleNode = (NewState == ECheckBoxState::Checked);

        Node->MarkBlueprintDirty();
    }
}

void UALS_SGraphNode::CreateInputSideAddButton(TSharedPtr<SVerticalBox> InputBox)
{
    UALS_Node* Node = Cast<UALS_Node>(GraphNode);
    if (!Node) return;

    TSharedRef<SButton> AddPinButton = SNew(SButton)
        .Text(NSLOCTEXT("ALS", "AddPin", " + Pin"))
        .ContentPadding(FMargin(-3, 2, -3, 0))
        .ToolTipText(FText::FromString("Add a new input pin to append a value. Maximum 15 pins allowed."))
        .OnClicked(this, &UALS_SGraphNode::OnAddPinClicked)
        .IsEnabled_Lambda([Node]() -> bool
            {
                return (Node->GetWildcardPinCount() < Node->MaximumInputs);
            });

    TSharedRef<SButton> Toggle3DButton = SNew(SButton)
        .Text(NSLOCTEXT("ALS","3DDebug", "3D"))
        .ToolTipText(FText::FromString("Enable this to draw the debug message in world space."))
        .ContentPadding(FMargin(-3, 2, -5, 0))
        .OnClicked(this, &UALS_SGraphNode::On3DDebugClicked)
        .ForegroundColor_Lambda([Node]() -> FSlateColor
            {
                return Node->bPrintToWorld ? FSlateColor(FColor::Orange) : FSlateColor(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f));
            });

    TSharedRef<SHorizontalBox> FirstButtonBox = SNew(SHorizontalBox);
    FirstButtonBox->AddSlot()
        .AutoWidth()
        .HAlign(HAlign_Left)
        .Padding(2, 1)
        [
            AddPinButton
        ];

    FirstButtonBox->AddSlot()
        .AutoWidth()
        .HAlign(HAlign_Left)
        .Padding(2, 1)
        [
            Toggle3DButton
        ];

    if (PresetItems.IsEmpty())
    {
        PresetItems.Add(MakeShared<FPresetItem>(EPrintPreset::PrintInfo));
        PresetItems.Add(MakeShared<FPresetItem>(EPrintPreset::PrintWarn));
        PresetItems.Add(MakeShared<FPresetItem>(EPrintPreset::PrintError));
        PresetItems.Add(MakeShared<FPresetItem>(EPrintPreset::LogInfo));
        PresetItems.Add(MakeShared<FPresetItem>(EPrintPreset::LogWarn));
        PresetItems.Add(MakeShared<FPresetItem>(EPrintPreset::LogError));
        PresetItems.Add(MakeShared<FPresetItem>(EPrintPreset::Print3D));
    }

    FirstButtonBox->AddSlot()
        .AutoWidth()
        .HAlign(HAlign_Left)
        .Padding(2, 1)
        [
            SNew(SComboBox<TSharedPtr<FPresetItem>>)
                .ContentPadding(FMargin(1, 0, -7, 0))
                .ToolTipText(FText::FromString("Changing this will affect all the Configs to the Selected Preset Config"))
                .OptionsSource(&PresetItems)
                .HasDownArrow(true)
                .OnGenerateWidget(this, &UALS_SGraphNode::MakePresetRow)
                .OnSelectionChanged(this, &UALS_SGraphNode::OnPresetChosen)
                [
                    SNew(STextBlock).Text(this, &UALS_SGraphNode::GetPresetText)
                ]
        ];

    InputBox->AddSlot()
        .AutoHeight()
        .HAlign(HAlign_Fill)
        .VAlign(VAlign_Top)
        .Padding(10, 2, -10, 5)
        [
            FirstButtonBox
        ];
}

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 3

void UALS_SGraphNode::CreateAdvancedViewArrow(TSharedPtr<SVerticalBox> MainBox)
{
    UALS_Node* Node = Cast<UALS_Node>(GraphNode);
    if (!Node) return;

    MainBox->AddSlot()
        .AutoHeight()
        .HAlign(HAlign_Fill)
        .VAlign(VAlign_Top)
        .Padding(3, 1, 3, 3)
        [
            SNew(SCheckBox)
                .Visibility_Lambda([Node]() ->EVisibility
                    {
                        bool bShowAdvancedViewArrow = Node && (ENodeAdvancedPins::NoPins != Node->AdvancedPinDisplay);
                        return bShowAdvancedViewArrow ? EVisibility::Visible : EVisibility::Collapsed;
                    })
                .OnCheckStateChanged_Lambda([Node](ECheckBoxState NewCheckedState)
                    {
                        if (Node && (ENodeAdvancedPins::NoPins != Node->AdvancedPinDisplay))
                        {
                            bool bAdvancedPinsHidden = (NewCheckedState != ECheckBoxState::Checked);
                            Node->AdvancedPinDisplay = bAdvancedPinsHidden ? ENodeAdvancedPins::Hidden : ENodeAdvancedPins::Shown;
                            Node->SavedAdvancedPinState = Node->AdvancedPinDisplay;
                        }
                    })
                        .IsChecked_Lambda([Node]() -> ECheckBoxState
                            {
                                bool bAdvancedPinsHidden = Node && (ENodeAdvancedPins::Hidden == Node->AdvancedPinDisplay);
                                return bAdvancedPinsHidden ? ECheckBoxState::Unchecked : ECheckBoxState::Checked;
                            })
                        .Style(FAppStyle::Get(), "Graph.Node.AdvancedView")
                                [
                                    SNew(SHorizontalBox)
                                        + SHorizontalBox::Slot()
                                        .VAlign(VAlign_Center)
                                        .HAlign(HAlign_Center)
                                        [
                                            SNew(SImage)
                                                .Image_Lambda([Node]() -> const FSlateBrush*
                                                    {
                                                        bool bAdvancedPinsHidden = Node && (ENodeAdvancedPins::Hidden == Node->AdvancedPinDisplay);
                                                        return FAppStyle::GetBrush(bAdvancedPinsHidden ? TEXT("Icons.ChevronDown") : TEXT("Icons.ChevronUp"));
                                                    })
                                        ]
                                ]
        ];
}

#else

void UALS_SGraphNode::OnAdvancedViewChanged(const ECheckBoxState NewCheckedState)
{
    UALS_Node* Node = Cast<UALS_Node>(GraphNode);

    if (Node && (ENodeAdvancedPins::NoPins != Node->AdvancedPinDisplay))
    {
        bool bAdvancedPinsHidden = (NewCheckedState != ECheckBoxState::Checked);
        Node->AdvancedPinDisplay = bAdvancedPinsHidden ? ENodeAdvancedPins::Hidden : ENodeAdvancedPins::Shown;
        Node->SavedAdvancedPinState = Node->AdvancedPinDisplay;
    }
}

#endif

void UALS_SGraphNode::OnPresetChosen(TSharedPtr<FPresetItem> NewSelection, ESelectInfo::Type SelectInfo)
{
    if (!NewSelection.IsValid()) return;

    if (UALS_Node* Node = Cast<UALS_Node>(GraphNode))
    {
        const FScopedTransaction Transaction(NSLOCTEXT("ALS", "UndoPreset", "PresetChange"));
        Node->Modify();
        Node->GetGraph()->Modify();

        Node->PrintPreset = NewSelection->Mode;
        Node->SetPinDefaultsByPreset();

        Node->MarkBlueprintDirty();
    }
}

FReply UALS_SGraphNode::OnAddPinClicked()
{
    if (UALS_Node* Node = Cast<UALS_Node>(GraphNode))
    {
        const FScopedTransaction Transaction(NSLOCTEXT("ALS", "UndoText", "PinAddition"));
        Node->Modify();
        Node->GetGraph()->Modify();

        Node->CreateWildcardPin();

        Node->ReconstructNode();
        Node->MarkBlueprintDirty();

        return FReply::Handled();
    }
    return FReply::Unhandled();
}

FReply UALS_SGraphNode::On3DDebugClicked()
{
    if (UALS_Node* Node = Cast<UALS_Node>(GraphNode))
    {
        const FScopedTransaction Transaction(NSLOCTEXT("ALS", "UndoText", "3DToggle"));
        Node->Modify();
        Node->GetGraph()->Modify();

        Node->bPrintToWorld = !Node->bPrintToWorld;
        Node->UpdateTextLocationPin();

        Node->MarkBlueprintDirty();

        return FReply::Handled();
    }

    return FReply::Unhandled();
}

TSharedPtr<SGraphPin> UALS_SGraphNode::CreatePinWidget(UEdGraphPin* Pin) const
{
    if (Pin->PinType.PinCategory == UEdGraphSchema_K2::PC_Exec)
    {
        return SNew(UALS_SGraphPinExec, Pin);
    }
    
    return SNew(UALS_SGraphPin, Pin); 
}

TSharedRef<SWidget> UALS_SGraphNode::CreateTitleWidget(TSharedPtr<SNodeTitle> NodeTitle)
{
    UALS_Node* Node = Cast<UALS_Node>(GraphNode);
    if (!Node) return SNullWidget::NullWidget;

    FString FNodeTitle = FString::Printf(TEXT("%s"), Node->bPrintToWorld ? TEXT("Print String (3D)") : TEXT("Print String"));
    FText NodeTitleText = FText::FromString(FNodeTitle);

    UEnum* OutputType = StaticEnum<EPrintMode>();
    UEnum* LevelType = StaticEnum<ELogSeverity>();
    FString DurationStr = FString::SanitizeFloat(Node->PrintDuration);

    FText OutputModeText = OutputType->GetDisplayValueAsText(Node->PrintMode);
    FText LogSeverityText = LevelType->GetDisplayValueAsText(Node->LogSeverity);
    FText DurationText = FText::FromString(DurationStr);

    FText SubTitleText;

    if (Node->PrintMode != EPrintMode::LogOnly)
    {
        SubTitleText = FText::Format(NSLOCTEXT("ALS", "NodeSubTitle", "{0}, {1}, {2}"), OutputModeText, LogSeverityText, DurationText);
    }
    else
    {
        SubTitleText = FText::Format(NSLOCTEXT("ALS", "NodeSubTitle", "{0}, {1}"), OutputModeText, LogSeverityText);
    }

    FSlateFontInfo HeadingFont = FAppStyle::Get().GetFontStyle("NormalFontBold");
    HeadingFont.Size = 10;

    FSlateFontInfo SubHeadingFont = FAppStyle::Get().GetFontStyle("NormalFont");
    SubHeadingFont.Size = 6;

    TSharedRef<SButton> Toggle3DButton = SNew(SButton)
        .Text(NSLOCTEXT("ALS", "3DDebug", "3D"))
        .ToolTipText(NSLOCTEXT("ALS", "3DPinToolTip", "Enable this to draw the debug message in world space."))
        .ContentPadding(FMargin(-3, 1, -5, 1))
        .OnClicked(this, &UALS_SGraphNode::On3DDebugClicked)
        .ButtonColorAndOpacity_Lambda([Node]() -> FSlateColor
            {
                return Node->bPrintToWorld ? FSlateColor(FLinearColor::Green) : FSlateColor(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f));
            });

    return SNew(SBorder)
        .Padding(FMargin(0.0f, 0.0f, -8.5f, 1.0f))
        .BorderImage(FAppStyle::GetBrush("NoBorder"))
        [
            SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .AutoHeight()
                [
                    SNew(SHorizontalBox)
                        + SHorizontalBox::Slot()
                        .AutoWidth()
                        .VAlign(VAlign_Top)
                        .HAlign(HAlign_Left)
                        .Padding(2, 1)
                        [
                            SNew(STextBlock)
                                .Text(NodeTitleText)
                                .Font(HeadingFont)
                                .ColorAndOpacity(FColor::White)
                                .RenderOpacity(0.9)
                        ]

                        + SHorizontalBox::Slot()
                        .FillWidth(0.2)
                        .VAlign(VAlign_Top)
                        .HAlign(HAlign_Right)
                        .Padding(5, 2, 0, 0)
                        [
                            SNew(SCheckBox)
                                .OnCheckStateChanged(this, &UALS_SGraphNode::OnActiveStateChanged)
                                .ToolTipText(NSLOCTEXT("ALS", "NodeActivePinTooltip", "Set Node Active"))
                                .IsChecked_Lambda([Node]() -> ECheckBoxState
                                    {
                                        return Node->bToggleNode ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
                                    })
                        ]
                ]

                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(2, 1)
                [
                    SNew(STextBlock)
                        .Text(SubTitleText)
                        .Font(SubHeadingFont)
                        .ColorAndOpacity(FColor::White)
                        .RenderOpacity(0.85)
                ]
        ];
}

#undef LOCTEXT_NAMESPACE
