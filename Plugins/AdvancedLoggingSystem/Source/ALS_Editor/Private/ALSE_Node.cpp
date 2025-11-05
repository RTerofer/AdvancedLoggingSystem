//Copyright © 2025 RTerofer. All Rights Reserved.

#include "ALSE_Node.h"
#include "ALSE_SGraphNode.h"
#include "ALSE_Style.h"
#include "ALS_FunctionLibrary.h"
#include "ALS_Settings.h"
#include "ScopedTransaction.h"
#include "EdGraph/EdGraphPin.h"
#include "KismetCompiler.h" 
#include "EditorStyleSet.h"
#include "GraphEditAction.h"
#include "BlueprintNodeSpawner.h" 
#include "Framework/Commands/GenericCommands.h"
#include "Kismet/KismetSystemLibrary.h"
#include "EditorUtilityBlueprint.h"
#include "Animation/AnimBlueprint.h"
#include "K2Node_CallFunction.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Subsystems/EditorAssetSubsystem.h" 
#include "K2Node_MakeArray.h"
#include "FileHelpers.h"

static int32 UpgradedVersion = 0;

UALS_Node::UALS_Node()
{
    bCanRenameNode = false;
}

void UALS_Node::BeginDestroy()
{
    if (BoundGraphWeakPtr.IsValid() && GraphChangedHandle.IsValid())
    {
        GetGraph()->RemoveOnGraphChangedHandler(GraphChangedHandle);
        GraphChangedHandle.Reset();
    }
    Super::BeginDestroy();
}

void UALS_Node::MarkBlueprintDirty(bool NotifyNode)
{
    if (UBlueprint* Blueprint = GetBlueprint())
    {
        Blueprint->Modify();
        FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

        if (NotifyNode)
        {
            ALSNotifyNode(GetGraph(), this);
        }
    }
}

void UALS_Node::AllocateDefaultPins()
{
    Super::AllocateDefaultPins();

    if (SavedPins.Num() > 0)
    {
        ReallocatePinsDuringReconstruction(Pins);
    }
    else
    {
        CreateDefaultPins();
		ReconstructNode();
    }
}

void UALS_Node::CreateDefaultPins()
{
    if (!DurationPinId.IsValid())
    {
        ExecPinId = CreateExecPin()->PinId;
    }
    
    if (!WorldContextId.IsValid())
    {
        WorldContextId = CreateWorldContextPin()->PinId;
    }

    if (!TextLocationPinId.IsValid())
    {
        TextLocationPinId = CreateTextLocationPin()->PinId;
    }

    if (!WildcardPinId.IsValid())
    {
        WildcardPinId = CreateWildcardPin()->PinId;
    }

    CreateAdvancedPins();
}

void UALS_Node::CreateAdvancedPins()
{
    if(!DurationPinId.IsValid())
    {
        UEdGraphPin* DurationPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Real, UEdGraphSchema_K2::PC_Float, nullptr, TEXT("Duration"));
        DurationPin->PinFriendlyName = FText::FromString("Duration");
        DurationPin->bAdvancedView = true;
        DurationPin->bHidden = true;
        DurationPin->PinToolTip = FString("Time in secs the message stays visible on screen.");

        PrintDuration = UALS_Settings::GetConfigFromPreset(PrintPreset).Duration;

        DurationPin->DefaultValue = FString::SanitizeFloat(PrintDuration);
        SetDefaultValue(DurationPin);

        DurationPinId = DurationPin->PinId;
	}

    if (!ColorPinId.IsValid())
    {
        UEdGraphPin* ColorPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Struct, NAME_None, TBaseStructure<FLinearColor>::Get(), TEXT("Color"));
        ColorPin->PinFriendlyName = FText::FromString("Color");
        ColorPin->bAdvancedView = true;
        ColorPin->bHidden = true;
        ColorPin->PinToolTip = FString("Color of the On-Screen message.");

        PrintColor = UALS_Settings::GetConfigFromPreset(PrintPreset).Color;

        ColorPin->DefaultValue = PrintColor.ToString();
        SetDefaultValue(ColorPin);

        ColorPinId = ColorPin->PinId;
    }

    if (!KeyPinId.IsValid())
    {
        UEdGraphPin* KeyPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Name, NAME_None, TBaseStructure<FLinearColor>::Get(), TEXT("Key"));
        KeyPin->PinFriendlyName = FText::FromString("Key");
        KeyPin->bAdvancedView = true;
        KeyPin->bHidden = true;
        KeyPin->PinToolTip = FString("When Key Provided, the Messages of same key will be replaced. Useful in tick messages");

        PrintKey = UALS_Settings::GetConfigFromPreset(PrintPreset).Key;

        KeyPin->DefaultValue = PrintKey.ToString();
        SetDefaultValue(KeyPin);

        KeyPinId = KeyPin->PinId;
    }

    if (!PrintModePinId.IsValid())
    {
        UEdGraphPin* PrintModePin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Byte, StaticEnum<EPrintMode>(), TEXT("PrintMode"));
        PrintModePin->PinFriendlyName = FText::FromString("PrintMode\t\t\t\t");
        PrintModePin->bAdvancedView = true;
        PrintModePin->bHidden = true;
        PrintModePin->PinToolTip = FString("Choose how the message is output: on screen, log or both.");

        PrintMode = UALS_Settings::GetConfigFromPreset(PrintPreset).PrintMode;

        PrintModePin->DefaultValue = UEnum::GetValueAsString(PrintMode);
        SetDefaultValue(PrintModePin);

        PrintModePinId = PrintModePin->PinId;
    }

    if (!LogSeverityPinId.IsValid())
    {
        UEdGraphPin* LogSeverityPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Byte, StaticEnum<ELogSeverity>(), TEXT("LogSeverity"));
        LogSeverityPin->PinFriendlyName = FText::FromString("LogSeverity\t\t\t\t");
        LogSeverityPin->bAdvancedView = true;
        LogSeverityPin->bHidden = true;
        LogSeverityPin->PinToolTip = FString("Set the severity level to Info, Warning or Error.");

        LogSeverity = UALS_Settings::GetConfigFromPreset(PrintPreset).LogSeverity;

        LogSeverityPin->DefaultValue = UEnum::GetValueAsString(LogSeverity);
        SetDefaultValue(LogSeverityPin);

        LogSeverityPinId = LogSeverityPin->PinId;
    }

    AdvancedPins.AddUnique(DurationPinId);
    AdvancedPins.AddUnique(ColorPinId);
    AdvancedPins.AddUnique(KeyPinId);
    AdvancedPins.AddUnique(PrintModePinId);
    AdvancedPins.AddUnique(LogSeverityPinId);
}

