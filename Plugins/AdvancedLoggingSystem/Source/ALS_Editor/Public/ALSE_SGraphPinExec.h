//Copyright © 2025 RTerofer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "KismetPins/SGraphPinExec.h"

class ALS_EDITOR_API UALS_SGraphPinExec : public SGraphPinExec
{
public:
    SLATE_BEGIN_ARGS(UALS_SGraphPinExec) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, UEdGraphPin* InPin);

    void SetCustomTooltip();

protected:
    virtual FReply OnMouseButtonDown(const FGeometry& SenderGeometry, const FPointerEvent& MouseEvent) override;

    virtual EVisibility GetDefaultValueVisibility() const override;

    TSharedRef<SWidget> GetDefaultValueWidget() override;

    void OnDefaultTextCommitted(const FText& NewText, ETextCommit::Type CommitInfo);

    UEdGraphPin* OwnerPin;

private:
    TSharedPtr<SWidget> CreateContextMenu();
};
