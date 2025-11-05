//Copyright © 2025 RTerofer. All Rights Reserved.

#include "ALSE_SGraphPin.h"
#include "ALSE_Style.h"
#include "EdGraphSchema_K2.h"
#include "ScopedTransaction.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/Colors/SColorBlock.h"
#include "Widgets/Colors/SColorBlock.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Input/SVectorInputBox.h"
#include "KismetPins/SGraphPinString.h"
#include "Widgets/Colors/SColorPicker.h"
#include "DetailLayoutBuilder.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"

void UALS_SGraphPin::Construct(const FArguments& InArgs, UEdGraphPin* InPin)
{
    OwnerPin = InPin; 
    this->SetCursor(EMouseCursor::CardinalCross);

    SGraphPin::Construct(SGraphPin::FArguments(), InPin);
    SGraphPin::GraphPinObj = InPin;

    SetCustomTooltip();
}

void UALS_SGraphPin::SetCustomTooltip()
{
    if (OwnerPin)
    {
        if (UALS_Node* Node = Cast<UALS_Node>(OwnerPin->GetOwningNode()))
        {
            const FString& TipText = Node->GetSavedPinData(OwnerPin).PinToolTip;
            SetToolTip(SNew(SToolTip).Text(FText::FromString(TipText)));
        }
    }
}

FReply UALS_SGraphPin::OnMouseButtonDown(const FGeometry& SenderGeometry, const FPointerEvent& MouseEvent)
{
    if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
    {
        FSlateApplication::Get().PushMenu(
            SharedThis(this),
            FWidgetPath(),
            CreateContextMenu().ToSharedRef(),
            MouseEvent.GetScreenSpacePosition(),
            FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu)
        );
        return FReply::Handled();
    }
    return SGraphPin::OnMouseButtonDown(SenderGeometry, MouseEvent);
}

TSharedPtr<SWidget> UALS_SGraphPin::CreateContextMenu()
{
    FMenuBuilder MenuBuilder(true, nullptr);

    UEdGraphPin* Pin = OwnerPin;
    if (Pin)
    {
        UALS_Node* Node = Cast<UALS_Node>(Pin->GetOwningNode());
        if (Node)
        {
            TArray<UEdGraphPin*> Pins = Pin->GetOwningNode()->Pins;

            if (Pin->Direction == EGPD_Input && Pin->LinkedTo.Num() == 0 && !Node->AdvancedPins.Contains(Pin->PinId) && Pin->PinId != Node->TextLocationPinId)
            {
                MenuBuilder.AddMenuEntry(
                    FText::FromString("Remove Pin"),
                    FText::FromString("Remove this unconnected input pin."),
                    FSlateIcon(),
                    FUIAction(FExecuteAction::CreateLambda([this, Pin, Node]()
                        {
                           Node->RemoveInputPin(Pin);                    
                        }))
                );
            }
            else
            {
                MenuBuilder.AddMenuEntry(
                    FText::FromString("Remove Pin"),
                    FText::FromString("Cannot remove this pin"),
                    FSlateIcon(),
                    FUIAction(
                        FExecuteAction(),
                        FCanExecuteAction::CreateLambda([]() { return false; }) 
                    )
                );
            }
        }
    }


    return MenuBuilder.MakeWidget();
}

void UALS_SGraphPin::OnDefaultTextCommitted(const FText& NewText, ETextCommit::Type CommitInfo)
{
    UEdGraphPin* Pin = OwnerPin;
    if (Pin)
    {
        UALS_Node* Node = Cast<UALS_Node>(Pin->GetOwningNode());
        if (Node)
        {
            FString NewValue = NewText.ToString();

            if (Pin->DefaultValue != NewValue)
            {
                const FScopedTransaction Transaction(NSLOCTEXT("ALS", "UndoText", "PinDefaultText"));
                Node->Modify();
                Node->GetGraph()->Modify();

                Pin->DefaultValue = NewValue;
                Node->SetDefaultValue(Pin);

                Node->MarkBlueprintDirty();
                return;
            }
        }
    }
}

