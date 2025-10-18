//Copyright © 2025 RTerofer. All Rights Reserved.

#include "ALSE.h"
#include "ALSE_Style.h"
#include "ALSE_Commands.h"
#include "ALS_Settings.h"
#include "ScopedTransaction.h"
#include "Blueprint/UserWidget.h"
#include "BlueprintNodeSpawner.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "ToolMenus.h"
#include "LevelEditor.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/Input/SButton.h"
#include "EditorUtilityBlueprint.h"
#include "WidgetBlueprint.h"
#include "Animation/AnimBlueprint.h"
#include "Framework/Application/SlateApplication.h"
#include "Engine/LevelScriptBlueprint.h"
#include "Editor/EditorEngine.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "K2Node_CallFunction.h"
#include "Kismet/GameplayStatics.h"
#include "ISettingsModule.h"

static const FName ALSEditorTabName("ALSEditor");
#define LOCTEXT_NAMESPACE "FALSEditorModule"

FDelegateHandle BeginPIEDelegateHandle;

static FText GetOutputModeText(EPrintMode Mode)
{
    switch (Mode)
    {
    case EPrintMode::ScreenOnly:      return FText::FromString("ScreenOnly");
    case EPrintMode::LogOnly:         return FText::FromString("LogOnly");
    case EPrintMode::ScreenAndLog:    return FText::FromString("Screen&Log");
    default:                          return FText::FromString("Mixed");
    }
}

static FText GetLogSeverityText(ELogSeverity Mode)
{
    switch (Mode)
    {
    case ELogSeverity::Info:         return FText::FromString("Info");
    case ELogSeverity::Warning:      return FText::FromString("Warning");
    case ELogSeverity::Error :       return FText::FromString("Error");
    default:                         return FText::FromString("Mixed");
    }
}

void FALSEditorModule::StartupModule()
{
    // Node registration
    UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(UALS_Node::StaticClass());
    FBlueprintActionDatabase::Get().GetAllActions();
    FBlueprintActionDatabase::Get().RefreshClassActions(UALS_Node::StaticClass());

    // Style initialization
    FALSStyle::Initialize();
    FALSStyle::ReloadTextures();

    FALSCommands::Register();

    if (GEditor)
    {
        GEditor->RegisterForUndo(this);
    }

    CommandsList = MakeShareable(new FUICommandList);

    // Command bindings
    CommandsList->MapAction(
        FALSCommands::Get().ToolbarMenu,
        FExecuteAction::CreateLambda([]()
            {
                //Nothing Here
            })
    );

    CommandsList->MapAction(
        FALSCommands::Get().OpenListsForALS,
        FExecuteAction::CreateRaw(this, &FALSEditorModule::ShowBlueprintWindowForALS)
    );

    CommandsList->MapAction(
        FALSCommands::Get().OpenListsForUPS,
        FExecuteAction::CreateRaw(this, &FALSEditorModule::ShowBlueprintWindowForUPS)
    );

    CommandsList->MapAction(
        FALSCommands::Get().OpenLogViewer,
        FExecuteAction::CreateRaw(this, &FALSEditorModule::ShowAllALSLogs)
    );

    CommandsList->MapAction(
        FALSCommands::Get().OpenDocumentation,
        FExecuteAction::CreateLambda([]()
            {
                FPlatformProcess::LaunchURL(TEXT("https://rterofer.gitbook.io/advanced-logging-system-plugin-documentation"), nullptr, nullptr);
            })
    );

    CommandsList->MapAction(
        FALSCommands::Get().OpenGithub,
        FExecuteAction::CreateLambda([]()
            {
                FPlatformProcess::LaunchURL(TEXT("https://github.com/RTerofer/AdvancedLoggingSystem"), nullptr, nullptr);
            })
    );

    CommandsList->MapAction(
        FALSCommands::Get().OpenForum,
        FExecuteAction::CreateLambda([]()
            {
                FPlatformProcess::LaunchURL(TEXT("https://forums.unrealengine.com/t/advanced-logging-system-plugin/2513455"), nullptr, nullptr);
            })
    );

    CommandsList->MapAction(
        FALSCommands::Get().OpenFeedback,
        FExecuteAction::CreateLambda([]()
            {
                FPlatformProcess::LaunchURL(TEXT("https://fab.com/s/64efe02d3f99"), nullptr, nullptr);
            })
    );

    UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FALSEditorModule::RegisterMenus));
}

void FALSEditorModule::ShutdownModule()
{
    PendingCheckedStateALS.Empty();
    PendingCheckedStateUPS.Empty();
    PendingOutputModesALS.Empty();
    PendingLogSeverityALS.Empty();

    PrintModeRowCache.Empty();
    LogSeverityRowCache.Empty();

    if (GEditor)
    {
        GEditor->UnregisterForUndo(this);
    }

    UToolMenus::UnregisterOwner(this);
}

void FALSEditorModule::RegisterMenus()
{
    FToolMenuOwnerScoped OwnerScoped(this);

    UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
    FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");

    Section.AddEntry(
        FToolMenuEntry::InitComboButton(
            "ALSActions",
            FUIAction(),
            FOnGetContent::CreateRaw(this, &FALSEditorModule::GenerateToolbarMenu),
            LOCTEXT("ALS_Label", "ALS"),
            LOCTEXT("ALS_Tooltip", "Click to Open ALS Menu"),
            FSlateIcon(FALSStyle::GetStyleSetName(), "ALS.ToolBar"),
            false
        )
    ).SetCommandList(CommandsList);
}

TSharedRef<SWidget> FALSEditorModule::GenerateToolbarMenu()
{
    FMenuBuilder MenuBuilder(true, CommandsList);

    MenuBuilder.BeginSection("BatchOperations", LOCTEXT("BatchOperations", "Batch Operations"));
    {
        MenuBuilder.AddMenuEntry(FALSCommands::Get().OpenListsForALS);
        MenuBuilder.AddMenuEntry(FALSCommands::Get().OpenListsForUPS);
    }
    MenuBuilder.EndSection();

    MenuBuilder.BeginSection("LogsViewer", LOCTEXT("LogsViewer", "Logs Viewer"));
    {
        MenuBuilder.AddMenuEntry(FALSCommands::Get().OpenLogViewer);
    }
    MenuBuilder.EndSection();

    MenuBuilder.BeginSection("Support", LOCTEXT("Support", "Support"));
    {
        MenuBuilder.AddMenuEntry(FALSCommands::Get().OpenDocumentation);
        MenuBuilder.AddMenuEntry(FALSCommands::Get().OpenGithub);
        MenuBuilder.AddMenuEntry(FALSCommands::Get().OpenForum);
        MenuBuilder.AddMenuEntry(FALSCommands::Get().OpenFeedback);
    }
    MenuBuilder.EndSection();

    return MenuBuilder.MakeWidget();
}