UEdGraphPin* UALS_Node::CreateTextLocationPin()
{
    UEdGraphPin* DebugWorld_Pin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Wildcard, TEXT("TextLocation"));
    DebugWorld_Pin->PinFriendlyName = FText::FromString("Text Location");
    DebugWorld_Pin->DefaultValue = FVector::ZeroVector.ToString();
    DebugWorld_Pin->bHidden = true;
    DebugWorld_Pin->PinToolTip = FString("Enable this to draw the debug message in world space");
    SetDefaultValue(DebugWorld_Pin);

    return DebugWorld_Pin;
}

UEdGraphPin* UALS_Node::CreateWorldContextPin()
{
    UEdGraphPin* WorldContextPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Object, UObject::StaticClass(), TEXT("WorldContextObject"));
    WorldContextPin->PinFriendlyName = FText::FromString(TEXT("World Context"));
    WorldContextPin->bHidden = true;
    WorldContextPin->PinToolTip = FString("This will be used as CallerContext and to GetWorld when creating logs.");
    SetDefaultValue(WorldContextPin);

    return WorldContextPin;
}

UEdGraphPin* UALS_Node::CreateWildcardPin()
{
    FString PinName = FString::Printf(TEXT("Input %d"), NumberOfInputs);
    UEdGraphPin* NewInputPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Wildcard, *PinName);

    FString FriendlyName = FString::Printf(TEXT("%s"), *GetAlphabetFromIndex(GetWildcardPinCount()));
    NewInputPin->PinFriendlyName = FText::FromString(FriendlyName);

    FString DefaultPinValue = UALS_Settings::Get()->PinDefaultValue;
    NewInputPin->DefaultValue = DefaultPinValue;
    NewInputPin->PinToolTip = FString("Attach a variable or literal to be printed. Connect any data type.");
    SetDefaultValue(NewInputPin);

    NumberOfInputs++;
    return NewInputPin;
}

UEdGraphPin* UALS_Node::CreateExecPin()
{
    FString PinName = FString::Printf(TEXT("Exec %d"), NumberOfExecs);
    UEdGraphPin* NewExecPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, FName(*PinName));
    NewExecPin->PinFriendlyName = FText::FromString(" ");

    FString DefaultExecValue = UALS_Settings::Get()->ExecDefaultValue;
    NewExecPin->DefaultValue = DefaultExecValue;
    NewExecPin->PinToolTip = FString("Enter optional [Label] that will be shown when this exec triggers.");
    SetDefaultValue(NewExecPin);

    FString ThenPinName = FString::Printf(TEXT("Then %d"), NumberOfExecs);
    UEdGraphPin* NewThenPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, *ThenPinName);
    NewThenPin->PinFriendlyName = FText::FromString(" ");
    NewThenPin->PinToolTip = FString("Exec");

    NumberOfExecs++;
    return NewExecPin;
}

void UALS_Node::UpdateTextLocationPin()
{
    UEdGraphPin* DurationPin = FindPinById(DurationPinId);
    UEdGraphPin* ColorPin = FindPinById(ColorPinId);

    DurationPin->bAdvancedView = !bPrintToWorld;
    ColorPin->bAdvancedView = !bPrintToWorld;

    if (!bPrintToWorld)
    {
        UEdGraphPin* DebugPin = FindPinById(TextLocationPinId);
        DebugPin->BreakAllPinLinks();
        DebugPin->PinType.PinCategory = UEdGraphSchema_K2::PC_Wildcard;
        DebugPin->PinFriendlyName = FText::FromString("Text Location");
        DebugPin->DefaultValue = GetSavedDefaultValue(DebugPin);
    }
}

void UALS_Node::RemoveInputPin(UEdGraphPin* PinToRemove)
{
    const FScopedTransaction Transaction(NSLOCTEXT("ALS", "UndoText", "PinRemoval"));
    Modify();
    GetGraph()->Modify();

    if (!PinToRemove || !Pins.Contains(PinToRemove))
    {
        return;
    }

    if (PinToRemove->PinType.PinCategory == UEdGraphSchema_K2::PC_Exec && PinToRemove->Direction == EGPD_Input)
    {
        UEdGraphPin* PossibleThen = GetCorrespondingThenPin(PinToRemove);

        if (PossibleThen)
        {
            int32 FoundIndex = GetSavedPinIndex(PossibleThen);

            if (FoundIndex != INDEX_NONE)
            {
                SavedPins.RemoveAt(FoundIndex);
            }

            Pins.Remove(PossibleThen);
            PossibleThen->Modify();
            PossibleThen->MarkAsGarbage();
        }
    }

    int32 FoundIndex = GetSavedPinIndex(PinToRemove);

    if (FoundIndex != INDEX_NONE)
    {
        SavedPins.RemoveAt(FoundIndex);
    }

    SavedDefaultValues.Remove(PinToRemove->PinId);

    Pins.Remove(PinToRemove);
    PinToRemove->Modify();
    PinToRemove->MarkAsGarbage();

    ReconstructNode();
}

void UALS_Node::Serialize(FArchive& Ar)
{
    Super::Serialize(Ar);
    Ar << CurrentVersion;
}

void UALS_Node::PostLoad()
{
    Super::PostLoad();

    if (CurrentVersion != UpgradedVersion)
    {
        UpgradeNode(CurrentVersion);
    }
}

void UALS_Node::UpgradeNode(int32 OldVersion)
{
    if (OldVersion == 0)
    {
		// Upgrade logic from version 0 to 1
    }
}

void UALS_Node::PostEditImport()
{
    Super::PostEditImport();
}

void UALS_Node::PostEditUndo()
{
    Super::PostEditUndo();
}