void UALS_SGraphPin::OnDefaultFloatCommitted(const float NewValue, ETextCommit::Type CommitInfo)
{
    UEdGraphPin* Pin = OwnerPin;
    if (Pin)
    {
        UALS_Node* Node = Cast<UALS_Node>(Pin->GetOwningNode());
        if (Node)
        {
            FString NewStr = FString::SanitizeFloat(NewValue);

            if (Pin->DefaultValue != NewStr)
            {
                const FScopedTransaction Transaction(NSLOCTEXT("ALS", "UndoText", "PrintDuration"));
                Node->Modify();
                Node->GetGraph()->Modify();

                Pin->DefaultValue = NewStr;
                Node->PrintDuration = NewValue;
                Node->SetDefaultValue(Pin);

                Node->MarkBlueprintDirty();
                return;
            }
        }
    }
}

FVector UALS_SGraphPin::GetStoredVectorValue() const
{
    UEdGraphPin* Pin = OwnerPin;
    FVector Value = FVector::ZeroVector;

    if (Pin)
    {
        UALS_Node* Node = Cast<UALS_Node>(Pin->GetOwningNode());
        if (Node)
        {
            Value.InitFromString(Node->GetSavedDefaultValue(Pin));        
        }
    }

    return Value;
}

void UALS_SGraphPin::OnDefaultVectorCommitted(float NewValue, ETextCommit::Type CommitType, int32 Axis)
{
    UEdGraphPin* Pin = OwnerPin;
    if (Pin)
    {
        UALS_Node* Node = Cast<UALS_Node>(Pin->GetOwningNode());
        if (Node)
        {
            FVector CurrentValue = GetStoredVectorValue();

            switch (Axis)
            {
                case 0: CurrentValue.X = NewValue; break;
                case 1: CurrentValue.Y = NewValue; break;
                case 2: CurrentValue.Z = NewValue; break;
            }

            FString NewVectorValue = CurrentValue.ToString();

            if (Pin->DefaultValue != NewVectorValue)
            {
                const FScopedTransaction Transaction(NSLOCTEXT("ALS", "UndoText", "TextLocation"));
                Node->Modify();
                Node->GetGraph()->Modify();

                Pin->DefaultValue = NewVectorValue;
                Node->SetDefaultValue(Pin);

                Node->MarkBlueprintDirty();
                return;
            }
        }
    }
}

void UALS_SGraphPin::OnDefaultColorPicked(FLinearColor NewColor)
{
    UEdGraphPin* Pin = OwnerPin;

    if (Pin)
    {
        UALS_Node* Node = Cast<UALS_Node>(Pin->GetOwningNode());
        if (Node)
        {
            FString NewColorString = NewColor.ToString();

            if (Pin->DefaultValue != NewColorString)
            {
                const FScopedTransaction Transaction(NSLOCTEXT("ALS", "UndoText", "PrintColor"));
                Node->Modify();
                Node->GetGraph()->Modify();

                Pin->DefaultValue = NewColorString;
                Node->PrintColor = NewColor;
                Node->SetDefaultValue(Pin);

                Node->MarkBlueprintDirty(false);
                return;
            }
        }
    }
    return;

}

void UALS_SGraphPin::OnOutputModeChosen(TSharedPtr<FOutputModeItem> NewSelection, ESelectInfo::Type SelectInfo)
{
    if (!NewSelection.IsValid()) return;

    UEdGraphPin* Pin = OwnerPin;

    if (Pin)
    {
        UALS_Node* Node = Cast<UALS_Node>(Pin->GetOwningNode());

        if (Node)
        {
            CurrentOutputMode = NewSelection->Mode;
            UEnum* EnumPtr = StaticEnum<EPrintMode>();
            FString NewEnumString = EnumPtr->GetNameStringByValue((int64)CurrentOutputMode);

            if (Pin->DefaultValue != NewEnumString)
            {
                const FScopedTransaction Transaction(NSLOCTEXT("ALS", "UndoText", "PrintMode"));
                Node->Modify();
                Node->GetGraph()->Modify();

                Pin->DefaultValue = NewEnumString;
                Node->PrintMode = CurrentOutputMode;
                Node->SetDefaultValue(Pin);

                Node->MarkBlueprintDirty();
                return;
            }
        }
    }
}