// ALS Manager
void FALSEditorModule::ShowBlueprintWindowForALS()
{
    if (ExistingALSWindow.IsValid())
    {
        ExistingALSWindow->RequestDestroyWindow();
        ExistingALSWindow.Reset();
    }

    if (ExistingUPSWindow.IsValid())
    {
        ExistingUPSWindow->RequestDestroyWindow();
        ExistingUPSWindow.Reset();
    }

    TArray<FAssetData> ALSBlueprints;
    GetBlueprintsWithALSNodes(ALSBlueprints);

    if (ALSBlueprints.IsEmpty())
    {
        FMessageDialog::Open(EAppMsgType::Ok, FText::FromString
        ("No ALS nodes detected across Project Blueprints\nAdd one to a Blueprint and reopen this window to manage ALS nodes."));

        return;
    }

    ExistingALSWindow = SNew(SWindow)
        .Title(FText::FromString("Manage PrintString (ALS) Nodes"))
        .ClientSize(FVector2D(700, FMath::Clamp(ALSBlueprints.Num() * 45 + 150, 300, 600)))
        .SizingRule(ESizingRule::UserSized);

    TSharedRef<SVerticalBox> MainBox = SNew(SVerticalBox);

    // Heading row (BP Name | LogSeverity | Print Mode | Is Enabled | | Delete)
    MainBox->AddSlot()
        .AutoHeight().Padding(5.f)
        [
            SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1.0f).Padding(10)
                [
                    SNew(STextBlock).Text(FText::FromString("BP NAME")).Font(FAppStyle::GetFontStyle("BoldFont")).Justification(ETextJustify::Left)
                ]
                + SHorizontalBox::Slot().FillWidth(0.55f).Padding(10)
                [
                    SNew(STextBlock).Text(FText::FromString("PRINT MODE")).Font(FAppStyle::GetFontStyle("BoldFont")).Justification(ETextJustify::Center)
                ]
                + SHorizontalBox::Slot().FillWidth(0.5f).Padding(10)
                [
                    SNew(STextBlock).Text(FText::FromString("LOG LEVEL")).Font(FAppStyle::GetFontStyle("BoldFont")).Justification(ETextJustify::Center)
                ]
                + SHorizontalBox::Slot().FillWidth(0.3f).Padding(10)
                [
                    SNew(STextBlock).Text(FText::FromString("ON/OFF")).Font(FAppStyle::GetFontStyle("BoldFont")).Justification(ETextJustify::Center)
                ]
                + SHorizontalBox::Slot().FillWidth(0.5f).Padding(10)
                [
                    SNew(STextBlock).Text(FText::FromString("DELETE")).Font(FAppStyle::GetFontStyle("BoldFont")).Justification(ETextJustify::Center)
                ]
        ];

    PrintModeRowCache.Empty();
    LogSeverityRowCache.Empty();

    ALSScrollContainer = SNew(SScrollBox);
    for (const FAssetData& Blueprint : ALSBlueprints)
    {
        ALSScrollContainer->AddSlot()
            [
                CreateALSRow(Blueprint, ALSBlueprints)
            ];
    }

    MainBox->AddSlot().FillHeight(1.f).Padding(5)[ALSScrollContainer.ToSharedRef()];

    ExistingALSWindow->SetOnWindowClosed(FOnWindowClosed::CreateLambda([this](const TSharedRef<SWindow>& Window)
        {
            PendingOutputModesALS.Empty();
            PendingLogSeverityALS.Empty();
            PendingCheckedStateALS.Empty();
        }));

    ExistingALSWindow->SetContent(MainBox);
    FSlateApplication::Get().AddWindow(ExistingALSWindow.ToSharedRef());
}

TSharedRef<SWidget> FALSEditorModule::CreateALSRow(const FAssetData& Blueprint, TArray<FAssetData>& RelevantBlueprints)
{
    TSharedPtr<FPrintModeRowData> PrintModePtr = MakeShared<FPrintModeRowData>();
    PrintModePtr->Blueprint = Blueprint;
    PrintModePtr->ComboOptions.Empty();
    PrintModePtr->ComboOptions.Add(MakeShared<EPrintMode>(EPrintMode::ScreenOnly));
    PrintModePtr->ComboOptions.Add(MakeShared<EPrintMode>(EPrintMode::LogOnly));
    PrintModePtr->ComboOptions.Add(MakeShared<EPrintMode>(EPrintMode::ScreenAndLog));
    PrintModeRowCache.Add(PrintModePtr);

    TSharedPtr<FLogSeverityRowData> LogSeverityPtr = MakeShared<FLogSeverityRowData>();
    LogSeverityPtr->Blueprint = Blueprint;
    LogSeverityPtr->ComboOptions.Empty();
    LogSeverityPtr->ComboOptions.Add(MakeShared<ELogSeverity>(ELogSeverity::Info));
    LogSeverityPtr->ComboOptions.Add(MakeShared<ELogSeverity>(ELogSeverity::Warning));
    LogSeverityPtr->ComboOptions.Add(MakeShared<ELogSeverity>(ELogSeverity::Error));
    LogSeverityRowCache.Add(LogSeverityPtr);

    TSharedRef<SWidget> RowWidget = SNew(SHorizontalBox)
        // BP NAME
        + SHorizontalBox::Slot().FillWidth(1.0f).Padding(5, 2).HAlign(HAlign_Left)
        [
            SNew(STextBlock).Text(FText::FromName(Blueprint.AssetName))
        ]

        // PRINT MODE COMBOBOX
        +SHorizontalBox::Slot().FillWidth(0.55f).Padding(10, 2)
        [
            SNew(SComboBox<TSharedPtr<EPrintMode>>)
                .OptionsSource(&PrintModePtr->ComboOptions)
                .OnGenerateWidget_Lambda([](TSharedPtr<EPrintMode> Item)
                    {
                        return SNew(STextBlock).Text(GetOutputModeText(*Item));
                    })
                [
                    SNew(STextBlock).Text_Lambda([this, Blueprint]()
                        {
                            const EPrintMode CurrentMode = GetBlueprintALSOutputMode(Blueprint);
                            return GetOutputModeText(CurrentMode);
                        })
                ]
                .OnSelectionChanged_Lambda([this, Blueprint](TSharedPtr<EPrintMode> NewItem, ESelectInfo::Type)
                    {
                        if (NewItem.IsValid())
                        {
                            PendingOutputModesALS.FindOrAdd(Blueprint) = *NewItem;
                            ApplyPendingChangesForALS(Blueprint);
                        }
                    })
                [
                    SNew(STextBlock).Text_Lambda([this, Blueprint]()
                        {
                            if (const EPrintMode* Mode = PendingOutputModesALS.Find(Blueprint))
                                return GetOutputModeText(*Mode);

                            EPrintMode Mode = GetBlueprintALSOutputMode(Blueprint);
                            return GetOutputModeText(Mode);
                        })
                ]
                .ToolTipText(LOCTEXT("PrintMode_Tooltip", "Choose how the message is output: on screen, log or both."))

        ]


        // LOG LEVEL COMBOBOX
        + SHorizontalBox::Slot().FillWidth(0.5f).Padding(10, 2)
        [
            SNew(SComboBox<TSharedPtr<ELogSeverity>>)
                .OptionsSource(&LogSeverityPtr->ComboOptions)
                .OnGenerateWidget_Lambda([](TSharedPtr<ELogSeverity> Item)
                    {
                        return SNew(STextBlock).Text(GetLogSeverityText(*Item));
                    })
                [
                    SNew(STextBlock).Text_Lambda([this, Blueprint]()
                        {
                            const ELogSeverity CurrentMode = GetBlueprintALSLogSeverity(Blueprint);
                            return GetLogSeverityText(CurrentMode);
                        })
                ]
                .OnSelectionChanged_Lambda([this, Blueprint](TSharedPtr<ELogSeverity> NewItem, ESelectInfo::Type)
                    {
                        if (NewItem.IsValid())
                        {
                            PendingLogSeverityALS.FindOrAdd(Blueprint) = *NewItem;
                            ApplyPendingChangesForALS(Blueprint);
                        }
                    })
                [
                    SNew(STextBlock).Text_Lambda([this, Blueprint]()
                        {
                            if (const ELogSeverity* Mode = PendingLogSeverityALS.Find(Blueprint))
                                return GetLogSeverityText(*Mode);

                            ELogSeverity Mode = GetBlueprintALSLogSeverity(Blueprint);
                            return GetLogSeverityText(Mode);
                        })
                ]
                .ToolTipText(LOCTEXT("LogSeverity_Tooltip", "Set the severity level to Info, Warning or Error."))

        ]

        // IS ENABLED CHECKBOX
        +SHorizontalBox::Slot().FillWidth(0.3f).Padding(10, 2).HAlign(HAlign_Center)
        [
            SNew(SCheckBox)
                .IsChecked(GetBlueprintALSCheckedState(Blueprint))
                .OnCheckStateChanged_Lambda([this, Blueprint](ECheckBoxState NewState)
                    {
                        PendingCheckedStateALS.Add(Blueprint, NewState);
                        ApplyPendingChangesForALS(Blueprint);
                    })
                .ToolTipText(LOCTEXT("IsEnabled_Tooltip", "Toggle the node’s active state. Unchecked nodes will skip printing."))
        ]

        // DELETE BUTTON
        + SHorizontalBox::Slot().FillWidth(0.5f).Padding(10, 2)
        [
            SNew(SButton)
                .Text(FText::FromString("Delete"))
                .OnClicked_Lambda([this, Blueprint, &RelevantBlueprints]()
                    {
                        EAppReturnType::Type Result = FMessageDialog::Open(EAppMsgType::OkCancel,
                        FText::FromString("Are you sure you want to delete all ALS nodes in " + Blueprint.AssetName.ToString() + "?\n\nNOTE: Make sure to compile and save after verifying. You can also revert this action by Undo"));

                        if (Result == EAppReturnType::Ok)
                        {
                            DeleteALSNode(Blueprint);
                        }

                        return FReply::Handled();
                    })
                .ToolTipText(LOCTEXT("Delete_Tooltip", "Delete the ALS Print String's found in this BP while preserving its exec chains."))
        ];

     return RowWidget;
}

void FALSEditorModule::GetBlueprintsWithALSNodes(TArray<FAssetData>& OutBlueprints) const
{
    auto BlueprintContainsALS = [](const FAssetData& BlueprintAsset) -> bool
        {
            UBlueprint* Blueprint = Cast<UBlueprint>(BlueprintAsset.GetAsset());
            if (!Blueprint) return false;

            TArray<UEdGraph*> AllGraphs;
            Blueprint->GetAllGraphs(AllGraphs);

            for (UEdGraph* Graph : AllGraphs)
            {
                for (UEdGraphNode* Node : Graph->Nodes)
                {
                    if (Cast<UALS_Node>(Node))
                    {
                        return true;
                    }
                }
            }

            return false;
        };

    OutBlueprints.Empty();

    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (World && World->PersistentLevel)
    {
        if (ULevelScriptBlueprint* LevelBP = Cast<ULevelScriptBlueprint>(World->PersistentLevel->GetLevelScriptBlueprint()))
        {
            FAssetData LevelAsset(LevelBP);
            if (BlueprintContainsALS(LevelAsset))
            {
                OutBlueprints.Add(LevelAsset);
            }
        }
    }

    FARFilter Filter;
    Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
    Filter.ClassPaths.Add(UWidgetBlueprint::StaticClass()->GetClassPathName());
    Filter.ClassPaths.Add(UAnimBlueprint::StaticClass()->GetClassPathName());
    Filter.ClassPaths.Add(UEditorUtilityBlueprint::StaticClass()->GetClassPathName());
    Filter.bRecursivePaths = true;

    Filter.PackagePaths.Add("/Game");

    TArray<FAssetData> FilteredAssets;
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    AssetRegistryModule.Get().GetAssets(Filter, FilteredAssets);

    for (FAssetData& Asset : FilteredAssets)
    {
        if (BlueprintContainsALS(Asset))
        {
            OutBlueprints.Add(Asset);
        }
    }
}

EPrintMode FALSEditorModule::GetBlueprintALSOutputMode(const FAssetData& BlueprintAsset) const
{
    UBlueprint* Blueprint = Cast<UBlueprint>(BlueprintAsset.GetAsset());
    if (!Blueprint) return EPrintMode::ScreenAndLog;

    TArray<UEdGraph*> AllGraphs;
    Blueprint->GetAllGraphs(AllGraphs);

    TArray<EPrintMode> FoundModes;
    UEnum* OutputEnum = StaticEnum<EPrintMode>();

    for (UEdGraph* Graph : AllGraphs)
    {
        for (UEdGraphNode* Node : Graph->Nodes)
        {
            if (UALS_Node* PNode = Cast<UALS_Node>(Node))
            {
                UEdGraphPin* OutputPin = PNode->FindPinById(PNode->PrintModePinId);
                if (!OutputPin) continue;

                const int64 EnumValue = OutputEnum->GetValueByNameString(PNode->GetSavedDefaultValue(OutputPin));
                if (EnumValue != INDEX_NONE)
                {
                    FoundModes.Add(static_cast<EPrintMode>(EnumValue));
                }
            }
        }
    }

    if (FoundModes.IsEmpty()) return EPrintMode::ScreenAndLog;

    // Check for consistency
    const EPrintMode FirstMode = FoundModes[0];
    for (EPrintMode Mode : FoundModes)
    {
        if (Mode != FirstMode)
        {
            return (EPrintMode)0xFF;
        }
    }
    return FirstMode;
}

ELogSeverity FALSEditorModule::GetBlueprintALSLogSeverity(const FAssetData& BlueprintAsset) const
{
    UBlueprint* Blueprint = Cast<UBlueprint>(BlueprintAsset.GetAsset());
    if (!Blueprint) return ELogSeverity::Info;

    TArray<UEdGraph*> AllGraphs;
    Blueprint->GetAllGraphs(AllGraphs);

    TArray<ELogSeverity> FoundModes;
    UEnum* OutputEnum = StaticEnum<ELogSeverity>();

    for (UEdGraph* Graph : AllGraphs)
    {
        for (UEdGraphNode* Node : Graph->Nodes)
        {
            if (UALS_Node* PNode = Cast<UALS_Node>(Node))
            {
                UEdGraphPin* OutputPin = PNode->FindPinById(PNode->LogSeverityPinId);
                if (!OutputPin) continue;

                const int64 EnumValue = OutputEnum->GetValueByNameString(PNode->GetSavedDefaultValue(OutputPin));
                if (EnumValue != INDEX_NONE)
                {
                    FoundModes.Add(static_cast<ELogSeverity>(EnumValue));
                }
            }
        }
    }

    if (FoundModes.IsEmpty()) return ELogSeverity::Info;

    // Check for consistency
    const ELogSeverity FirstMode = FoundModes[0];
    for (ELogSeverity Mode : FoundModes)
    {
        if (Mode != FirstMode)
        {
            return (ELogSeverity)0xFF;
        }
    }
    return FirstMode;
}

ECheckBoxState FALSEditorModule::GetBlueprintALSCheckedState(const FAssetData& BlueprintAsset) const
{
    UBlueprint* Blueprint = Cast<UBlueprint>(BlueprintAsset.GetAsset());
    if (!Blueprint) return ECheckBoxState::Undetermined;

    TArray<UALS_Node*> Nodes;
    TArray<UEdGraph*> AllGraphs;
    Blueprint->GetAllGraphs(AllGraphs);

    for (UEdGraph* Graph : AllGraphs)
    {
        for (UEdGraphNode* Node : Graph->Nodes)
        {
            if (UALS_Node* CustomNode = Cast<UALS_Node>(Node))
            {
                Nodes.Add(CustomNode);
            }
        }
    }

    if (Nodes.Num() == 0)
        return ECheckBoxState::Undetermined;

    bool bAllEnabled = true;
    bool bAllDisabled = true;
    for (UALS_Node* Node : Nodes)
    {
        if (Node->bToggleNode) bAllDisabled = false;
        else bAllEnabled = false;
    }

    if (bAllEnabled)   return ECheckBoxState::Checked;
    if (bAllDisabled)  return ECheckBoxState::Unchecked;

    return ECheckBoxState::Undetermined;
}

void FALSEditorModule::ApplyPendingChangesForALS(const FAssetData& BlueprintAsset)
{
    UBlueprint* Blueprint = Cast<UBlueprint>(BlueprintAsset.GetAsset());
    if (!Blueprint) return;

    bool bHasCheckChange = PendingCheckedStateALS.Contains(BlueprintAsset);
    bool bHasOutputModeChange = PendingOutputModesALS.Contains(BlueprintAsset);
    bool bHasLogSeverityChange = PendingLogSeverityALS.Contains(BlueprintAsset);

    if (!bHasCheckChange && !bHasOutputModeChange && !bHasLogSeverityChange)
    {
        return;
    }

    ECheckBoxState DesiredState = ECheckBoxState::Undetermined;
    if (bHasCheckChange)
    {
        DesiredState = PendingCheckedStateALS[BlueprintAsset];
    }

    EPrintMode DesiredOutput = EPrintMode::ScreenAndLog;
    if (bHasOutputModeChange)
    {
        DesiredOutput = PendingOutputModesALS[BlueprintAsset];
    }

    ELogSeverity DesiredLogSeverity = ELogSeverity::Info;
    if (bHasLogSeverityChange)
    {
        DesiredLogSeverity = PendingLogSeverityALS[BlueprintAsset];
    }

    const FScopedTransaction Transaction(NSLOCTEXT("ALS", "UndoText", "ALS PrintString Modifications"));

    TArray<UEdGraph*> AllGraphs;
    Blueprint->GetAllGraphs(AllGraphs);

    for (UEdGraph* Graph : AllGraphs)
    {
        Graph->Modify();

        for (UEdGraphNode* Node : Graph->Nodes)
        {
            if (UALS_Node* CustomNode = Cast<UALS_Node>(Node))
            {
                CustomNode->Modify();

                // If we changed the checkbox
                if (bHasCheckChange)
                {
                    CustomNode->bToggleNode = (DesiredState == ECheckBoxState::Checked);
                }

                // If we changed the output mode
                if (bHasOutputModeChange)
                {
                    UEdGraphPin* OutputPin = CustomNode->FindPinById(CustomNode->PrintModePinId);
                    UEnum* OutputEnum = StaticEnum<EPrintMode>();
                    FString NewValue = OutputEnum->GetNameStringByValue((int64)DesiredOutput);

                    OutputPin->DefaultValue = NewValue;
                    CustomNode->PrintMode = DesiredOutput;
                    CustomNode->SetDefaultValue(OutputPin);
                }

                // If we changed Log level
                if (bHasLogSeverityChange)
                {
                    UEdGraphPin* LevelPin = CustomNode->FindPinById(CustomNode->LogSeverityPinId);
                    UEnum* LevelEnum = StaticEnum<ELogSeverity>();
                    FString NewValue = LevelEnum->GetNameStringByValue((int64)DesiredLogSeverity);

                    LevelPin->DefaultValue = NewValue;
                    CustomNode->LogSeverity = DesiredLogSeverity;
                    CustomNode->SetDefaultValue(LevelPin);
                }
            }
        }

        Graph->NotifyGraphChanged();
    }

    // Mark blueprint dirty, compile
    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

    // Remove from maps if needed
    if (bHasCheckChange)  PendingCheckedStateALS.Remove(BlueprintAsset);
    if (bHasOutputModeChange)  PendingOutputModesALS.Remove(BlueprintAsset);
    if (bHasLogSeverityChange)  PendingLogSeverityALS.Remove(BlueprintAsset);
}

void FALSEditorModule::DeleteALSNode(const FAssetData& BlueprintAsset)
{
    UBlueprint* Blueprint = Cast<UBlueprint>(BlueprintAsset.GetAsset());
    if (!Blueprint) return;

    const FScopedTransaction Transaction(NSLOCTEXT("ALS", "UndoText", "Deleted Nodes"));
    Blueprint->Modify();

    TArray<UEdGraph*> AllGraphs;
    Blueprint->GetAllGraphs(AllGraphs);

    TArray<UALS_Node*> NodesToDelete;

    for (UEdGraph* Graph : AllGraphs)
    {
        Graph->Modify();

        for (UEdGraphNode* Node : Graph->Nodes)
        {
            if (UALS_Node* CustomNode = Cast<UALS_Node>(Node))
            {
                NodesToDelete.Add(CustomNode);
            }
        }

        for (UALS_Node* NodeToDelete : NodesToDelete)
        {
            TArray<UEdGraphPin*> ExecInputPins = NodeToDelete->GetExecPins();
            TArray<UEdGraphPin*> ExecOutputPins = NodeToDelete->GetThenPins();

            for (int32 i = 0; i < ExecInputPins.Num(); i++)
            {
                if (ExecInputPins[i]->HasAnyConnections() && ExecOutputPins[i]->HasAnyConnections())
                {
                    for (UEdGraphPin* ExecConnection : ExecInputPins[i]->LinkedTo)
                    {
                        ExecConnection->MakeLinkTo(ExecOutputPins[i]->LinkedTo[0]);
                    }
                }
            }

            Graph->RemoveNode(NodeToDelete, true);
        }

        Graph->NotifyGraphChanged();
    }
    
    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
    RefreshALSContainer();
}

void FALSEditorModule::RefreshALSContainer()
{
    ALSScrollContainer->ClearChildren();

    TArray<FAssetData> ALSBlueprints;
    GetBlueprintsWithALSNodes(ALSBlueprints);

    for (const FAssetData& Blueprint : ALSBlueprints)
    {
        ALSScrollContainer->AddSlot()
            [
                CreateALSRow(Blueprint, ALSBlueprints)
            ];
    }

    return;
}


// Regular Manager
void FALSEditorModule::ShowBlueprintWindowForUPS()
{
    if (ExistingUPSWindow.IsValid())
    {
        ExistingUPSWindow->RequestDestroyWindow();
        ExistingUPSWindow.Reset();
    }

    if (ExistingALSWindow.IsValid())
    {
        ExistingALSWindow->RequestDestroyWindow();
        ExistingALSWindow.Reset();
    }

    TArray<FAssetData> RelevantBlueprints;
    GetBlueprintsWithUPSNodes(RelevantBlueprints);

    if (RelevantBlueprints.IsEmpty())
    {
        FMessageDialog::Open(EAppMsgType::Ok, FText::FromString
        ("No PrintString nodes detected across Project Blueprints\nAdd one to a Blueprint and reopen this window to manage Print String nodes."));

        return;
    }

    ExistingUPSWindow = SNew(SWindow)
        .Title(FText::FromString("Manage PrintString (Regular) Nodes"))
        .ClientSize(FVector2D(550, FMath::Clamp(RelevantBlueprints.Num() * 45 + 150, 300, 600)))
        .SizingRule(ESizingRule::UserSized);


    // Heading row (BP Name | Is Enabled | Replace | Delete)
    TSharedRef<SVerticalBox> MainBox = SNew(SVerticalBox);
    MainBox->AddSlot().AutoHeight().Padding(5.f)
        [
            SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1.0f).Padding(10)
                [
                    SNew(STextBlock).Text(FText::FromString("BP NAME")).Font(FAppStyle::GetFontStyle("BoldFont")).Justification(ETextJustify::Left)
                ]
                + SHorizontalBox::Slot().FillWidth(0.3f).Padding(10)
                [
                    SNew(STextBlock).Text(FText::FromString("ON/OFF")).Font(FAppStyle::GetFontStyle("BoldFont")).Justification(ETextJustify::Center)
                ]
                + SHorizontalBox::Slot().FillWidth(0.5f).Padding(10)
                [
                    SNew(STextBlock).Text(FText::FromString("REPLACE")).Font(FAppStyle::GetFontStyle("BoldFont")).Justification(ETextJustify::Center)
                ]
                + SHorizontalBox::Slot().FillWidth(0.5f).Padding(10)
                [
                    SNew(STextBlock).Text(FText::FromString("DELETE")).Font(FAppStyle::GetFontStyle("BoldFont")).Justification(ETextJustify::Center)
                ]
        ];


    UPSScrollContainer = SNew(SScrollBox);
    for (const FAssetData& Blueprint : RelevantBlueprints)
    {
        UPSScrollContainer->AddSlot()
            [
                CreateUPSRow(Blueprint, RelevantBlueprints)
            ];
    }

    MainBox->AddSlot().FillHeight(1.f).Padding(5)[UPSScrollContainer.ToSharedRef()];

    ExistingUPSWindow->SetOnWindowClosed(FOnWindowClosed::CreateLambda([this](const TSharedRef<SWindow>& Window)
        {
            PendingCheckedStateUPS.Empty();
        }));

    ExistingUPSWindow->SetContent(MainBox);
    FSlateApplication::Get().AddWindow(ExistingUPSWindow.ToSharedRef());
}

TSharedRef<SWidget> FALSEditorModule::CreateUPSRow(const FAssetData& Blueprint, TArray<FAssetData>& RelevantBlueprints)
{
    TSharedRef<SWidget> RowWidget = SNew(SHorizontalBox)

        // BP NAME
        + SHorizontalBox::Slot().FillWidth(1.0f).Padding(10, 2).HAlign(HAlign_Left)
        [
            SNew(STextBlock).Text(FText::FromName(Blueprint.AssetName))
        ]

        // IS ENABLED
        + SHorizontalBox::Slot().FillWidth(0.3f).Padding(10, 2).HAlign(HAlign_Center)
        [
            SNew(SCheckBox)
                .IsChecked(GetBlueprintUPSCheckedState(Blueprint))
                .OnCheckStateChanged_Lambda([this, Blueprint](ECheckBoxState NewState)
                    {
                        PendingCheckedStateUPS.Add(Blueprint, NewState);
                        ApplyPendingChangesForUPS(Blueprint);
                    })
                .ToolTipText(LOCTEXT("IsEnabled_Tooltip", "Toggle the node’s active state. Unchecked nodes will skip printing."))
        ]

        // REPLACE
        + SHorizontalBox::Slot().FillWidth(0.5f).Padding(10, 2)
        [
            SNew(SButton)
                .Text(FText::FromString("Replace"))
                .OnClicked_Lambda([this, Blueprint, &RelevantBlueprints]()
                    {
                        EAppReturnType::Type Result = FMessageDialog::Open(EAppMsgType::OkCancel,
                        FText::FromString("Are you sure you want to replace all Print String nodes in " + Blueprint.AssetName.ToString() + " with ALS?\n\nNOTE: Make sure to compile and save after verifying. You can also revert this action by Undo"));

                        if (Result == EAppReturnType::Ok)
                        {
                            ReplaceUPSWithALS(Blueprint);
                        }

                        return FReply::Handled();
                    })
                .ToolTipText(LOCTEXT("Replace_Tooltip", "Convert this PrintString node into a ALS PrintString node with preserved data and its exec chain."))
        ]

        // DELETE
        + SHorizontalBox::Slot().FillWidth(0.5f).Padding(10, 2)
        [
            SNew(SButton)
                .Text(FText::FromString("Delete"))
                .OnClicked_Lambda([this, Blueprint, &RelevantBlueprints]()
                    {
                        EAppReturnType::Type Result = FMessageDialog::Open(EAppMsgType::OkCancel,
                        FText::FromString("Are you sure you want to delete all Print String nodes in " + Blueprint.AssetName.ToString() + "?\n\nNOTE: Make sure to compile and save after verifying. You can also revert this action by Undo"));

                        if (Result == EAppReturnType::Ok)
                        {
                            DeleteUPSNode(Blueprint);
                        }

                        return FReply::Handled();
                    })
                .ToolTipText(LOCTEXT("Delete_Tooltip", "Delete the Print String's found in this BP while preserving its exec chains."))
        ];

    return RowWidget;
}

void FALSEditorModule::GetBlueprintsWithUPSNodes(TArray<FAssetData>& OutBlueprints) const
{
    auto BlueprintContainsUPS = [](const FAssetData& BlueprintAsset) -> bool
        {
            UBlueprint* Blueprint = Cast<UBlueprint>(BlueprintAsset.GetAsset());
            if (!Blueprint) return false;

            TArray<UEdGraph*> AllGraphs;
            Blueprint->GetAllGraphs(AllGraphs);

            for (UEdGraph* Graph : AllGraphs)
            {
                for (UEdGraphNode* Node : Graph->Nodes)
                {
                    if (UK2Node_CallFunction* CallFuncNode = Cast<UK2Node_CallFunction>(Node))
                    {
                        if (CallFuncNode->FunctionReference.GetMemberName() == "PrintString")
                        {
                            return true;
                        }
                    }
                }
            }

            return false;
        };

    OutBlueprints.Empty();

    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (World && World->PersistentLevel)
    {
        if (ULevelScriptBlueprint* LevelBP = Cast<ULevelScriptBlueprint>(World->PersistentLevel->GetLevelScriptBlueprint()))
        {
            FAssetData LevelAsset(LevelBP);
            if (BlueprintContainsUPS(LevelAsset))
            {
                OutBlueprints.Add(LevelAsset);
            }
        }
    }

    FARFilter Filter;
    Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
    Filter.ClassPaths.Add(UWidgetBlueprint::StaticClass()->GetClassPathName());
    Filter.ClassPaths.Add(UAnimBlueprint::StaticClass()->GetClassPathName());
    Filter.ClassPaths.Add(UEditorUtilityBlueprint::StaticClass()->GetClassPathName());
    Filter.bRecursivePaths = true;

    Filter.PackagePaths.Add("/Game");

    TArray<FAssetData> FilteredAssets;
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    AssetRegistryModule.Get().GetAssets(Filter, FilteredAssets);

    for (FAssetData& Asset : FilteredAssets)
    {
        if (BlueprintContainsUPS(Asset))
        {
            OutBlueprints.Add(Asset);
        }
    }
}

ECheckBoxState FALSEditorModule::GetBlueprintUPSCheckedState(const FAssetData& BlueprintAsset) const
{
    UBlueprint* Blueprint = Cast<UBlueprint>(BlueprintAsset.GetAsset());
    if (!Blueprint) return ECheckBoxState::Unchecked;

    TArray<UEdGraph*> AllGraphs;
    Blueprint->GetAllGraphs(AllGraphs);

    bool bHasScreenEnabled = false;
    bool bHasLogEnabled = false;
    bool bHasScreenDisabled = false;
    bool bHasLogDisabled = false;

    for (UEdGraph* Graph : AllGraphs)
    {
        for (UEdGraphNode* Node : Graph->Nodes)
        {
            if (UK2Node_CallFunction* CallFuncNode = Cast<UK2Node_CallFunction>(Node))
            {
                if (CallFuncNode->FunctionReference.GetMemberName() == "PrintString")
                {
                    UEdGraphPin* ScreenPin = CallFuncNode->FindPinChecked(TEXT("bPrintToScreen"));
                    UEdGraphPin* LogPin = CallFuncNode->FindPinChecked(TEXT("bPrintToLog"));

                    if (ScreenPin && LogPin)
                    {
                        bool bScreen = (ScreenPin->DefaultValue == TEXT("true"));
                        bool bLog = (LogPin->DefaultValue == TEXT("true"));

                        if (bScreen)
                            bHasScreenEnabled = true;
                        else
                            bHasScreenDisabled = true;

                        if (bLog)
                            bHasLogEnabled = true;
                        else
                            bHasLogDisabled = true;
                    }
                }
            }
        }
    }

    // **Determine Check State**
    if (bHasScreenEnabled && bHasLogEnabled && !bHasScreenDisabled && !bHasLogDisabled)
    {
        return ECheckBoxState::Checked;
    }
    if (bHasScreenDisabled && bHasLogDisabled && !bHasScreenEnabled && !bHasLogEnabled)
    {
        return ECheckBoxState::Unchecked;
    }

    return ECheckBoxState::Undetermined;
}

void FALSEditorModule::ApplyPendingChangesForUPS(const FAssetData& BlueprintAsset)
{
    UBlueprint* Blueprint = Cast<UBlueprint>(BlueprintAsset.GetAsset());
    if (!Blueprint) return;

    ECheckBoxState DesiredState = PendingCheckedStateUPS[BlueprintAsset];

    const FScopedTransaction Transaction(NSLOCTEXT("ALS", "UndoText", "Regular PrintString Modifications"));

    TArray<UEdGraph*> AllGraphs;
    Blueprint->GetAllGraphs(AllGraphs);

    for (UEdGraph* Graph : AllGraphs)
    {
        Graph->Modify();

        for (UEdGraphNode* Node : Graph->Nodes)
        {
            if (UK2Node_CallFunction* CallFuncNode = Cast<UK2Node_CallFunction>(Node))
            {
                if (CallFuncNode->FunctionReference.GetMemberName() == "PrintString")
                {
                    CallFuncNode->Modify();

                    UEdGraphPin* ScreenPin = CallFuncNode->FindPinChecked(TEXT("bPrintToScreen"));
                    UEdGraphPin* LogPin = CallFuncNode->FindPinChecked(TEXT("bPrintToLog"));

                    if (ScreenPin && LogPin)
                    {
                        ScreenPin->DefaultValue = (DesiredState == ECheckBoxState::Checked) ? TEXT("true") : TEXT("false");
                        LogPin->DefaultValue = (DesiredState == ECheckBoxState::Checked) ? TEXT("true") : TEXT("false");
                    }
                }
            }
        }

        Graph->NotifyGraphChanged();
    }

    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
    PendingCheckedStateUPS.Remove(BlueprintAsset);
}

void FALSEditorModule::DeleteUPSNode(const FAssetData& BlueprintAsset)
{
    UBlueprint* Blueprint = Cast<UBlueprint>(BlueprintAsset.GetAsset());
    if (!Blueprint) return;

    const FScopedTransaction Transaction(NSLOCTEXT("ALS", "UndoText", "Deleted Nodes"));
    Blueprint->Modify();
    
    TArray<UEdGraph*> AllGraphs;
    Blueprint->GetAllGraphs(AllGraphs);

    TArray<UK2Node_CallFunction*> NodesToDelete;

    for (UEdGraph* Graph : AllGraphs)
    {
        Graph->Modify();

        for (UEdGraphNode* Node : Graph->Nodes)
        {
            if (UK2Node_CallFunction* CallFuncNode = Cast<UK2Node_CallFunction>(Node))
            {
                if (CallFuncNode->FunctionReference.GetMemberName() == "PrintString")
                {
                    NodesToDelete.Add(CallFuncNode);
                }
            }
        }

        for (UK2Node_CallFunction* NodeToDelete : NodesToDelete)
        {
            if (NodeToDelete->GetExecPin()->HasAnyConnections() && NodeToDelete->GetThenPin()->HasAnyConnections())
            {
                for (UEdGraphPin* ExecConnection : NodeToDelete->GetExecPin()->LinkedTo)
                {
                    ExecConnection->MakeLinkTo(NodeToDelete->GetThenPin()->LinkedTo[0]);   
                }
            }

            Graph->RemoveNode(NodeToDelete, true);
        }

        Graph->NotifyGraphChanged();
    }

    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
    RefreshUPSContainer();
}

void FALSEditorModule::ReplaceUPSWithALS(const FAssetData& BlueprintAsset)
{
    if (!BlueprintAsset.GetAsset()) return;

    UBlueprint* Blueprint = Cast<UBlueprint>(BlueprintAsset.GetAsset());
    if (!Blueprint) return;

    const FScopedTransaction Transaction(NSLOCTEXT("ALS", "UndoText", "Replaced Nodes"));
    Blueprint->Modify();

    TArray<UEdGraph*> AllGraphs;
    Blueprint->GetAllGraphs(AllGraphs);

    auto ReplaceDataPins = [&](UALS_Node* NewNode, UEdGraphPin* NewPin, UEdGraphPin* OldPin)
        {
            if (OldPin && NewPin)
            {
                if (OldPin->HasAnyConnections())
                {
                    for (UEdGraphPin* OldConnections : OldPin->LinkedTo)
                    {
                        NewPin->PinType = OldPin->PinType;
                        NewPin->MakeLinkTo(OldConnections);
                    }
                }
                else
                {
                    NewPin->DefaultValue = OldPin->DefaultValue;
                    NewNode->SetDefaultValue(NewPin);
                }
            }
        };

    for (UEdGraph* Graph : AllGraphs)
    {
        TArray<UK2Node_CallFunction*> NodesToReplace;
        Graph->Modify();

        for (UEdGraphNode* Node : Graph->Nodes)
        {
            if (UK2Node_CallFunction* CallFuncNode = Cast<UK2Node_CallFunction>(Node)) //UKismetSystemLibrary::PrintString();
            {
                if (CallFuncNode->FunctionReference.GetMemberName() == "PrintString")
                {
                    NodesToReplace.Add(CallFuncNode);
                }
            }
        }

        for (UK2Node_CallFunction* OldNode : NodesToReplace)
        {
            // **Create ALS Node**
            UALS_Node* NewNode = NewObject<UALS_Node>(Graph);
            Graph->AddNode(NewNode, false, false);

            NewNode->Modify();

            NewNode->AllocateDefaultPins();
            NewNode->CreateNewGuid();

            NewNode->NodePosX = OldNode->NodePosX;
            NewNode->NodePosY = OldNode->NodePosY;

            // Exec Pin Replacements
            UEdGraphPin* OldExecPin = OldNode->GetExecPin();
            UEdGraphPin* NewExecPin = NewNode->FindPinById(NewNode->ExecPinId);

            UEdGraphPin* OldThenPin = OldNode->GetThenPin();
            UEdGraphPin* NewThenPin = NewNode->GetCorrespondingThenPin(NewExecPin);

            if (OldExecPin && NewExecPin)
            {
                if (OldExecPin->HasAnyConnections())
                {
                    for (UEdGraphPin* OldConnections : OldExecPin->LinkedTo)
                    {
                        NewExecPin->MakeLinkTo(OldConnections);
                    }
                }
            }

            if (OldThenPin && NewThenPin)
            {
                if (OldThenPin->HasAnyConnections())
                {
                    NewThenPin->MakeLinkTo(OldThenPin->LinkedTo[0]);
                }
            }

            // Data Pin Replacement
            UEdGraphPin* NewValuePin = NewNode->FindPinById(NewNode->WildcardPinId);
            UEdGraphPin* OldValuePin = OldNode->FindPinChecked(TEXT("InString"));
            ReplaceDataPins(NewNode, NewValuePin, OldValuePin);


            //WorldContext Pin Replacement
            UEdGraphPin* NewContextPin = NewNode->FindPinById(NewNode->WorldContextId);
            UEdGraphPin* OldContextPin = OldNode->FindPinChecked(TEXT("WorldContextObject"));
            ReplaceDataPins(NewNode, NewContextPin, OldContextPin);


            //  Color Pin Replacement
            UEdGraphPin* NewColorPin = NewNode->FindPinById(NewNode->ColorPinId);
            UEdGraphPin* OldColorPin = OldNode->FindPinChecked(TEXT("TextColor"));
            if (NewColorPin && OldColorPin)
            {
                FLinearColor ColorValue;
                ColorValue.InitFromString(OldColorPin->DefaultValue);
                NewNode->PrintColor = ColorValue;
            }
            ReplaceDataPins(NewNode, NewColorPin, OldColorPin);


            // Duration Pin Replacement
            UEdGraphPin* NewDurationPin = NewNode->FindPinById(NewNode->DurationPinId);
            UEdGraphPin* OldDurationPin = OldNode->FindPinChecked(TEXT("Duration"));
            if (NewDurationPin && OldDurationPin)
            {
                float DurationValue = FCString::Atof(*OldDurationPin->DefaultValue);
                NewNode->PrintDuration = DurationValue;
            }
            ReplaceDataPins(NewNode, NewDurationPin, OldDurationPin);


            // PrintMode Pin Replacement
            UEdGraphPin* NewPrintModePin = NewNode->FindPinById(NewNode->PrintModePinId);
            UEdGraphPin* OldScreenPin = OldNode->FindPinChecked(TEXT("bPrintToScreen"));
            UEdGraphPin* OldLogPin = OldNode->FindPinChecked(TEXT("bPrintToLog"));
            if (NewPrintModePin && OldScreenPin && OldLogPin)
            {
                bool bScreen = (OldScreenPin->DefaultValue == TEXT("true"));
                bool bLog = (OldLogPin->DefaultValue == TEXT("true"));

                EPrintMode NewPrintMode;

                if (bScreen && bLog) NewPrintMode = EPrintMode::ScreenAndLog;
                else if (bScreen && !bLog) NewPrintMode = EPrintMode::ScreenOnly;
                else NewPrintMode = EPrintMode::LogOnly;

                NewNode->PrintMode = NewPrintMode;
            }

            Graph->RemoveNode(OldNode);
        }

        Graph->NotifyGraphChanged();
    }

    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
    RefreshUPSContainer();
}

void FALSEditorModule::RefreshUPSContainer()
{
    UPSScrollContainer->ClearChildren();

    TArray<FAssetData> UPSBlueprints;
    GetBlueprintsWithUPSNodes(UPSBlueprints);

    for (const FAssetData& Blueprint : UPSBlueprints)
    {
        UPSScrollContainer->AddSlot()
            [
                CreateUPSRow(Blueprint, UPSBlueprints)
            ];
    }

    return;
}


//Post Undo - Redo Validation
void FALSEditorModule::PostUndo(bool bSuccess)
{
    if (ALSScrollContainer.IsValid())
    {
        RefreshALSContainer();
    }

    if (UPSScrollContainer.IsValid())
    {
        RefreshUPSContainer();
    }
}

void FALSEditorModule::PostRedo(bool bSuccess)
{
    if (ALSScrollContainer.IsValid())
    {
        RefreshALSContainer();
    }

    if (UPSScrollContainer.IsValid())
    {
        RefreshUPSContainer();
    }
}


// SHOW LOGS
void FALSEditorModule::ShowAllALSLogs()
{
    UClass* WidgetClass = UALS_Settings::Get()->LogViewerWidget.LoadSynchronous();
    if (!WidgetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load ALS_Logs!"));
        return;
    }

    UUserWidget* LogWidget = CreateWidget<UUserWidget>(GEditor->GetEditorWorldContext().World(), WidgetClass);
    if (!LogWidget)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create ALS_Logs Widget."));
        return;
    }

    TSharedRef<SWindow> LogWindow = SNew(SWindow)
        .Title(FText::FromString("ALS Log Viewer"))
        .ClientSize(FVector2D(1100, 700))
        .SupportsMaximize(false)
        .SupportsMinimize(false)
        .SizingRule(ESizingRule::UserSized);

    LogWindow->SetContent(LogWidget->TakeWidget());
    FSlateApplication::Get().AddWindow(LogWindow);
}


#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FALSEditorModule, ALS_Editor)