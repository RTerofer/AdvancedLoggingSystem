//Copyright © 2025 RTerofer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SGraphPin.h"
#include "ALS_Definitions.h"
#include "ALSE_Node.h"

struct FOutputModeItem
{
    EPrintMode Mode;

    FOutputModeItem(EPrintMode InMode)
    {
        Mode = InMode;
    }
};

struct FLogSeverityItem
{
    ELogSeverity Mode;

    FLogSeverityItem(ELogSeverity InMode)
    {
        Mode = InMode;
    }
};

class ALS_EDITOR_API UALS_SGraphPin : public SGraphPin
{
public:
    SLATE_BEGIN_ARGS(UALS_SGraphPin) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, UEdGraphPin* InPin);

    void SetCustomTooltip();

protected:
    // Override mouse button handling to add context menu
    virtual FReply OnMouseButtonDown(const FGeometry& SenderGeometry, const FPointerEvent& MouseEvent) override;

    TSharedRef<SWidget> GetDefaultValueWidget() override;

    FVector GetStoredVectorValue() const;

    TOptional<float> GetVectorX() const { return GetStoredVectorValue().X; }
    TOptional<float> GetVectorY() const { return GetStoredVectorValue().Y; }
    TOptional<float> GetVectorZ() const { return GetStoredVectorValue().Z; }

    void OnDefaultVectorCommitted(float NewValue, ETextCommit::Type CommitType, int32 Axis);

    FLinearColor GetColorFromPin() const;

    FReply OnColorClicked(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);

    void OnDefaultColorPicked(FLinearColor NewColor);

    void OnDefaultTextCommitted(const FText& NewText, ETextCommit::Type CommitInfo);

    void OnDefaultFloatCommitted(const float NewValue, ETextCommit::Type CommitInfo);

    void OnOutputModeChosen(TSharedPtr<FOutputModeItem> NewSelection, ESelectInfo::Type SelectInfo);

    void OnLogSeverityChosen(TSharedPtr<FLogSeverityItem> NewSelection, ESelectInfo::Type SelectInfo);

    TSharedRef<SWidget> MakeOutputModeRow(TSharedPtr<FOutputModeItem> InItem) const;
    FText GetOutputModeText() const;

    TSharedRef<SWidget> MakeLogSeverityRow(TSharedPtr<FLogSeverityItem> InItem) const;
    FText GetLogSeverityText() const;

    UEdGraphPin* OwnerPin;

    TArray<TSharedPtr<FOutputModeItem>> OutputModeItems;
    EPrintMode CurrentOutputMode = EPrintMode::ScreenAndLog;

    TArray<TSharedPtr<FLogSeverityItem>> LogSeverityItems;
    ELogSeverity CurrentLogSeverity = ELogSeverity::Warning;

private:
    TSharedPtr<SWidget> CreateContextMenu();
};