void UALS_SGraphPin::OnLogSeverityChosen(TSharedPtr<FLogSeverityItem> NewSelection, ESelectInfo::Type SelectInfo)
{
    if (!NewSelection.IsValid()) return;

    UEdGraphPin* Pin = OwnerPin;

    if (Pin)
    {
        UALS_Node* Node = Cast<UALS_Node>(Pin->GetOwningNode());

        if (Node)
        {
            CurrentLogSeverity = NewSelection->Mode;
            UEnum* EnumPtr = StaticEnum<ELogSeverity>();
            FString NewEnumString = EnumPtr->GetNameStringByValue((int64)CurrentLogSeverity);

            if (Pin->DefaultValue != NewEnumString)
            {
                const FScopedTransaction Transaction(NSLOCTEXT("ALS", "UndoText", "LogSeverity"));
                Node->Modify();
                Node->GetGraph()->Modify();

                Pin->DefaultValue = NewEnumString;
                Node->LogSeverity = CurrentLogSeverity;
                Node->SetDefaultValue(Pin);

                Node->MarkBlueprintDirty();
                return;
            }
        }
    }
}

TSharedRef<SWidget> UALS_SGraphPin::MakeOutputModeRow(TSharedPtr<FOutputModeItem> InItem) const
{
    FText Label = FText::FromString("Unknown");
    if (InItem.IsValid())
    {
        switch (InItem->Mode)
        {
        case EPrintMode::ScreenOnly:   
            Label = FText::FromString("Screen Only"); 
            break;
        case EPrintMode::LogOnly:      
            Label = FText::FromString("Log Only");
            break;
        case EPrintMode::ScreenAndLog: 
            Label = FText::FromString("Screen & Log");
            break;
        }
    }
    return SNew(STextBlock).Text(Label);
}

TSharedRef<SWidget> UALS_SGraphPin::MakeLogSeverityRow(TSharedPtr<FLogSeverityItem> InItem) const
{
    FText Label = FText::FromString("Unknown");
    if (InItem.IsValid())
    {
        switch (InItem->Mode)
        {
        case ELogSeverity::Info:
            Label = FText::FromString("Info");
            break;
        case ELogSeverity::Warning:
            Label = FText::FromString("Warning");
            break;
        case ELogSeverity::Error:
            Label = FText::FromString("Error");
            break;
        }
    }

    return SNew(STextBlock).Text(Label);
}

FText UALS_SGraphPin::GetOutputModeText() const
{
    switch (CurrentOutputMode)
    {
    case EPrintMode::ScreenOnly:   
        return FText::FromString("Screen Only");
    case EPrintMode::LogOnly:      
        return FText::FromString("Log Only");
    case EPrintMode::ScreenAndLog: 
        return FText::FromString("Screen & Log");
    default:                       
        return FText::FromString("Unknown");
    }
}

FText UALS_SGraphPin::GetLogSeverityText() const
{
    switch (CurrentLogSeverity)
    {
    case ELogSeverity::Info:         
        return FText::FromString("Info");
    case ELogSeverity::Warning:      
        return FText::FromString("Warning");
    case ELogSeverity::Error:        
        return FText::FromString("Error");  
    default:                      
        return FText::FromString("Unknown");
    }
}

FLinearColor UALS_SGraphPin::GetColorFromPin() const
{
    UEdGraphPin* Pin = OwnerPin;

    if (Pin)
    {
        UALS_Node* Node = Cast<UALS_Node>(Pin->GetOwningNode());

        if (Node)
        {
            FLinearColor ParsedColor = FLinearColor::White;
            ParsedColor.InitFromString(Node->GetSavedDefaultValue(Pin));

            return ParsedColor;
        }
    }

    return FLinearColor::Yellow;
}

FReply UALS_SGraphPin::OnColorClicked(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    UEdGraphPin* Pin = OwnerPin;
    if (Pin)
    {
        UALS_Node* Node = Cast<UALS_Node>(Pin->GetOwningNode());

        if (Node)
        {
            FColorPickerArgs PickerArgs;
            PickerArgs.InitialColor = GetColorFromPin();
            PickerArgs.OnColorCommitted = FOnLinearColorValueChanged::CreateSP(this, &UALS_SGraphPin::OnDefaultColorPicked);
            OpenColorPicker(PickerArgs);

            return FReply::Handled();
        }
    }

    return FReply::Unhandled();
}

