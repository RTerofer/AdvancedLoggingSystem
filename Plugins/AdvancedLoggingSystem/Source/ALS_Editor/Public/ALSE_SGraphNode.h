//Copyright © 2025 RTerofer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ALS_Definitions.h"
#include "SGraphNode.h"

struct FPresetItem
{
    EPrintPreset Mode;

    FPresetItem(EPrintPreset InMode)
    {
        Mode = InMode;
    }
};


class ALS_EDITOR_API UALS_SGraphNode : public SGraphNode
{
public:

    SLATE_BEGIN_ARGS(UALS_SGraphNode) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, UEdGraphNode* InNode);

    virtual void CreatePinWidgets() override;
    virtual void CreateInputSideAddButton(TSharedPtr<SVerticalBox> InputBox) override;

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 3
    virtual void CreateAdvancedViewArrow(TSharedPtr<SVerticalBox> MainBox) override;
#else
    virtual void OnAdvancedViewChanged(const ECheckBoxState NewCheckedState) override;
#endif

    TSharedPtr<SGraphPin> CreatePinWidget(UEdGraphPin* Pin) const override;
    virtual TSharedRef<SWidget> CreateTitleWidget(TSharedPtr<SNodeTitle> NodeTitle);

    TSharedRef<SWidget> MakePresetRow(TSharedPtr<FPresetItem> InItem) const;
    FText GetPresetText() const;
    TArray<TSharedPtr<FPresetItem>> PresetItems;

    void OnPresetChosen(TSharedPtr<FPresetItem> NewSelection, ESelectInfo::Type SelectInfo);
    FReply OnAddPinClicked();
    FReply On3DDebugClicked();
    void OnActiveStateChanged(ECheckBoxState NewState);
};