void UALS_Node::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    if (!PropertyChangedEvent.Property) return;

    FName PropertyName = PropertyChangedEvent.Property->GetFName();

    if (PropertyName == GET_MEMBER_NAME_CHECKED(UALS_Node, PrintDuration))
    {
        const FScopedTransaction Transaction(NSLOCTEXT("ALS", "UndoText", "Duration"));
        Modify();
        GetGraph()->Modify();

        if (UEdGraphPin* Pin = FindPinById(DurationPinId))
        {
            Pin->DefaultValue = FString::SanitizeFloat(PrintDuration);
            SetDefaultValue(Pin);

            ALSNotifyNode(GetGraph(), this);
        }
    }

    if (PropertyName == GET_MEMBER_NAME_CHECKED(UALS_Node, PrintColor))
    {
        const FScopedTransaction Transaction(NSLOCTEXT("ALS", "UndoText", "Color"));
        Modify();
        GetGraph()->Modify();

        if (UEdGraphPin* Pin = FindPinById(ColorPinId))
        {
            Pin->DefaultValue = PrintColor.ToString();
            SetDefaultValue(Pin);

            ALSNotifyNode(GetGraph(), this);
        }
    }

    if (PropertyName == GET_MEMBER_NAME_CHECKED(UALS_Node, PrintKey))
    {
        const FScopedTransaction Transaction(NSLOCTEXT("ALS", "UndoText", "Key"));
        Modify();
        GetGraph()->Modify();

        if (UEdGraphPin* Pin = FindPinById(KeyPinId))
        {
            Pin->DefaultValue = PrintKey.ToString();
            SetDefaultValue(Pin);

            ALSNotifyNode(GetGraph(), this);
        }
    }

    if (PropertyName == GET_MEMBER_NAME_CHECKED(UALS_Node, LogSeverity))
    {
        const FScopedTransaction Transaction(NSLOCTEXT("ALS", "UndoText", "LogSeverity"));
        Modify();
        GetGraph()->Modify();

        if (UEdGraphPin* Pin = FindPinById(LogSeverityPinId))
        {
            UEnum* Enum = StaticEnum<ELogSeverity>();
            Pin->DefaultValue = Enum->GetValueAsString(LogSeverity);
            SetDefaultValue(Pin);

            ALSNotifyNode(GetGraph(), this);
        }
    }    
    
    if (PropertyName == GET_MEMBER_NAME_CHECKED(UALS_Node, PrintMode))
    {
        const FScopedTransaction Transaction(NSLOCTEXT("ALS", "UndoText", "PrintMode"));
        Modify();
        GetGraph()->Modify();

        if (UEdGraphPin* Pin = FindPinById(PrintModePinId))
        {
            UEnum* Enum = StaticEnum<EPrintMode>();
            Pin->DefaultValue = Enum->GetValueAsString(PrintMode);
            SetDefaultValue(Pin);

            ALSNotifyNode(GetGraph(), this);
        }
    }

    if (PropertyName == GET_MEMBER_NAME_CHECKED(UALS_Node, PrintPreset))
    {
        const FScopedTransaction Transaction(NSLOCTEXT("ALS", "UndoText", "PrintPreset"));
        Modify();
        GetGraph()->Modify();
        SetPinDefaultsByPreset();
        ALSNotifyNode(GetGraph(), this);
        
    }

    if (PropertyName == GET_MEMBER_NAME_CHECKED(UALS_Node, bToggleNode))
    {
        const FScopedTransaction Transaction(NSLOCTEXT("ALS", "UndoText", "ActiveState"));
        Modify();
        GetGraph()->Modify();
        ALSNotifyNode(GetGraph(), this);
    }

    if (PropertyName == GET_MEMBER_NAME_CHECKED(UALS_Node, bPrintToWorld))
    {
        const FScopedTransaction Transaction(NSLOCTEXT("ALS", "UndoText", "PrintToWorld"));
        Modify();
        GetGraph()->Modify();
        UpdateTextLocationPin();
        ALSNotifyNode(GetGraph(), this);
    }

    if (PropertyName == GET_MEMBER_NAME_CHECKED(UALS_Node, bShowWorldContext))
    {
        const FScopedTransaction Transaction(NSLOCTEXT("ALS", "UndoText", "ShowWorldContext"));
        Modify();
        GetGraph()->Modify();
        ALSNotifyNode(GetGraph(), this);
    }
}

void UALS_Node::PrepareForCopying()
{
    Super::PrepareForCopying();
}

void UALS_Node::PostPasteNode()
{
    Super::PostPasteNode();

    for (auto Pin : Pins)
    {
        if (IsWildcardPin(Pin))
        {
            if (!Pin->HasAnyConnections())
            {
                Pin->PinType.PinCategory = UEdGraphSchema_K2::PC_Wildcard;
                Pin->PinType.ContainerType = EPinContainerType::None;
            }
        }
    }
}

void UALS_Node::ReconstructNode()
{
    if (GetGraph() && !GraphChangedHandle.IsValid())
    {
        BoundGraphWeakPtr = GetGraph();
        GraphChangedHandle = GetGraph()->AddOnGraphChangedHandler(FOnGraphChanged::FDelegate::CreateUObject(this, &UALS_Node::HandleGraphChanged));
    }

    SaveCurrentPins();
    Super::ReconstructNode();
}

bool UALS_Node::IsNodeSafeToIgnore() const
{
    return true;
}

void UALS_Node::SaveCurrentPins()
{
    auto SavePin = [&](UEdGraphPin* Pin)
        {
            FSavedPinData PinData = FSavedPinData(Pin);
            int32 FoundIndex = GetSavedPinIndex(Pin);

            if (FoundIndex != INDEX_NONE)
            {
                SavedPins[FoundIndex] = PinData;
            }
            else
            {
                SavedPins.Add(PinData);
            }
        };

    // Exec
    for (UEdGraphPin* Pin : Pins)
    {
        if (Pin->PinType.PinCategory == UEdGraphSchema_K2::PC_Exec)
        {
            SavePin(Pin);
        }
    }

    // Data
    for (UEdGraphPin* Pin : Pins)
    {
        if (Pin->PinType.PinCategory != UEdGraphSchema_K2::PC_Exec && !AdvancedPins.Contains(Pin->PinId))
        {
            if (Pin->HasAnyConnections())
            {
                Pin->PinType = Pin->LinkedTo[0]->PinType;
            }

            SavePin(Pin);
        }
    }

    // Advanced
    for (UEdGraphPin* Pin : Pins)
    {
        if (AdvancedPins.Contains(Pin->PinId))
        {
            SavePin(Pin);
        }
    }
}

