//Copyright © 2025 RTerofer. All Rights Reserved.

#include "ALSE_SGraphPinExec.h"
#include "ALSE_Style.h"
#include "ALSE_Node.h"
#include "EdGraphSchema_K2.h"
#include "ScopedTransaction.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/Colors/SColorBlock.h"
#include "Widgets/Colors/SColorPicker.h"


void UALS_SGraphPinExec::Construct(const FArguments& InArgs, UEdGraphPin* InPin)
{
    OwnerPin = InPin;
    this->SetCursor(EMouseCursor::CardinalCross);
    SGraphPinExec::GraphPinObj = InPin;
    SGraphPinExec::Construct(SGraphPinExec::FArguments(), InPin);

    SetCustomTooltip();
}

void UALS_SGraphPinExec::SetCustomTooltip()
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

FReply UALS_SGraphPinExec::OnMouseButtonDown(const FGeometry& SenderGeometry, const FPointerEvent& MouseEvent)
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
    return SGraphPinExec::OnMouseButtonDown(SenderGeometry, MouseEvent);
}

EVisibility UALS_SGraphPinExec::GetDefaultValueVisibility() const
{
    return EVisibility::Visible;
}

TSharedPtr<SWidget> UALS_SGraphPinExec::CreateContextMenu()
{
    FMenuBuilder MenuBuilder(true, nullptr);

    UEdGraphPin* Pin = OwnerPin;
    if (!Pin) return SNullWidget::NullWidget;

    UALS_Node* Node = Cast<UALS_Node>(Pin->GetOwningNode());
    if (!Node) return SNullWidget::NullWidget;

    if (Pin->Direction == EGPD_Input && Pin->LinkedTo.Num() == 0 && Pin->bAdvancedView == false)
    {
        if (Pin->PinId != Node->ExecPinId)
        {
            MenuBuilder.AddMenuEntry(
                FText::FromString("Remove Pin"),
                FText::FromString("Remove this Unconnected Exec pin."),
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
                    FCanExecuteAction::CreateLambda([]() { return false; })  // Disabled
                )
            );
        }
    }

    return MenuBuilder.MakeWidget();
}

void UALS_SGraphPinExec::OnDefaultTextCommitted(const FText& NewText, ETextCommit::Type CommitInfo)
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
                const FScopedTransaction Transaction(NSLOCTEXT("ALS", "UndoText", "ExecDefaultText"));
                Node->Modify();
                Node->GetGraph()->Modify();

                Pin->DefaultValue = NewValue;
                Node->SetDefaultValue(Pin);

                Node->MarkBlueprintDirty();
            }
        }
    }
}

TSharedRef<SWidget> UALS_SGraphPinExec::GetDefaultValueWidget()
{
    UEdGraphPin* Pin = OwnerPin;
    if (!Pin) return SNullWidget::NullWidget;

    UALS_Node* Node = Cast<UALS_Node>(Pin->GetOwningNode());
    if (!Node) return SNullWidget::NullWidget;

    FString SavedDefault = Node->GetSavedDefaultValue(Pin);

    TSharedRef<SEditableTextBox> EditableTextBox = SNew(SEditableTextBox)
        .HintText(FText::FromString(""))
        .Text(FText::FromString(Node->GetSavedDefaultValue(Pin)))
        .ToolTipText(FText::FromString("Text given would be fired when this execution gets triggered"))
        .OnTextCommitted(this, &UALS_SGraphPinExec::OnDefaultTextCommitted)
        .SelectAllTextWhenFocused(true)
        .SelectAllTextOnCommit(false)
        .ClearKeyboardFocusOnCommit(true);

    TSharedPtr<SHorizontalBox> HorizontalBox = SNew(SHorizontalBox)
        + SHorizontalBox::Slot()
        .AutoWidth()
        [
            EditableTextBox
        ];

    if (!SavedDefault.IsEmpty())
    {
        HorizontalBox->AddSlot()
            .FillWidth(1.0f)
            .VAlign(VAlign_Center)
            .Padding(FMargin(1, 0, 0, 0))
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



