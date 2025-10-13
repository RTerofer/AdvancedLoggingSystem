//Copyright © 2025 RTerofer. All Rights Reserved.

#pragma once

#include "Framework/Application/IInputProcessor.h"
#include "Framework/Commands/InputChord.h"

class ALS_API FALSInputProcessor : public IInputProcessor
{
public:
    FALSInputProcessor();

    virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override {};

    virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& KeyEvent) override;

    bool DoesChordMatch(const FInputChord& InChord, const FKeyEvent& InEvent) const;

private:
    FInputChord LogsChord, PropertyChord;
};