void UALS_Node::ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins)
{
    // Exec
    for (const FSavedPinData& SavedPin : SavedPins)
    {
        if (SavedPin.PinType.PinCategory == UEdGraphSchema_K2::PC_Exec)
        {
            UEdGraphPin* ExecPin = CreatePin(SavedPin.Direction, SavedPin.PinType.PinCategory, SavedPin.PinName);
            ExecPin->PinId = SavedPin.PinId;
            ExecPin->PinFriendlyName = SavedPin.PinFriendlyName;
            ExecPin->DefaultValue = GetSavedDefaultValue(ExecPin);
            ExecPin->PinToolTip = SavedPin.PinToolTip;
        }
    }

    // Data
    int32 NumPinOrder = 0;
    for (const FSavedPinData& SavedPin : SavedPins)
    {
        if (SavedPin.PinType.PinCategory != UEdGraphSchema_K2::PC_Exec && !AdvancedPins.Contains(SavedPin.PinId))
        {
            UEdGraphPin* InputPin = CreatePin(EGPD_Input, SavedPin.PinType.PinCategory, SavedPin.PinType.PinSubCategoryObject.Get(), SavedPin.PinName);
            InputPin->PinType = SavedPin.PinType;
            InputPin->PinId = SavedPin.PinId;
            InputPin->PinToolTip = SavedPin.PinToolTip;

            if (InputPin->PinId != TextLocationPinId && InputPin->PinId != WorldContextId)
            {
                NumPinOrder++;
                FString FriendlyName = FString::Printf(TEXT("%s"), *GetAlphabetFromIndex(NumPinOrder));
                InputPin->PinFriendlyName = FText::FromString(FriendlyName);
            }
            else
            {
                InputPin->PinFriendlyName = SavedPin.PinFriendlyName;
            }

            InputPin->DefaultValue = GetSavedDefaultValue(InputPin);
        }
    }

    // Advanced
    for (const FSavedPinData& SavedPin : SavedPins)
    {
        if (AdvancedPins.Contains(SavedPin.PinId))
        {
            UEdGraphPin* AdvancedPin = CreatePin(SavedPin.Direction, SavedPin.PinType.PinCategory, SavedPin.PinType.PinSubCategoryObject.Get(), SavedPin.PinName);
            AdvancedPin->PinId = SavedPin.PinId;
            AdvancedPin->PinFriendlyName = SavedPin.PinFriendlyName;
            AdvancedPin->PinToolTip = SavedPin.PinToolTip;
            AdvancedPin->bAdvancedView = SavedPin.bAdvancedView;
            AdvancedPin->DefaultValue = GetSavedDefaultValue(AdvancedPin);
        }
    }
}

void UALS_Node::HandleGraphChanged(const FEdGraphEditAction& Action)
{
    for (UEdGraphPin* Pin : GetDataPins())
    {
        if (Pin->HasAnyConnections())
        {
            Pin->PinType = Pin->LinkedTo[0]->PinType;
        }
    }   
}

bool UALS_Node::IsConnectionDisallowed(const UEdGraphPin* MyPin, const UEdGraphPin* OtherPin, FString& OutReason) const
{
    if (MyPin->PinId == TextLocationPinId)
    {
        const FEdGraphPinType& OtherPinType = OtherPin->PinType;
        const FName OtherPinSubCategory = OtherPinType.PinSubCategory;

        bool ConnectionFailed = true;

        if (OtherPinType.PinSubCategoryObject.IsValid())
        {
            if (UClass* ConnectedClass = Cast<UClass>(OtherPinType.PinSubCategoryObject.Get()))
            {
                if (ConnectedClass->IsChildOf(AActor::StaticClass()) || ConnectedClass->IsChildOf(USceneComponent::StaticClass()))
                {
                    ConnectionFailed = false;
                }
            }

            if (OtherPinType.PinSubCategoryObject == TBaseStructure<FVector>::Get())
            {
                ConnectionFailed = false;
            }
        }

        if (OtherPinSubCategory == FName("self"))
        {
            ConnectionFailed = false;
        }

        if (ConnectionFailed)
        {
            OutReason = TEXT("Only Actors, Scene Component and Vectors are allowed to be connected");
            return true;
        }

        return ConnectionFailed;
    }

    if (IsWildcardPin(MyPin))
    {
        const int32 ConnectionCount = GetWildcardPinConnectedCount();
        if (ConnectionCount >= 10)
        {
            OutReason = TEXT("Maximum of 10 connections allowed, Try to disconnect any or use ALS Append Node!");
            return true;
        }

        if (OtherPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Exec)
        {
            OutReason = TEXT("Execution pins are not allowed!");
            return true;
        }

        return false;
    }

    return false;
}

void UALS_Node::PinConnectionListChanged(UEdGraphPin* Pin)
{
    Super::PinConnectionListChanged(Pin);

    const FScopedTransaction Transaction(NSLOCTEXT("ALS", "UndoText", "Connection"));
    Modify();
    GetGraph()->Modify();

    FEdGraphPinType& CurrentPinType = Pin->PinType;

    // On Connected
    if (Pin->HasAnyConnections())
    {
        const UEdGraphPin* ConnectedPin = Pin->LinkedTo[0];
        const FEdGraphPinType& ConnectedPinType = ConnectedPin->PinType;

        // On Text Location Pin
        if (Pin->PinId == TextLocationPinId)
        {
            bool BaseConnectionSuccess = false;

            if (ConnectedPinType.PinSubCategoryObject.IsValid())
            {
                if (UClass* ConnectedClass = Cast<UClass>(ConnectedPinType.PinSubCategoryObject.Get()))
                {
                    if (ConnectedClass->IsChildOf(AActor::StaticClass()))
                    {
                        Pin->PinFriendlyName = FText::FromString("TextLocation - Actor + Offset");
                        BaseConnectionSuccess = true;
                    }
                    if (ConnectedClass->IsChildOf(USceneComponent::StaticClass()))
                    {
                        Pin->PinFriendlyName = FText::FromString("TextLocation - Scene + Offset");
                        BaseConnectionSuccess = true;
                    }
                }

                if (ConnectedPinType.PinSubCategoryObject == TBaseStructure<FVector>::Get())
                {
                    Pin->PinFriendlyName = FText::FromString("TextLocation - Vector");
                    BaseConnectionSuccess = true;
                }
            }

            if (ConnectedPinType.PinSubCategory == FName("self"))
            {
                Pin->PinFriendlyName = FText::FromString("TextLocation - Self + Offset");
                BaseConnectionSuccess = true;
            }

            if (BaseConnectionSuccess)
            {
                CurrentPinType = ConnectedPinType;
            }
        }

        // On Data Pins
        if (IsWildcardPin(Pin))
        {
            CurrentPinType = ConnectedPinType;
        }

        MarkBlueprintDirty();
        return;
    }

    //On Disconnected
    else
    {
        // On Text Location Pin
        if (Pin->PinId == TextLocationPinId)
        {
            Pin->PinFriendlyName = FText::FromString("Text Location");
        }

        // On Data Pins
        if (IsWildcardPin(Pin) || Pin->PinId == TextLocationPinId)
        {
            CurrentPinType.PinCategory = UEdGraphSchema_K2::PC_Wildcard;
            CurrentPinType.ContainerType = EPinContainerType::None;
            CurrentPinType.PinSubCategory = NAME_None;
            CurrentPinType.PinSubCategoryObject = nullptr;
        }

        Pin->DefaultValue = GetSavedDefaultValue(Pin);
        MarkBlueprintDirty();
        return;
    }
}

