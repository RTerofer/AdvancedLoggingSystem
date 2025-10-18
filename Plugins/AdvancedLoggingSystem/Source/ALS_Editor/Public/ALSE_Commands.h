//Copyright © 2025 RTerofer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "ALSE_Style.h"
#include "Styling/AppStyle.h"

class ALS_EDITOR_API FALSCommands : public TCommands<FALSCommands>
{
public:
    FALSCommands()
        : TCommands<FALSCommands>(TEXT("ALS"), NSLOCTEXT("ALS", "ALS", "ALS Plugin"), NAME_None, FALSStyle::GetStyleSetName())
    {
    }

    virtual void RegisterCommands() override;

    TSharedPtr<FUICommandInfo> ToolbarMenu;
    TSharedPtr<FUICommandInfo> OpenListsForALS;
    TSharedPtr<FUICommandInfo> OpenListsForUPS;
    TSharedPtr<FUICommandInfo> OpenLogViewer;
    TSharedPtr<FUICommandInfo> OpenDocumentation;
    TSharedPtr<FUICommandInfo> OpenGithub;
    TSharedPtr<FUICommandInfo> OpenForum;
    TSharedPtr<FUICommandInfo> OpenFeedback;
};
