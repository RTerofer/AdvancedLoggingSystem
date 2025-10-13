//Copyright © 2025 RTerofer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ALSE_Node.h"
#include "ALS_Definitions.h"
#include "Widgets/Layout/SScrollBox.h"
#include "EditorUndoClient.h"
#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;

struct FPrintModeRowData
{
    FAssetData Blueprint;
    TArray<TSharedPtr<EPrintMode>> ComboOptions;
    TSharedPtr<EPrintMode> InitiallySelected;
};

struct FLogSeverityRowData
{
    FAssetData Blueprint;
    TArray<TSharedPtr<ELogSeverity>> ComboOptions;
    TSharedPtr<ELogSeverity> InitiallySelected;
};

class ALS_EDITOR_API FALSEditorModule : public IModuleInterface, public FEditorUndoClient
{
protected:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

private:
    //ALS Nodes Window Functions
    void ShowBlueprintWindowForALS();
    TSharedRef<SWidget> CreateALSRow(const FAssetData& Blueprint, TArray<FAssetData>& RelevantBlueprints);
    void GetBlueprintsWithALSNodes(TArray<FAssetData>& OutBlueprints) const;
    EPrintMode GetBlueprintALSOutputMode(const FAssetData& BlueprintAsset) const;
    ELogSeverity GetBlueprintALSLogSeverity(const FAssetData& BlueprintAsset) const;
    ECheckBoxState GetBlueprintALSCheckedState(const FAssetData& BlueprintAsset) const;
    void ApplyPendingChangesForALS(const FAssetData& BlueprintAsset);
    void DeleteALSNode(const FAssetData& BlueprintAsset);

    void RefreshALSContainer();

private:
    //Unreal's PrintString Nodes Window Functions
    void ShowBlueprintWindowForUPS();
    TSharedRef<SWidget> CreateUPSRow(const FAssetData& Blueprint, TArray<FAssetData>& RelevantBlueprints);
    void GetBlueprintsWithUPSNodes(TArray<FAssetData>& OutBlueprints) const;
    ECheckBoxState GetBlueprintUPSCheckedState(const FAssetData& BlueprintAsset) const;
    void ApplyPendingChangesForUPS(const FAssetData& BlueprintAsset);
    void ReplaceUPSWithALS(const FAssetData& BlueprintAsset);
    void DeleteUPSNode(const FAssetData& BlueprintAsset);

    void RefreshUPSContainer();

private:

    void ShowAllALSLogs();
    void RegisterMenus();
    TSharedRef<SWidget> GenerateToolbarMenu();

    virtual void PostUndo(bool bSuccess) override;
    virtual void PostRedo(bool bSuccess) override;

private:
    TSharedPtr<class FUICommandList> CommandsList;

    // Safe Changes Cache
    TMap<FAssetData, ECheckBoxState> PendingCheckedStateALS;
    TMap<FAssetData, ELogSeverity> PendingLogSeverityALS;
    TMap<FAssetData, EPrintMode> PendingOutputModesALS;

    TMap<FAssetData, ECheckBoxState> PendingCheckedStateUPS;

private:
    TSharedPtr<SWindow> ExistingALSWindow = nullptr;
    TSharedPtr<SWindow> ExistingUPSWindow = nullptr;

    TSharedPtr<SScrollBox> ALSScrollContainer = nullptr;
    TSharedPtr<SScrollBox> UPSScrollContainer = nullptr;

    TArray<TSharedPtr<FPrintModeRowData>> PrintModeRowCache;
    TArray<TSharedPtr<FLogSeverityRowData>> LogSeverityRowCache;

};