void UALS_Node::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
    Super::ExpandNode(CompilerContext, SourceGraph);

    TArray<UEdGraphPin*> ExecInPins = GetExecPins();
    TArray<UEdGraphPin*> ExecOutPins = GetThenPins();

    if (ExecInPins.Num() == 0 || ExecOutPins.Num() == 0)
    {
        CompilerContext.MessageLog.Error(TEXT("ALS: Missing Exec Pins!"));
        BreakAllNodeLinks();
        return;
    }

    TArray<UEdGraphPin*> DataPins = GetDataPins();
    int32 NumConnected = GetWildcardPinConnectedCount();

    int32 MaxValues = 0;
    FName FunctionName = NAME_None;

    if (NumConnected <= 3)
    {
        FunctionName = FName("PrintHelperONE");
        MaxValues = 3;
    }
    else if (NumConnected <= 5)
    {
        FunctionName = FName("PrintHelperTWO");
        MaxValues = 5;
    }
    else if (NumConnected <= 7)
    {
        FunctionName = FName("PrintHelperTHREE");
        MaxValues = 7;
    }
    else if (NumConnected <= 10)
    {
        FunctionName = FName("PrintHelperFOUR");
        MaxValues = 10;
    }
    else
    {
        CompilerContext.MessageLog.Error(TEXT("ALS: Upto 10 connected pins are allowed to compile. Disconnect additional pins. Optionally use ALS Append Node"));
        BreakAllNodeLinks();
        return;
    }

    auto SpawnHelperNode = [&](int32 ExecInt, UEdGraphPin* ThisExecIn, UEdGraphPin* ThisExecOut)
        {
            UK2Node_CallFunction* PrintHelperNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
            PrintHelperNode->FunctionReference.SetExternalMember(FunctionName, UALS_FunctionLibrary::StaticClass());
            PrintHelperNode->AllocateDefaultPins();

            UEdGraphPin* PH_ContextPin = PrintHelperNode->FindPinChecked(TEXT("WorldContextObject"));
            UEdGraphPin* PH_ColorPin = PrintHelperNode->FindPinChecked(TEXT("Color"));
            UEdGraphPin* PH_KeyPin = PrintHelperNode->FindPinChecked(TEXT("Key"));
            UEdGraphPin* PH_DurationPin = PrintHelperNode->FindPinChecked(TEXT("Duration"));
            UEdGraphPin* PH_PrintModePin = PrintHelperNode->FindPinChecked(TEXT("PrintMode"));
            UEdGraphPin* PH_LogSeverityPin = PrintHelperNode->FindPinChecked(TEXT("LogSeverity"));
            UEdGraphPin* PH_DrawDebugPin = PrintHelperNode->FindPinChecked(TEXT("DrawDebug"));
            UEdGraphPin* PH_TextLocation = PrintHelperNode->FindPinChecked(TEXT("TextLocation"));
            UEdGraphPin* PH_BaseObject = PrintHelperNode->FindPinChecked(TEXT("BaseObject"));
            UEdGraphPin* PH_SourceIDPin = PrintHelperNode->FindPinChecked(TEXT("SourceID"));

            UEdGraphPin* MyContextPin = FindPinById(WorldContextId);
            UEdGraphPin* MyColorPin = FindPinById(ColorPinId);
            UEdGraphPin* MyKeyPin = FindPinById(KeyPinId);
            UEdGraphPin* MyDurationPin = FindPinById(DurationPinId);
            UEdGraphPin* MyPrintModePin = FindPinById(PrintModePinId);
            UEdGraphPin* MyLogSeverityPin = FindPinById(LogSeverityPinId);
            UEdGraphPin* MyTextLocPin = FindPinById(TextLocationPinId);

			// Context Pin
            if (MyContextPin)
            {
                if (MyContextPin->HasAnyConnections()) CompilerContext.CopyPinLinksToIntermediate(*MyContextPin, *PH_ContextPin);
            }

			// Color Pin
            if (MyColorPin)
            {
                if (MyColorPin->HasAnyConnections()) CompilerContext.CopyPinLinksToIntermediate(*MyColorPin, *PH_ColorPin);
                else PH_ColorPin->DefaultValue = GetSavedDefaultValue(MyColorPin);
            }
            else
            {
				PH_ColorPin->DefaultValue = PrintColor.ToString();
            }

			// Key Pin
            if (MyKeyPin)
            {
                if (MyKeyPin->HasAnyConnections()) CompilerContext.CopyPinLinksToIntermediate(*MyKeyPin, *PH_KeyPin);
                else PH_KeyPin->DefaultValue = GetSavedDefaultValue(MyKeyPin);
            }
            else
            {
				PH_KeyPin->DefaultValue = PrintKey.ToString();
            }

			// Duration Pin
            if (MyDurationPin)
            {
                if (MyDurationPin->HasAnyConnections()) CompilerContext.CopyPinLinksToIntermediate(*MyDurationPin, *PH_DurationPin);
                else PH_DurationPin->DefaultValue = GetSavedDefaultValue(MyDurationPin);
            }
            else
            {
				PH_DurationPin->DefaultValue = FString::SanitizeFloat(PrintDuration);
            }

			// PrintMode Pin
            if (MyPrintModePin)
            {
                if (MyPrintModePin->HasAnyConnections()) CompilerContext.CopyPinLinksToIntermediate(*MyPrintModePin, *PH_PrintModePin);
                else PH_PrintModePin->DefaultValue = GetSavedDefaultValue(MyPrintModePin);
            }
            else
            {
                UEnum* Enum = StaticEnum<EPrintMode>();
				PH_PrintModePin->DefaultValue = Enum->GetValueAsString(PrintMode);
            }

			// LogSeverity Pin
            if (MyLogSeverityPin)
            {
                if (MyLogSeverityPin->HasAnyConnections()) CompilerContext.CopyPinLinksToIntermediate(*MyLogSeverityPin, *PH_LogSeverityPin);
                else PH_LogSeverityPin->DefaultValue = GetSavedDefaultValue(MyLogSeverityPin);
            }
            else
            {
                UEnum* Enum = StaticEnum<ELogSeverity>();
                PH_LogSeverityPin->DefaultValue = Enum->GetValueAsString(LogSeverity);
            }

            PH_DrawDebugPin->DefaultValue = bPrintToWorld ? "true" : "false";

            FString SourceID = FString::Printf(TEXT("%s::%d"), *GetNodeTitle(ENodeTitleType::MenuTitle).ToString(), this->GetUniqueID());

            PH_SourceIDPin->DefaultValue = SourceID;

            bool BaseActorFound = false;
            bool BaseVectorFound = false;

            if (MyTextLocPin->HasAnyConnections())
            {
                const FEdGraphPinType& ConnectedPinType = MyTextLocPin->PinType;

                if (ConnectedPinType.PinSubCategoryObject.IsValid())
                {
                    if (ConnectedPinType.PinSubCategoryObject == TBaseStructure<FVector>::Get())
                    {
                        CompilerContext.CopyPinLinksToIntermediate(*MyTextLocPin, *PH_TextLocation);
                        BaseVectorFound = true;
                    }
                    else if (UClass* ConnectedClass = Cast<UClass>(ConnectedPinType.PinSubCategoryObject.Get()))
                    {
                        if (ConnectedClass->IsChildOf(AActor::StaticClass()) || ConnectedClass->IsChildOf(USceneComponent::StaticClass()))
                        {
                            CompilerContext.CopyPinLinksToIntermediate(*MyTextLocPin, *PH_BaseObject);
                            BaseActorFound = true;
                        }
                    }
                }

                if (ConnectedPinType.PinSubCategory == FName(TEXT("self")))
                {
                    CompilerContext.CopyPinLinksToIntermediate(*MyTextLocPin, *PH_BaseObject);
                    BaseActorFound = true;
                }
            }

            if (!BaseVectorFound)
            {
                FVector TextLocationDefault;
                TextLocationDefault.InitFromString(GetSavedDefaultValue(MyTextLocPin));
                PH_TextLocation->DefaultValue = FString::Printf(TEXT("%f, %f, %f"), TextLocationDefault.X, TextLocationDefault.Y, TextLocationDefault.Z);
            }

            TArray<EPinType> Temp_PinTypes;
            TArray<FString>  Temp_LiteralStrings;

            Temp_PinTypes.SetNum(DataPins.Num());
            Temp_LiteralStrings.SetNum(DataPins.Num());

            int32 ValueIndex = 0;

            for (int32 i = 0; i < DataPins.Num(); i++)
            {
                UEdGraphPin* DataPin = DataPins[i];
                bool bConnected = DataPin->HasAnyConnections();

                if (bConnected)
                {
                    if (DataPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Object)
                    {
                        Temp_PinTypes[i] = EPinType::ObjectProperty;
                        Temp_LiteralStrings[i] = TEXT("null");
                    }
                    else
                    {
                        Temp_PinTypes[i] = EPinType::OtherProperty;
                        Temp_LiteralStrings[i] = TEXT("null");
                    }

                    if (ValueIndex < MaxValues)
                    {
                        FString ValuePinName = FString::Printf(TEXT("Value%d"), ValueIndex + 1);
                        UEdGraphPin* PH_ValuePin = PrintHelperNode->FindPinChecked(*ValuePinName);

                        PH_ValuePin->PinType = DataPin->PinType;
                        CompilerContext.CopyPinLinksToIntermediate(*DataPin, *PH_ValuePin);

                        ValueIndex++;
                    }
                    else
                    {
                        CompilerContext.MessageLog.Error(*FString::Printf(TEXT("Too many connected pins for function %s"), *FunctionName.ToString()));
                    }
                }
                else
                {
                    Temp_PinTypes[i] = EPinType::UnConnected;
                    Temp_LiteralStrings[i] = GetSavedDefaultValue(DataPin);
                }
            }

            while (ValueIndex < MaxValues)
            {
                FString ValuePinName = FString::Printf(TEXT("Value%d"), ValueIndex + 1);
                UEdGraphPin* PH_ValuePin = PrintHelperNode->FindPinChecked(*ValuePinName);

                PH_ValuePin->PinType.PinCategory = UEdGraphSchema_K2::PC_String;
                PH_ValuePin->DefaultValue = TEXT("");

                Temp_PinTypes.Add(EPinType::UnHandled);
                Temp_LiteralStrings.Add(TEXT(""));

                ValueIndex++;
            }

            if (!GetSavedDefaultValue(ThisExecIn).IsEmpty())
            {
                Temp_PinTypes.Insert(EPinType::UnConnected, 0);
                Temp_LiteralStrings.Insert(FString::Printf(TEXT("[%s] "), *GetSavedDefaultValue(ThisExecIn)), 0);
            }

            UEdGraphPin* PH_PinTypesArray = PrintHelperNode->FindPinChecked(TEXT("PinTypes"));
            UK2Node_MakeArray* MakePinTypesNode = CompilerContext.SpawnIntermediateNode<UK2Node_MakeArray>(this, SourceGraph);
            MakePinTypesNode->NumInputs = Temp_PinTypes.Num();
            MakePinTypesNode->AllocateDefaultPins();

            UEdGraphPin* OutPinTypesPin = MakePinTypesNode->GetOutputPin();
            OutPinTypesPin->PinType.PinCategory = UEdGraphSchema_K2::PC_Byte;
            OutPinTypesPin->PinType.PinSubCategoryObject = StaticEnum<EPinType>();

            for (int32 i = 0; i < Temp_PinTypes.Num(); i++)
            {
                UEdGraphPin* ElemPin = MakePinTypesNode->FindPinChecked(*FString::Printf(TEXT("[%d]"), i));
                ElemPin->PinType.PinCategory = UEdGraphSchema_K2::PC_Byte;
                ElemPin->PinType.PinSubCategoryObject = StaticEnum<EPinType>();

                uint8 EnumValue = static_cast<uint8>(Temp_PinTypes[i]);
                UEnum* EnumType = StaticEnum<EPinType>();
                FString EnumName = EnumType->GetNameStringByValue(EnumValue);

                ElemPin->DefaultValue = EnumName;
            }

            OutPinTypesPin->MakeLinkTo(PH_PinTypesArray);


            UEdGraphPin * PH_LiteralStringsArray = PrintHelperNode->FindPinChecked(TEXT("UnConnectedValues"));
            UK2Node_MakeArray* MakeLiteralArrayNode = CompilerContext.SpawnIntermediateNode<UK2Node_MakeArray>(this, SourceGraph);
            MakeLiteralArrayNode->NumInputs = Temp_LiteralStrings.Num();
            MakeLiteralArrayNode->AllocateDefaultPins();

            UEdGraphPin* OutLiteralArrayPin = MakeLiteralArrayNode->GetOutputPin();
            OutLiteralArrayPin->PinType.PinCategory = UEdGraphSchema_K2::PC_String;

            for (int32 i = 0; i < Temp_LiteralStrings.Num(); i++)
            {
                UEdGraphPin* ElemPin = MakeLiteralArrayNode->FindPinChecked(*FString::Printf(TEXT("[%d]"), i));
                ElemPin->PinType.PinCategory = UEdGraphSchema_K2::PC_String;
                ElemPin->DefaultValue = Temp_LiteralStrings[i];
            }

            OutLiteralArrayPin->MakeLinkTo(PH_LiteralStringsArray);
                

            UEdGraphPin* PH_ExecIn = PrintHelperNode->GetExecPin();
            UEdGraphPin* PH_ThenOut = PrintHelperNode->GetThenPin();

            CompilerContext.MovePinLinksToIntermediate(*ThisExecIn, *PH_ExecIn);
            CompilerContext.MovePinLinksToIntermediate(*ThisExecOut, *PH_ThenOut);

            return PrintHelperNode;
        };


    // Process each Exec Input Pin.
    for (int32 i = 0; i < ExecInPins.Num(); i++)
    {
        UEdGraphPin* ThisExecIn = ExecInPins[i];
        UEdGraphPin* ThisThenOut = ExecOutPins[i];

        if (!ThisExecIn || !ThisThenOut || !ThisExecIn->HasAnyConnections())
        {
            continue;
        }

        // If node is inactive or if its a shipping build, spawn an empty helper to keep the execution intact
        if (!bToggleNode)
        {
            UK2Node_CallFunction* EmptyNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
            EmptyNode->FunctionReference.SetExternalMember(FName("PrintHelperEmpty"), UALS_FunctionLibrary::StaticClass());
            EmptyNode->AllocateDefaultPins();

            UEdGraphPin* EmptyExecIn = EmptyNode->GetExecPin();
            UEdGraphPin* EmptyThenOut = EmptyNode->GetThenPin();

            CompilerContext.MovePinLinksToIntermediate(*ThisExecIn, *EmptyExecIn);
            CompilerContext.MovePinLinksToIntermediate(*ThisThenOut, *EmptyThenOut);
            continue;
        }
        else
        {
            SpawnHelperNode(i, ThisExecIn, ThisThenOut);
        }
    }

    BreakAllNodeLinks();
}

UEdGraphPin* UALS_Node::GetCorrespondingThenPin(UEdGraphPin* ExecInputPin) const
{
    if (!ExecInputPin || ExecInputPin->Direction != EGPD_Input || ExecInputPin->PinType.PinCategory != UEdGraphSchema_K2::PC_Exec)
    {
        return nullptr; 
    }

    FString ExecPinName = ExecInputPin->PinName.ToString();

    FString Prefix, IndexStr;
    if (ExecInputPin->PinName.ToString().Split(TEXT(" "), nullptr, &IndexStr))
    {
        FString TargetThenName = FString::Printf(TEXT("Then %s"), *IndexStr);

        for (UEdGraphPin* Pin : Pins)
        {
            if (Pin->PinName.ToString().Equals(TargetThenName) && Pin->Direction == EGPD_Output && Pin->PinType.PinCategory == UEdGraphSchema_K2::PC_Exec)
            {
                return Pin;
            }
        }
    }

    return nullptr; 
}

TArray<UEdGraphPin*> UALS_Node::GetExecPins() const
{
    TArray<UEdGraphPin*> ExecInputPins;
    for (UEdGraphPin* Pin : Pins)
    {
        if (Pin->PinType.PinCategory == UEdGraphSchema_K2::PC_Exec)
        {
            if (Pin->Direction == EGPD_Input)
            {
                ExecInputPins.Add(Pin);
            }
        }
    }

    return ExecInputPins;
}

