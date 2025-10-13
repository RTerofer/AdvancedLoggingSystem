//Copyright © 2025 RTerofer. All Rights Reserved.

#include "ALSE_Commands.h"

#define LOCTEXT_NAMESPACE "FALSEditorModule"

void FALSCommands::RegisterCommands()
{
    UI_COMMAND(ToolbarMenu, "ALS", "Main Menu for ALS", EUserInterfaceActionType::Button, FInputChord());
    UI_COMMAND(OpenListsForALS, "Manage PrintString (ALS)", "Manage all ALS PrintString nodes in this Project", EUserInterfaceActionType::Button, FInputChord());
    UI_COMMAND(OpenListsForUPS, "Manage PrintString (Regular)", "Manage all Regular PrintString nodes in this Project", EUserInterfaceActionType::Button, FInputChord());
    UI_COMMAND(OpenLogViewer, "Open Logs Viewer", "Open LogsViewer In Editor", EUserInterfaceActionType::Button, FInputChord());
    UI_COMMAND(OpenDocumentation, "Documentation", "Open Github Documentation", EUserInterfaceActionType::Button, FInputChord());
    UI_COMMAND(OpenSupport, "Support Forum", "Open ALS Forum for Support and Questions", EUserInterfaceActionType::Button, FInputChord());
    UI_COMMAND(OpenFeedback, "Feedback & Rating", "Found this plugin useful? Leave a rating or suggestion to encourage improvements", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE

