//Copyright © 2025 RTerofer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Engine/DeveloperSettings.h"
#include "Framework/Commands/InputChord.h"
#include "Misc/Paths.h"     
#include "Blueprint/UserWidget.h"
#include "ALS_Definitions.h"
#include "ALS_Settings.generated.h"

UCLASS(Config = Game, defaultconfig, meta = (DisplayName = "AdvancedLoggingSystem"))
class ALS_API UALS_Settings : public UDeveloperSettings
{
    GENERATED_BODY()

protected:
    virtual FName GetCategoryName() const override;

public:
    static const UALS_Settings* Get();
    static FPrintConfig GetConfigFromPreset(EPrintPreset Preset);
    static bool IsFileLoggingAllowed();
    static bool IsPropertyInspectorAllowed();

    // If true, includes the caller's context name (e.g. [MyActor]) before the print message
    UPROPERTY(Config, EditDefaultsOnly, Category = "GENERAL SETTINGS", meta = (DisplayName = "Show Context Name"))
    bool bShowCallerName = false;

    // When enabled, File logging stays active in Shipping builds and both LogsViewer can be opened at runtime. NOTE: This needs "Enable File Logging" to be turned on
    UPROPERTY(Config, EditDefaultsOnly, Category = "GENERAL SETTINGS", meta = (DisplayName = "Enable FileLogging In Shipping"))
    bool bEnableFileLogInShipping = false;

    // When enabled, PropertyInspector can be opened at runtime.
    UPROPERTY(Config, EditDefaultsOnly, Category = "GENERAL SETTINGS", meta = (DisplayName = "Enable PropertyInspector In Shipping"))
    bool bEnableInspectorInShipping = false;

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

    /** What chord opens/closes the Logs Viewer */
    UPROPERTY(config, EditDefaultsOnly, Category = "INPUT SETTINGS", meta = (DisplayName = "Open Logs Viewer"))
    FInputChord LogToggleChord = FInputChord(EKeys::L, true, true, false,false);

    /** What chord opens/closes the Property Inspector */
    UPROPERTY(config, EditDefaultsOnly, Category = "INPUT SETTINGS", meta = (DisplayName = "Open Property Inspector"))
    FInputChord PropertyToggleChord = FInputChord(EKeys::P, true, true, false, false);

    
    /*Disables the global shortcut bindings for opening the Logs Viewer and Property Inspector.
      Recommended to enable if ALS shortcut keys interfere with your own input setup.*/
    UPROPERTY(config, EditDefaultsOnly, Category = "INPUT SETTINGS", meta = (DisplayName = "Disable ALS Shortcut Keys"))
    bool bDisableALSBindings = false;

    /*Override Mouse + Input Mode
      When true (default), ALS handles cursor visibility and input mode:
        Widget Open  -  Show cursor, SetInputModeGameAndUI
        Widget Close -  Hide cursor, SetInputModeGameOnly
      Disable if your project manages its own mouse/input flow (e.g., non-game apps or custom viewport frameworks).
      In that case, call Shift-F1 or your own handler manually */
    UPROPERTY(config, EditDefaultsOnly, Category = "INPUT SETTINGS", meta = (DisplayName = "Override Mouse & Input Mode"))
    bool bOverrideInputMode = true;

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

    // Enables or disables writing to file logs
    UPROPERTY(Config, EditDefaultsOnly, Category = "LOG VIEWER", meta = (DisplayName = "Enable File Logging"))
    bool bEnableFileLog = true;

    // When enabled, Print Service or Task will log only if the current message differs from the previous message.
    UPROPERTY(Config, EditDefaultsOnly, Category = "LOG VIEWER", meta = (DisplayName = "File Only Unique Messages (BT Task & Service)"))
    bool LogUniqueMsgsForBT = true;

    // When enabled, Property Insector will log only if the current message differs from the previous message.
    UPROPERTY(Config, EditDefaultsOnly, Category = "LOG VIEWER", meta = (DisplayName = "File Only Unique Messages (Property Inspector)"))
    bool LogUniqueMsgsForPD = true;

    // By Default, the SessionID is created for each play regardless of logged or not. This makes it easier to differentiate multiplayer logs, since all the instance will have same SessionID
    // But if you want the SessionID to be created only when logged, then enabling this would be better.
    UPROPERTY(Config, EditDefaultsOnly, Category = "LOG VIEWER", meta = (DisplayName = "Create Session Only If Logged"))
    bool bCreateSessionOnlyIfLogged = false;

    // Location where all ALS log files will be saved
    UPROPERTY(Config, EditDefaultsOnly, Category = "LOG VIEWER", meta = (DisplayName = "File Log Folder"))
    FDirectoryPath FileLogRootDir = FDirectoryPath{ FPaths::ProjectSavedDir() + TEXT("Logs/ALS") };

    // Max lists to create per context
    UPROPERTY(Config, EditDefaultsOnly, Category = "LOG VIEWER",
        meta = (DisplayName = "Max Lists to Create", ClampMin = "5", ClamALSx = "100000", UIMin = "5", UIMax = "10000"))
        int32 MaxNumberOfListsToCreate = 2000;

    // Alerts if log file size (in MB) to parse is larger than the threshold size. Increasing this will suppress the file size warnings.
    UPROPERTY(Config, EditDefaultsOnly, Category = "LOG VIEWER",
        meta = (DisplayName = "Log Size Warning Threshold (MB)", ClampMin = "1", ClamALSx = "1024"))
        int64 MaxFileSizeToParse = 10;

    // Auto-delete logs older than N days
    UPROPERTY(Config, EditDefaultsOnly, Category = "LOG VIEWER",
        meta = (DisplayName = "Archive Logs Older Than (Days)", ClampMin = "1", ClamALSx = "365"))
        int32 MaxFileLogAge = 3;

    // Enabling this will list all the archived logs from the ArchivedLogs directory.
    UPROPERTY(Config, EditDefaultsOnly, Category = "LOG VIEWER", meta = (DisplayName = "Access Archived Logs in Viewer"))
    bool bIncludeArchivedLogsInViewer = false;

    // Widget to spawn for LogsViewer, If you have duplicated or created your own widget, then changing this is necessary
    UPROPERTY(Config, EditDefaultsOnly, Category = "LOG VIEWER", meta = (DisplayName = "Log Viewer Widget", AllowedClasses = "/Script/UMG.UserWidget"))
    TSoftClassPtr<UUserWidget> LogViewerWidget = TSoftClassPtr<UUserWidget>(FSoftClassPath(
        TEXT("/AdvancedLoggingSystem/UMG/LogsViewer/ALS_LogsViewer.ALS_LogsViewer_C")));

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

    // How often the property inspector updates tracked properties (in seconds). 0 means tick
    UPROPERTY(Config, EditDefaultsOnly, Category = "PROPERTY INSPECTOR", 
        meta = (DisplayName = "Update Interval (Seconds)", ClampMin = "0.0", ClamALSx = "10.0"))
    float RefreshTimer = 0.1f;

    // When disabled, the object's internal name (e.g., "Object_C_1") is used instead of the Actor's label name from the Outliner.
    UPROPERTY(Config, EditDefaultsOnly, Category = "PROPERTY INSPECTOR", meta = (DisplayName = "Use Actor Label"))
    bool UseActorLabel = true;

    // The context color of runtime property inspector messages
    UPROPERTY(Config, EditDefaultsOnly, Category = "PROPERTY INSPECTOR", meta = (DisplayName = "Context Color"))
    FSlateColor ContextColor = FLinearColor(1, 1, 1, 0.5);

    // The property name color of runtime property inspector messages
    UPROPERTY(Config, EditDefaultsOnly, Category = "PROPERTY INSPECTOR", meta = (DisplayName = "Property Name Color"))
    FSlateColor PropertyColor = FLinearColor(0.5, 0.4, 1, 1);

    // The message color of runtime property inspector messages
    UPROPERTY(Config, EditDefaultsOnly, Category = "PROPERTY INSPECTOR", meta = (DisplayName = "Message Color"))
    FSlateColor MessageColor = FLinearColor(1, 1, 0, 1);

    // Widget to spawn for PropertyDebugger, If you have duplicated or created your own widget, then changing this is necessary
    UPROPERTY(Config, EditDefaultsOnly, Category = "PROPERTY INSPECTOR", meta = (DisplayName = "Property Inspector Widget", AllowedClasses = "/Script/UMG.UserWidget"))
    TSoftClassPtr<UUserWidget> PropertyInspectorWidget = TSoftClassPtr<UUserWidget>(FSoftClassPath(
        TEXT("/AdvancedLoggingSystem/UMG/PropertyInspector/ALS_PropertyInspector.ALS_PropertyInspector_C")));

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

    // Preset used for PrintInfo
    UPROPERTY(Config, EditDefaultsOnly, Category = "PRESETS", meta = (DisplayName = "Print Info"))
    FPrintConfig PrintInfoConfig = FPrintConfig(NAME_None, 3.0f, FColor::Green, ELogSeverity::Info, EPrintMode::ScreenAndLog);

    // Preset used for PrintWarn
    UPROPERTY(Config, EditDefaultsOnly, Category = "PRESETS", meta = (DisplayName = "Print Warn"))
    FPrintConfig PrintWarnConfig = FPrintConfig(NAME_None, 5.0f, FColor::Yellow, ELogSeverity::Warning, EPrintMode::ScreenAndLog);

    // Preset used for PrintError
    UPROPERTY(Config, EditDefaultsOnly, Category = "PRESETS", meta = (DisplayName = "Print Error"))
    FPrintConfig PrintErrorConfig = FPrintConfig(NAME_None, 7.0f, FColor::Red, ELogSeverity::Error, EPrintMode::ScreenAndLog);

    // Preset used for LogInfo
    UPROPERTY(Config, EditDefaultsOnly, Category = "PRESETS", meta = (DisplayName = "Log Info"))
    FPrintConfig LogInfoConfig = FPrintConfig(NAME_None, 0.0f, FColor::Green, ELogSeverity::Info, EPrintMode::LogOnly);

    // Preset used for LogWarn
    UPROPERTY(Config, EditDefaultsOnly, Category = "PRESETS", meta = (DisplayName = "Log Warn"))
    FPrintConfig LogWarnConfig = FPrintConfig(NAME_None, 0.0f, FColor::Yellow, ELogSeverity::Warning, EPrintMode::LogOnly);

    // Preset used for LogError
    UPROPERTY(Config, EditDefaultsOnly, Category = "PRESETS", meta = (DisplayName = "Log Error"))
    FPrintConfig LogErrorConfig = FPrintConfig(NAME_None, 0.0f, FColor::Red, ELogSeverity::Error, EPrintMode::LogOnly);

    // Preset used for Print3D
    UPROPERTY(Config, EditDefaultsOnly, Category = "PRESETS", meta = (DisplayName = "Print 3D"))
    FPrintConfig Print3DConfig = FPrintConfig(NAME_None, 5.0f, FColor::Orange, ELogSeverity::Info, EPrintMode::LogOnly);

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

    // Default hint or value shown in exec pin on new node
    UPROPERTY(Config, EditDefaultsOnly, Category = "NODE SPAWN SETTINGS", meta = (DisplayName = "Default Exec Pin Text"))
    FString ExecDefaultValue = "";

    // Default value or label shown in input pins on new node
    UPROPERTY(Config, EditDefaultsOnly, Category = "NODE SPAWN SETTINGS", meta = (DisplayName = "Default Input Pin Text"))
    FString PinDefaultValue = "Hello";

    // Default Preset
    UPROPERTY(Config, EditDefaultsOnly, Category = "NODE SPAWN SETTINGS", meta = (DisplayName = "Default Preset"))
    EPrintPreset PrintPreset = EPrintPreset::PrintInfo;

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

        // Node title color when log level is Info
    UPROPERTY(Config, EditDefaultsOnly, Category = "NODE COLORS", meta = (DisplayName = "Info Title Color"))
    FLinearColor NodeInfoColor = FLinearColor::Green;

    // Node title color when log level is Warning
    UPROPERTY(Config, EditDefaultsOnly, Category = "NODE COLORS", meta = (DisplayName = "Warning Title Color"))
    FLinearColor NodeWarningColor = FLinearColor::Yellow;

    // Node title color when log level is Error
    UPROPERTY(Config, EditDefaultsOnly, Category = "NODE COLORS", meta = (DisplayName = "Error Title Color"))
    FLinearColor NodeErrorColor = FLinearColor::Red;

    // Node title color when 3D debug is active
    UPROPERTY(Config, EditDefaultsOnly, Category = "NODE COLORS", meta = (DisplayName = "3D Title Color"))
    FLinearColor Node3DColor = FLinearColor(0.8, 0.25, 0.0, 1.0);

    // Title color when node is active
    UPROPERTY(Config, EditDefaultsOnly, Category = "NODE COLORS", meta = (DisplayName = "Active Title Color"))
    FLinearColor NodeActiveTitleColor = FLinearColor::Gray;

    // Body color when node is active
    UPROPERTY(Config, EditDefaultsOnly, Category = "NODE COLORS", meta = (DisplayName = "Active Body Color"))
    FLinearColor NodeActiveBodyColor = FLinearColor::Gray;

    // Body color when node is inactive
    UPROPERTY(Config, EditDefaultsOnly, Category = "NODE COLORS", meta = (DisplayName = "Inactive Body Color"))
    FLinearColor NodeInActiveBodyColor = FLinearColor::White;

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

};