TSharedRef<SWidget> UALS_SGraphPin::GetDefaultValueWidget()
{
    UEdGraphPin* Pin = OwnerPin;
    if (!Pin)
    {
        return SNullWidget::NullWidget;
    }

    UALS_Node* Node = Cast<UALS_Node>(Pin->GetOwningNode());

    if (!Node)
    {
        return SNullWidget::NullWidget;
    }

    FString SavedDefault = Node->GetSavedDefaultValue(Pin);

    if (Pin->PinId == Node->TextLocationPinId && Node->bPrintToWorld)
    {
        if (Pin->HasAnyConnections())
        {
            if (Pin->LinkedTo[0]->PinType.PinCategory == UEdGraphSchema_K2::PC_Struct)
            {
                return SNullWidget::NullWidget;
            }
        }

        return SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().Padding(0)
            [
                SNew(SNumericEntryBox<float>)
                    .ToolTipText(FText::FromString("X Value/Offset of the Text Location"))
                    .LabelVAlign(VAlign_Center)
                    .Value(this, &UALS_SGraphPin::GetVectorX)
                    .OnValueCommitted(this, &UALS_SGraphPin::OnDefaultVectorCommitted, 0)
                    .Label()
                    [
                        SNew(STextBlock).Text(FText::FromString("X"))
                    ]
            ]
            + SHorizontalBox::Slot().AutoWidth().Padding(0)
            [
                SNew(SNumericEntryBox<float>)
                    .ToolTipText(FText::FromString("Y Value/Offset of the Text Location"))
                    .LabelVAlign(VAlign_Center)
                    .Value(this, &UALS_SGraphPin::GetVectorY)
                    .OnValueCommitted(this, &UALS_SGraphPin::OnDefaultVectorCommitted, 1)
                    .Label()
                    [
                        SNew(STextBlock).Text(FText::FromString("Y"))
                    ]
            ]
            + SHorizontalBox::Slot().AutoWidth().Padding(0)
            [
                SNew(SNumericEntryBox<float>)
                    .ToolTipText(FText::FromString("Z Value/Offset of the Text Location"))
                    .LabelVAlign(VAlign_Center)
                    .Value(this, &UALS_SGraphPin::GetVectorZ)
                    .OnValueCommitted(this, &UALS_SGraphPin::OnDefaultVectorCommitted, 2)
                    .Label()
                    [
                        SNew(STextBlock).Text(FText::FromString("Z"))
                    ]
            ];
    }

    if (Pin->PinId == Node->DurationPinId)
    {
        if (Pin->HasAnyConnections())
        {
            return SNullWidget::NullWidget;
        }

        float Value = FCString::Atof(*SavedDefault);
        return SNew(SSpinBox<float>)
            .ToolTipText(FText::FromString("Time in secs the message stays visible on screen."))
            .MinValue(0.0f)
            .Delta(0.1f)
            .Value(Value)
            .OnValueCommitted(this, &UALS_SGraphPin::OnDefaultFloatCommitted);
    }

    if (Pin->PinId == Node->ColorPinId)
    {
        if (Pin->HasAnyConnections())
        {
            return SNullWidget::NullWidget;
        }

        return SNew(SColorBlock)
            .ToolTipText(FText::FromString("Color of the On-Screen message."))
            .Color(this, &UALS_SGraphPin::GetColorFromPin)
            .OnMouseButtonDown(this, &UALS_SGraphPin::OnColorClicked);
    }

    if (Pin->PinId == Node->PrintModePinId)
    {
        if (Pin->HasAnyConnections())
        {
            return SNullWidget::NullWidget;
        }

        if (OutputModeItems.IsEmpty())
        {
            OutputModeItems.Add(MakeShared<FOutputModeItem>(EPrintMode::ScreenOnly));
            OutputModeItems.Add(MakeShared<FOutputModeItem>(EPrintMode::LogOnly));
            OutputModeItems.Add(MakeShared<FOutputModeItem>(EPrintMode::ScreenAndLog));
        }

        CurrentOutputMode = EPrintMode::ScreenAndLog;
        if (UEnum* EnumPtr = StaticEnum<EPrintMode>())
        {
            if (!Pin->DefaultValue.IsEmpty())
            {
                const int64 FoundVal = EnumPtr->GetValueByNameString(SavedDefault);
                if (FoundVal != INDEX_NONE)
                {
                    CurrentOutputMode = static_cast<EPrintMode>(FoundVal);
                }
            }
        }

        return SNew(SComboBox<TSharedPtr<FOutputModeItem>>)
            .ToolTipText(FText::FromString("Choose how the message is output: on screen, log or both.d"))
            .ContentPadding(FMargin(2, 2, 2, 2))
            .OptionsSource(&OutputModeItems)
            .HasDownArrow(true)
            .OnGenerateWidget(this, &UALS_SGraphPin::MakeOutputModeRow)
            .OnSelectionChanged(this, &UALS_SGraphPin::OnOutputModeChosen)
            [
                SNew(STextBlock)
                    .Text(this, &UALS_SGraphPin::GetOutputModeText)
            ];
    }

    if (Pin->PinId == Node->LogSeverityPinId)
    {
        if (Pin->HasAnyConnections())
        {
            return SNullWidget::NullWidget;
        }

        if (OutputModeItems.IsEmpty())
        {
            LogSeverityItems.Add(MakeShared<FLogSeverityItem>(ELogSeverity::Info));
            LogSeverityItems.Add(MakeShared<FLogSeverityItem>(ELogSeverity::Warning));
            LogSeverityItems.Add(MakeShared<FLogSeverityItem>(ELogSeverity::Error));
        }

        CurrentLogSeverity = ELogSeverity::Warning;
        if (UEnum* EnumPtr = StaticEnum<ELogSeverity>())
        {
            if (!Pin->DefaultValue.IsEmpty())
            {
                const int64 FoundVal = EnumPtr->GetValueByNameString(SavedDefault);
                if (FoundVal != INDEX_NONE)
                {
                    CurrentLogSeverity = static_cast<ELogSeverity>(FoundVal);
                }
            }
        }

        return SNew(SComboBox<TSharedPtr<FLogSeverityItem>>)
            .ToolTipText(FText::FromString("Set the severity level to Info, Warning or Error."))
            .OptionsSource(&LogSeverityItems)
            .ContentPadding(FMargin(2, 2, 2, 2))
            .HasDownArrow(true)
            .OnGenerateWidget(this, &UALS_SGraphPin::MakeLogSeverityRow)
            .OnSelectionChanged(this, &UALS_SGraphPin::OnLogSeverityChosen)
            [
                SNew(STextBlock)
                    .Text(this, &UALS_SGraphPin::GetLogSeverityText)
            ];
    }

    if (Pin->PinId == Node->WorldContextId)
    {
        return SNullWidget::NullWidget;
    }

    if (Pin->PinId == Node->KeyPinId)
    {
        if (Pin->HasAnyConnections())
        {
            return SNullWidget::NullWidget;
        }

        TSharedRef<SEditableTextBox> EditableTextBox = SNew(SEditableTextBox)
            .Padding(FMargin(5, 4, 5, 4))
            .Text(FText::FromString(SavedDefault))
            .HintText(FText::FromString("In String"))
            .ToolTipText(FText::FromString("When Key Provided, the Messages of same key will be replaced. Useful in tick messages"))
            .OnTextCommitted(this, &UALS_SGraphPin::OnDefaultTextCommitted)
            .SelectAllTextWhenFocused(true)
            .SelectAllTextOnCommit(false)
            .ClearKeyboardFocusOnCommit(true)
            .Font(FAppStyle::Get().GetFontStyle("Graph.NormalFont"))
            .Style(FAppStyle::Get(), "Graph.EditableTextBox")
            .OnKeyDownHandler(FOnKeyDown::CreateLambda([this](const FGeometry& MyGeometry, const FKeyEvent& KeyEvent) -> FReply
                {
                    if (KeyEvent.GetKey() == EKeys::Enter)
                    {
                        if (KeyEvent.IsShiftDown())
                        {
                            return FReply::Unhandled();
                        }
                        else
                        {
                            FSlateApplication::Get().ClearKeyboardFocus(EFocusCause::Cleared);
                            return FReply::Handled();
                        }
                    }
                    return FReply::Unhandled();
                }));

        TSharedPtr<SHorizontalBox> HorizontalBox = SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(FMargin(0, 0, 0, 0))
            [
                EditableTextBox
            ];

        if (!SavedDefault.IsEmpty())
        {
            HorizontalBox->AddSlot()
                .AutoWidth()
                .VAlign(VAlign_Center)
                .Padding(FMargin(3.5f, 0, 0, 0))
                [
                    SNew(SButton)
                        .ButtonStyle(FAppStyle::Get(), "HoverHintOnly")
                        .ToolTipText(FText::FromString("Clear Text"))
                        .OnClicked_Lambda([this, EditableTextBox, Node]() -> FReply
                            {
                                EditableTextBox->SetText(FText::FromName(NAME_None));
                                OnDefaultTextCommitted(FText::FromName(NAME_None), ETextCommit::Default);
                                return FReply::Handled();
                            })
                        [
                            SNew(SImage)
                                .Image(FALSStyle::Get().GetBrush("ALS.Undo"))
                        ]
                ];
        }

        return HorizontalBox.ToSharedRef();
    }

    if (Node->IsWildcardPin(Pin))
    {
        if (Pin->HasAnyConnections())
        {
            return SNullWidget::NullWidget;
        }

        TSharedRef<SMultiLineEditableTextBox> EditableTextBox = SNew(SMultiLineEditableTextBox)
            .Padding(FMargin(5, 4, 5, 4))
            .Text(FText::FromString(SavedDefault))
            .HintText(FText::FromString("In String"))
            .ToolTipText(FText::FromString("Given String will be printed or logged"))
            .OnTextCommitted(this, &UALS_SGraphPin::OnDefaultTextCommitted)
            .SelectAllTextWhenFocused(true)
            .SelectAllTextOnCommit(false)
            .ClearKeyboardFocusOnCommit(true)
            .Font(FAppStyle::Get().GetFontStyle("Graph.NormalFont"))
            .Style(FAppStyle::Get(), "Graph.EditableTextBox")
            .OnKeyDownHandler(FOnKeyDown::CreateLambda([this](const FGeometry& MyGeometry, const FKeyEvent& KeyEvent) -> FReply
                {
                    if (KeyEvent.GetKey() == EKeys::Enter)
                    {
                        if (KeyEvent.IsShiftDown())
                        {
                            return FReply::Unhandled(); 
                        }
                        else
                        {
                            FSlateApplication::Get().ClearKeyboardFocus(EFocusCause::Cleared);
                            return FReply::Handled();
                        }
                    }
                    return FReply::Unhandled();
                }));

        TSharedPtr<SHorizontalBox> HorizontalBox = SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(FMargin(0, 0, 0, 0))
            [
                EditableTextBox
            ];

        if (!SavedDefault.IsEmpty())
        {
            HorizontalBox->AddSlot()
                .AutoWidth()
                .VAlign(VAlign_Center)
                .Padding(FMargin(3.5f, 0, 0, 0))
                [
                    SNew(SButton)
                        .ButtonStyle(FAppStyle::Get(), "HoverHintOnly")
                        .ToolTipText(FText::FromString("Clear Text"))
                        .OnClicked_Lambda([this, EditableTextBox, Node]() -> FReply
                            {
                                EditableTextBox->SetText(FText::FromString(""));
                                OnDefaultTextCommitted(FText::FromString(""), ETextCommit::Default);
                                return FReply::Handled();
                            })
                        [
                            SNew(SImage)
                                .Image(FALSStyle::Get().GetBrush("ALS.Undo"))
                        ]
                ];
        }

        return HorizontalBox.ToSharedRef();
    }

    return SNullWidget::NullWidget;
}