TArray<UEdGraphPin*> UALS_Node::GetThenPins() const
{
    TArray<UEdGraphPin*> ExecOutputPins;
    for (UEdGraphPin* Pin : Pins)
    {
        if (Pin->PinType.PinCategory == UEdGraphSchema_K2::PC_Exec)
        {
            if (Pin->Direction == EGPD_Output)
            {
                ExecOutputPins.Add(Pin);
            }
        }
    }

    return ExecOutputPins;
}

TArray<UEdGraphPin*> UALS_Node::GetDataPins() const
{
    TArray<UEdGraphPin*> DataPins;
    for (UEdGraphPin* Pin : Pins)
    {
        if (IsWildcardPin(Pin))
        {
            DataPins.Add(Pin);
        }
    }

    return DataPins;
}

bool UALS_Node::IsWildcardPin(const UEdGraphPin* Pin) const
{
    if (Pin->PinType.PinCategory != UEdGraphSchema_K2::PC_Exec && 
        !AdvancedPins.Contains(Pin->PinId) && 
        Pin->PinId != TextLocationPinId && 
        Pin->PinId != WorldContextId)
    {
        return true;
    }

    return false;
}

bool UALS_Node::IsNonContextBP() const
{
    const UBlueprint* Blueprint = GetBlueprint();
    if (!Blueprint)
    {
        return true;
    }

    if (Blueprint->BlueprintType == BPTYPE_FunctionLibrary ||
        Blueprint->BlueprintType == BPTYPE_MacroLibrary ||
        Blueprint->IsA<UAnimBlueprint>() ||
        Blueprint->IsA<UEditorUtilityBlueprint>() ||
        Blueprint->ParentClass->GetName().Contains("EditorUtility") ||
        Blueprint->IsEditorOnly() ||
        bShowWorldContext)
    {
        return true;
    }

    return false;
}

int32 UALS_Node::GetWildcardPinCount() const
{
    int32 Count = 0;

    for (UEdGraphPin* Pin : Pins)
    {
        if (Pin->PinType.PinCategory != UEdGraphSchema_K2::PC_Exec && !AdvancedPins.Contains(Pin->PinId) && Pin->PinId != TextLocationPinId)
        {
            Count++;
        }
    }

    return Count;
}

int32 UALS_Node::GetExecPinCount() const
{
    int32 Count = 0;

    for (UEdGraphPin* Pin : Pins)
    {
        if (Pin->PinType.PinCategory == UEdGraphSchema_K2::PC_Exec && Pin->Direction == EGPD_Input)
        {
            Count++;
        }
    }

    return Count;
}

int32 UALS_Node::GetWildcardPinConnectedCount() const
{
    TArray<UEdGraphPin*> DataPins;
    int32 NumConnected = 0;

    for (UEdGraphPin* Pin : Pins)
    {
        if (IsWildcardPin(Pin))
        {
            DataPins.Add(Pin);
        }
    }

    for (UEdGraphPin* DataPin : DataPins)
    {
        if (DataPin->HasAnyConnections())
        {
            NumConnected++;
        }
    }

    return NumConnected;
}

FString UALS_Node::GetSavedDefaultValue(UEdGraphPin* Pin) const
{
    return GetSavedDefaultValue(Pin->PinId);
}

FString UALS_Node::GetSavedDefaultValue(FGuid PinId) const
{
    if (const FString* DefaultValue = SavedDefaultValues.Find(PinId))
    {
        return *DefaultValue;
    }

    return "";   
}

int32 UALS_Node::GetSavedPinIndex(UEdGraphPin* Pin) const
{
    return GetSavedPinIndex(Pin->PinId);
}

int32 UALS_Node::GetSavedPinIndex(FGuid PinId) const
{
    TMap<FGuid, int32> PinLookup;

    for (int32 i = 0; i < SavedPins.Num(); i++)
    {
        PinLookup.Add(SavedPins[i].PinId, i);
    }

    if (int32* FoundIndex = PinLookup.Find(PinId))
    {
        return *FoundIndex;
    }

    return INDEX_NONE;
}

FSavedPinData UALS_Node::GetSavedPinData(UEdGraphPin* Pin) const
{
    int32 FoundIndex = GetSavedPinIndex(Pin);

    if (FoundIndex != INDEX_NONE)
    {
        return SavedPins[FoundIndex];
    }

    static FSavedPinData Dummy;
    return Dummy;
}

void UALS_Node::SetDefaultValue(UEdGraphPin* Pin)
{
    SavedDefaultValues.Add(Pin->PinId, Pin->DefaultValue);
}

void UALS_Node::SetPinDefaultsByPreset()
{
    FPrintConfig CurrentConfig = UALS_Settings::GetConfigFromPreset(PrintPreset);
    PrintColor = CurrentConfig.Color;
    PrintDuration = CurrentConfig.Duration;
    LogSeverity = CurrentConfig.LogSeverity;
    PrintMode = CurrentConfig.PrintMode;

    if (UEdGraphPin* Pin = FindPinById(DurationPinId))
    {
        Pin->DefaultValue = FString::SanitizeFloat(PrintDuration);
        SetDefaultValue(Pin);
    }

    if (UEdGraphPin* Pin = FindPinById(ColorPinId))
    {
        Pin->DefaultValue = PrintColor.ToString();
        SetDefaultValue(Pin);
    }

    if (UEdGraphPin* Pin = FindPinById(LogSeverityPinId))
    {
        UEnum* Enum = StaticEnum<ELogSeverity>();
        Pin->DefaultValue = Enum->GetValueAsString(LogSeverity);
        SetDefaultValue(Pin);
    }

    if (UEdGraphPin* Pin = FindPinById(PrintModePinId))
    {
        UEnum* Enum = StaticEnum<EPrintMode>();
        Pin->DefaultValue = Enum->GetValueAsString(PrintMode);
        SetDefaultValue(Pin);
    }
}

FString UALS_Node::GetAlphabetFromIndex(int32 Index) const
{
    if (Index < 1 || Index > 26)
    {
        return TEXT("");
    }

    TCHAR Letter = 'A' + (Index - 1);
    return FString(1, &Letter);
}

FText UALS_Node::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
    return FText::FromString("Print String (ALS)");
}

FText UALS_Node::GetTooltipText() const
{
    return FText::FromString("Enhanced Print String (ALS)");
}

FText UALS_Node::GetMenuCategory() const
{
    return FText::FromString(TEXT("Development|AdvancedLoggingSystem"));
}

FText UALS_Node::GetKeywords() const
{
    return FText::FromString("Pr Pri Print");
}

FLinearColor UALS_Node::GetNodeTitleColor() const
{
    if (bToggleNode)
    {
        if (bPrintToWorld)
        {
            return UALS_Settings::Get()->Node3DColor;
        }

        if (LogSeverity == ELogSeverity::Warning)
        {
            return UALS_Settings::Get()->NodeWarningColor;
        }

        if (LogSeverity == ELogSeverity::Error)
        {
            return UALS_Settings::Get()->NodeErrorColor;
        }

        return UALS_Settings::Get()->NodeInfoColor;
    }

    return UALS_Settings::Get()->NodeActiveTitleColor;
}

FLinearColor UALS_Node::GetNodeBodyTintColor() const
{
    if (bToggleNode)
    {
        return UALS_Settings::Get()->NodeActiveBodyColor;
    }

    return UALS_Settings::Get()->NodeInActiveBodyColor;
}

FSlateIcon UALS_Node::GetIconAndTint(FLinearColor& OutColor) const
{
    OutColor = FLinearColor::White;
    return FSlateIcon(FAppStyle::GetAppStyleSetName(), "GraphEditor.Function_16x");
}

TSharedPtr<SGraphNode> UALS_Node::CreateVisualWidget()
{
    return SNew(UALS_SGraphNode, this);
}

void UALS_Node::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
    UClass* ActionKey = GetClass();
    if (ActionRegistrar.IsOpenForRegistration(ActionKey))
    {
        UBlueprintNodeSpawner* Spawner = UBlueprintNodeSpawner::Create(GetClass());
        ActionRegistrar.AddBlueprintAction(ActionKey, Spawner);
    }
}
