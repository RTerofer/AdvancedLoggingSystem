//Copyright © 2025 RTerofer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "K2Node.h"
#include "BlueprintActionDatabaseRegistrar.h" 
#include "ALS_Definitions.h"
#include "ALSE_Node.generated.h"

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
#define ALSNotifyNode(Graph, Node) Graph->NotifyNodeChanged(Node);
#else
#define ALSNotifyNode(Graph, Node) Graph->NotifyGraphChanged();
#endif

USTRUCT()
struct FSavedPinData
{
    GENERATED_BODY()

    UPROPERTY()
    FGuid PinId;

    UPROPERTY()
    FName PinName;

    UPROPERTY()
    FText PinFriendlyName;

    UPROPERTY()
    FString PinToolTip;

    UPROPERTY()
    FEdGraphPinType PinType;

    UPROPERTY()
    TEnumAsByte<EEdGraphPinDirection> Direction;

    UPROPERTY()
    bool bAdvancedView = false;

    UPROPERTY()
    bool HasAnyConnections = false;

    FSavedPinData() {}

	FSavedPinData(UEdGraphPin* Pin) 
        : PinId(Pin->PinId), PinName(Pin->PinName)
        , PinFriendlyName(Pin->PinFriendlyName)
        , PinToolTip(Pin->PinToolTip)
        , PinType(Pin->PinType)
        , Direction(Pin->Direction)
        , bAdvancedView(Pin->bAdvancedView)
        , HasAnyConnections(Pin->HasAnyConnections())
    {}
};

UCLASS()
class ALS_EDITOR_API UALS_Node : public UK2Node
{
    GENERATED_BODY()

protected:
    UALS_Node();
    virtual void BeginDestroy() override;
    virtual FText GetMenuCategory() const override;
    virtual FText GetKeywords() const override;
    virtual FLinearColor GetNodeTitleColor() const override;
    virtual FLinearColor GetNodeBodyTintColor() const override;
    virtual FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
    virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
    virtual FText GetTooltipText() const override;
    virtual bool IsConnectionDisallowed(const UEdGraphPin* MyPin, const UEdGraphPin* OtherPin, FString& OutReason) const override;
    virtual void PinConnectionListChanged(UEdGraphPin* Pin) override;
    virtual void ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins) override;
    virtual void Serialize(FArchive& Ar) override;
    virtual void PostEditImport() override;
    virtual void PostEditUndo() override;
    virtual void PostLoad() override;
    virtual bool ShouldShowNodeProperties() const override { return true; };
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
    virtual void PrepareForCopying() override;
    virtual void PostPasteNode() override;
    virtual bool IsNodeSafeToIgnore() const override;
    virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
    virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
    virtual TSharedPtr<SGraphNode> CreateVisualWidget() override;

public:
    virtual void AllocateDefaultPins() override;
    virtual void ReconstructNode() override;

private:
    void HandleGraphChanged(const FEdGraphEditAction& Action);

    void CreateDefaultPins();
    void CreateAdvancedPins();
    UEdGraphPin* CreateTextLocationPin();
    UEdGraphPin* CreateWorldContextPin();

    void SaveCurrentPins();

    FString GetAlphabetFromIndex(int32 Index) const;
    int32 GetSavedPinIndex(UEdGraphPin* Pin) const;
    int32 GetSavedPinIndex(FGuid PinId) const;

public:
    UEdGraphPin* CreateWildcardPin();
    UEdGraphPin* CreateExecPin();

    void MarkBlueprintDirty(bool NotifyNode = true);
    void RemoveInputPin(UEdGraphPin* PinToRemove);
    void SetDefaultValue(UEdGraphPin* Pin);
    void SetPinDefaultsByPreset();
    
    void UpdateTextLocationPin();

    FSavedPinData GetSavedPinData(UEdGraphPin* Pin) const;

    FString GetSavedDefaultValue(UEdGraphPin* Pin) const;
    FString GetSavedDefaultValue(FGuid PinId) const;
    TArray<UEdGraphPin*> GetExecPins() const;
    TArray<UEdGraphPin*> GetThenPins() const;
    TArray<UEdGraphPin*> GetDataPins() const;

    UEdGraphPin* GetCorrespondingThenPin(UEdGraphPin* ExecPin) const;

    int32 GetWildcardPinConnectedCount() const;
    int32 GetWildcardPinCount() const;
    int32 GetExecPinCount() const;

    bool IsWildcardPin(const UEdGraphPin* Pin) const;
    bool IsNonContextBP() const;

private:
    void UpgradeNode(int32 OldVersion);

private:

    UPROPERTY()
    int32 CurrentVersion = 0;

    UPROPERTY()
    TArray<FSavedPinData> SavedPins;

    UPROPERTY()
    int32 NumberOfInputs = 1;    
    
    UPROPERTY()
    int32 NumberOfExecs = 1;

public:
    FDelegateHandle GraphChangedHandle;
    TWeakObjectPtr<UEdGraph> BoundGraphWeakPtr;

    UPROPERTY()
    TEnumAsByte<ENodeAdvancedPins::Type> SavedAdvancedPinState = ENodeAdvancedPins::Hidden;

    UPROPERTY()
    int32 MaximumInputs = 15;

    UPROPERTY()
    int32 MaximumExecs = 10;

    UPROPERTY()
    TMap<FGuid, FString> SavedDefaultValues;

    UPROPERTY()
    TArray<FGuid> AdvancedPins;

    UPROPERTY()
    FGuid WildcardPinId;

    UPROPERTY()
    FGuid ExecPinId;

    UPROPERTY()
    FGuid WorldContextId;

    UPROPERTY()
    FGuid TextLocationPinId;

    UPROPERTY()
    FGuid DurationPinId;

    UPROPERTY()
    FGuid ColorPinId;

    UPROPERTY()
    FGuid KeyPinId;

    UPROPERTY()
    FGuid PrintModePinId;

    UPROPERTY()
    FGuid LogSeverityPinId;

    UPROPERTY()
    FGuid PresetPinId;

    UPROPERTY()
    bool bAdvancedView;

    UPROPERTY(EditDefaultsOnly, meta = (DisplayName = "Toggle Node", Category = "General"))
    bool bToggleNode = true;

    UPROPERTY(EditDefaultsOnly, meta = (DisplayName = "Print To World", Category = "General"))
    bool bPrintToWorld = false;

    UPROPERTY(EditDefaultsOnly, meta = (DisplayName = "Show World Context", Category = "General"))
    bool bShowWorldContext = false;

    UPROPERTY(EditDefaultsOnly, meta = (DisplayName = "Duration", Category = "Config"))
    float PrintDuration;

    UPROPERTY(EditDefaultsOnly, meta = (DisplayName = "Color", Category = "Config"))
    FLinearColor PrintColor;

    UPROPERTY(EditDefaultsOnly, meta = (DisplayName = "Key", Category = "Config"))
    FName PrintKey;

    UPROPERTY(EditDefaultsOnly, meta = (DisplayName = "Print Mode", Category = "Config"))
    EPrintMode PrintMode;

    UPROPERTY(EditDefaultsOnly, meta = (DisplayName = "Log Level", Category = "Config"))
    ELogSeverity LogSeverity;

    UPROPERTY(EditDefaultsOnly, meta = (DisplayName = "Config Preset", Category = "Presets"))
    EPrintPreset PrintPreset;

};
