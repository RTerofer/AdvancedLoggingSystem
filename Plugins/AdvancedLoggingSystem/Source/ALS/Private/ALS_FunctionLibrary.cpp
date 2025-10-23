//Copyright © 2025 RTerofer. All Rights Reserved.

#include "ALS_FunctionLibrary.h"
#include "ALS_Subsystem.h"
#include "ALS_FileLog.h"
#include "ALS_PropertyUMG.h"
#include "ALS_LogsUMG.h"
#include "ALS_Settings.h"
#include "ALS_Globals.h"
#include "Containers/StringFwd.h"
#include "UObject/Stack.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Engine/GameInstance.h"
#include "Kismet/KismetSystemLibrary.h"

void UALS_FunctionLibrary::PrintHelperEmpty()
{
    // Do nothing when the node is inactive
}

// Helper Functions
DEFINE_FUNCTION(UALS_FunctionLibrary::execPrintHelperONE)
{
    P_GET_OBJECT(UObject, WorldContext);
    P_GET_STRUCT(FLinearColor, PrintColor);
    P_GET_PROPERTY(FFloatProperty, Duration);
    P_GET_PROPERTY(FByteProperty, PrintModeByte);
    P_GET_PROPERTY(FByteProperty, LogSeverityByte);
    P_GET_UBOOL(DrawDebug);
    P_GET_STRUCT(FVector, TextLocation);
    P_GET_OBJECT(UObject, BaseObject);
    P_GET_PROPERTY(FStrProperty, SourceID);

    P_GET_TARRAY(EPinType, PinTypes);
    P_GET_TARRAY(FString, LiteralStrings);

    EPrintMode OutputMode = static_cast<EPrintMode>(PrintModeByte);
    ELogSeverity Level = static_cast<ELogSeverity>(LogSeverityByte);

    if (!WorldContext)
    {
        WorldContext = Stack.Object;
    }

    FString PrintString = ProcessProperty(Stack, PinTypes, LiteralStrings, 3);
    ProcessPrint(PrintString, PrintColor, Duration, OutputMode, Level, DrawDebug, TextLocation, BaseObject, WorldContext, SourceID);

    P_FINISH;
}

DEFINE_FUNCTION(UALS_FunctionLibrary::execPrintHelperTWO)
{
    P_GET_OBJECT(UObject, WorldContext);
    P_GET_STRUCT(FLinearColor, PrintColor);
    P_GET_PROPERTY(FFloatProperty, Duration);
    P_GET_PROPERTY(FByteProperty, PrintModeByte);
    P_GET_PROPERTY(FByteProperty, LogSeverityByte);
    P_GET_UBOOL(DrawDebug);
    P_GET_STRUCT(FVector, TextLocation);
    P_GET_OBJECT(UObject, BaseObject);
    P_GET_PROPERTY(FStrProperty, SourceID);

    P_GET_TARRAY(EPinType, PinTypes);
    P_GET_TARRAY(FString, LiteralStrings);

    EPrintMode OutputMode = static_cast<EPrintMode>(PrintModeByte);
    ELogSeverity Level = static_cast<ELogSeverity>(LogSeverityByte);

    if (!WorldContext)
    {
        WorldContext = Stack.Object;
    }

    FString PrintString = ProcessProperty(Stack, PinTypes, LiteralStrings, 5);
    ProcessPrint(PrintString, PrintColor, Duration, OutputMode, Level, DrawDebug, TextLocation, BaseObject, WorldContext, SourceID);

    P_FINISH;
}

DEFINE_FUNCTION(UALS_FunctionLibrary::execPrintHelperTHREE)
{
    P_GET_OBJECT(UObject, WorldContext);
    P_GET_STRUCT(FLinearColor, PrintColor);
    P_GET_PROPERTY(FFloatProperty, Duration);
    P_GET_PROPERTY(FByteProperty, PrintModeByte);
    P_GET_PROPERTY(FByteProperty, LogSeverityByte);
    P_GET_UBOOL(DrawDebug);
    P_GET_STRUCT(FVector, TextLocation);
    P_GET_OBJECT(UObject, BaseObject);
    P_GET_PROPERTY(FStrProperty, SourceID);

    P_GET_TARRAY(EPinType, PinTypes);
    P_GET_TARRAY(FString, LiteralStrings);

    EPrintMode OutputMode = static_cast<EPrintMode>(PrintModeByte);
    ELogSeverity Level = static_cast<ELogSeverity>(LogSeverityByte);

    if (!WorldContext)
    {
        WorldContext = Stack.Object;
    }

    FString PrintString = ProcessProperty(Stack, PinTypes, LiteralStrings, 7);
    ProcessPrint(PrintString, PrintColor, Duration, OutputMode, Level, DrawDebug, TextLocation, BaseObject, WorldContext, SourceID);

    P_FINISH;
}

DEFINE_FUNCTION(UALS_FunctionLibrary::execPrintHelperFOUR)
{
    P_GET_OBJECT(UObject, WorldContext);
    P_GET_STRUCT(FLinearColor, PrintColor);
    P_GET_PROPERTY(FFloatProperty, Duration);
    P_GET_PROPERTY(FByteProperty, PrintModeByte);
    P_GET_PROPERTY(FByteProperty, LogSeverityByte);
    P_GET_UBOOL(DrawDebug);
    P_GET_STRUCT(FVector, TextLocation);
    P_GET_OBJECT(UObject, BaseObject);
    P_GET_PROPERTY(FStrProperty, SourceID);

    P_GET_TARRAY(EPinType, PinTypes);
    P_GET_TARRAY(FString, LiteralStrings);

    EPrintMode OutputMode = static_cast<EPrintMode>(PrintModeByte);
    ELogSeverity Level = static_cast<ELogSeverity>(LogSeverityByte);

    if (!WorldContext)
    {
        WorldContext = Stack.Object;
    }

    FString PrintString = ProcessProperty(Stack, PinTypes, LiteralStrings, 10);
    ProcessPrint(PrintString, PrintColor, Duration, OutputMode, Level, DrawDebug, TextLocation, BaseObject, WorldContext, SourceID);

    P_FINISH;
}


// Helper Processing
FString UALS_FunctionLibrary::ProcessProperty(FFrame& Stack, TArray<EPinType>& PinTypes, TArray<FString>& LiteralStrings, int32 MaxValue)
{
    TArray<FString> CollectedStrings;
    CollectedStrings.Reserve(PinTypes.Num());

    int32 ProcessedStack = 0;

    FStringBuilderBase OutValue;

    for (int32 i = 0; i < PinTypes.Num(); i++)
    {
        switch (PinTypes[i])
        {
        case EPinType::OtherProperty:
        {
            if (ProcessedStack >= MaxValue)
            {
                return TEXT("Trying to access invalid number of stack");
            }

            Stack.StepCompiledIn<FProperty>(nullptr);

            FProperty* ValueProperty = Stack.MostRecentProperty;
            const void* ValuePtr = Stack.MostRecentPropertyAddress;

            if (!ValueProperty || !ValuePtr)
            {
                UE_LOG(LogTemp, Error, TEXT("Invalid Stack Address or Property"));
                OutValue.Appendf(TEXT("Invalid Stack Address or Property"));
                continue;
            }

            UALS_Globals::ConvertToString_Property(ValueProperty, ValuePtr, OutValue);

            ProcessedStack++;
            break;
        }

        case EPinType::ObjectProperty:
        {
            if (ProcessedStack >= MaxValue)
            {
                return TEXT("Trying to access invalid number of stack");
            }

            P_GET_OBJECT(UObject, ConnectedObject);

            if (ConnectedObject)
            {
                OutValue.Append(UKismetSystemLibrary::GetDisplayName(ConnectedObject));
            }
            else
            {
                OutValue.Append(TEXT("null_object"));
            }

            ProcessedStack++;
            break;
        }

        case EPinType::UnHandled:
        {
            if (ProcessedStack >= MaxValue)
            {
                return TEXT("Trying to access invalid number of stack");
            }

            P_GET_PROPERTY(FStrProperty, DummyValue);

            ProcessedStack++;
            break;
        }

        case EPinType::UnConnected:
        {
            if (LiteralStrings.IsValidIndex(i))
            {
                OutValue.Append(LiteralStrings[i]);
            }
            else
            {
                OutValue.Append(TEXT(""));
            }
            break;
        }

        default:
        {
            UE_LOG(LogTemp, Warning, TEXT("Unknown EPinType in PrintHelper"));
            break;
        }
        }
    }

    return OutValue.ToString();
}

void UALS_FunctionLibrary::ProcessPrint(const FString& PrintString, const FLinearColor& PrintColor, const float& Duration, const EPrintMode& PrintMode, const ELogSeverity& LogSeverity, const bool& DrawDebug, const FVector& TextLocation, const UObject* BaseObject, const UObject* Context, const FString& SourceID)
{
    FPrintConfig PrintConfig = FPrintConfig(PrintColor.ToFColor(true), Duration, LogSeverity, PrintMode);

    if (DrawDebug)
    {
        UALS_Globals::DrawALS(
            PrintString,
            BaseObject,
            TextLocation,
            PrintConfig,
            Context,
            SourceID
        );
    }
    else
    {
        UALS_Globals::PrintALS(
            PrintString,
            PrintConfig,
            Context,
            SourceID
        );
    }
}


// Exposed Functions
APlayerController* UALS_FunctionLibrary::GetPlayerControllerFromContext(const UObject* WorldContextObject)
{
    if (WorldContextObject)
    {
        if (const APlayerController* PC = Cast<APlayerController>(WorldContextObject))
        {
            return const_cast<APlayerController*>(PC);
        }

        if (const APawn* Pawn = Cast<APawn>(WorldContextObject))
        {
            if (APlayerController* Controller = Cast<APlayerController>(Pawn->GetController()))
            {
                return Controller;
            }
        }

        if (const UUserWidget* Widget = Cast<UUserWidget>(WorldContextObject))
        {
            if (APlayerController* OwningPC = Widget->GetOwningPlayer())
            {
                return OwningPC;
            }
        }

        if (const AActor* Actor = Cast<AActor>(WorldContextObject))
        {
            if (APlayerController* OwnerPC = Cast<APlayerController>(Actor->GetOwner()))
            {
                return OwnerPC;
            }

            if (APawn* OwnerPawn = Cast<APawn>(Actor->GetOwner()))
            {
                if (APlayerController* Controller = Cast<APlayerController>(OwnerPawn->GetController()))
                {
                    return Controller;
                }
            }

            if (APlayerController* InstigatorPC = Cast<APlayerController>(Actor->GetInstigatorController()))
            {
                return InstigatorPC;
            }
        }
    }

    UWorld* World = nullptr;
    if (WorldContextObject)
    {
        if (const UGameInstance* GameInstance = Cast<UGameInstance>(WorldContextObject))
        {
            return GameInstance->GetFirstLocalPlayerController();
        }

        World = WorldContextObject->GetWorld();
    }

    if (!World)
    {
        World = GEngine->GetCurrentPlayWorld();
    }

    return World ? World->GetGameInstance()->GetFirstLocalPlayerController() : nullptr;
}

void UALS_FunctionLibrary::OpenLogsViewer(const UObject* WorldContextObject, APlayerController* PlayerController)
{
    if (!PlayerController)
    {
       PlayerController = GetPlayerControllerFromContext(WorldContextObject);    
    }

    if (!PlayerController)
    {
        UE_LOG(LogBlueprint, Warning, TEXT("No Player Controller found. Failed to Open LogsViewer"));
        return;
    }

    auto* Store = PlayerController->GetWorld()->GetSubsystem<UALSStorageSubsystem>();

    if (UALS_PropertyUMG* Prop = Store->GetPropertyWidget(PlayerController))
    {
        if (Prop->IsInViewport())
        {
            Prop->PropertyWindow->SetIsExpanded_Animated(false);
        }
    }

    if (UALS_LogsUMG* Logs = Store->GetLogWidget(PlayerController))
    {
        if (Logs->IsInViewport())
        {
            CloseLogsViewer(WorldContextObject, PlayerController);
            return;
        }
    }

    TSubclassOf<UUserWidget> LogsClass = UALS_Settings::Get()->LogViewerWidget.LoadSynchronous();
    if (!LogsClass) return;

    UALS_LogsUMG* LogsWidget = CreateWidget<UALS_LogsUMG>(PlayerController, LogsClass);
    LogsWidget->AddToViewport(9999);

    Store->SetLogWidget(PlayerController, LogsWidget);
    ShowMouseCursor(WorldContextObject, PlayerController);
}

void UALS_FunctionLibrary::CloseLogsViewer(const UObject* WorldContextObject, APlayerController* PlayerController)
{
    if (!PlayerController)
    {
        PlayerController = GetPlayerControllerFromContext(WorldContextObject);
    }

    if (!PlayerController)
    {
        UE_LOG(LogBlueprint, Warning, TEXT("No Player Controller found. Failed to Close LogsViewer"));
        return;
    }

    auto* Store = PlayerController->GetWorld()->GetSubsystem<UALSStorageSubsystem>();

    if (UALS_LogsUMG* Logs = Store->GetLogWidget(PlayerController))
    {
        if (Logs->IsInViewport())
        {
            Logs->RemoveFromParent();
            Store->RemoveLogWidget(PlayerController);
        }

        bool CanHideCursor = true;
        if (UALS_PropertyUMG* Prop = Store->GetPropertyWidget(PlayerController))
        {
            if (Prop->IsInViewport())
            {
                CanHideCursor = !Prop->PropertyWindow->GetIsExpanded();
            }
        }

        if (CanHideCursor)
        {
            HideMouseCursor(WorldContextObject, PlayerController);
        }
    }
}

void UALS_FunctionLibrary::OpenPropertyInspector(const UObject* WorldContextObject, APlayerController* PlayerController)
{
    if (!PlayerController)
    {
        PlayerController = GetPlayerControllerFromContext(WorldContextObject);
    }

    if (!PlayerController)
    {
        UE_LOG(LogBlueprint, Warning, TEXT("No Player Controller found. Failed to open PropertyInspector"));
        return;
    }

    auto* Store = PlayerController->GetWorld()->GetSubsystem<UALSStorageSubsystem>();
    if (Store->GetLogWidget(PlayerController)) return;

    if (UALS_PropertyUMG* Prop = Store->GetPropertyWidget(PlayerController))
    {
        if (Prop->IsInViewport())
        {
            UExpandableArea* PropertyExpand = Prop->PropertyWindow;

            if (PropertyExpand)
            {
                PropertyExpand->SetIsExpanded_Animated(!PropertyExpand->GetIsExpanded());
            }
        }

        return;
    }

    TSubclassOf<UUserWidget> PropertyClass = UALS_Settings::Get()->PropertyInspectorWidget.LoadSynchronous();
    if (!PropertyClass) return;

    UALS_PropertyUMG* PropertyWidget = CreateWidget<UALS_PropertyUMG>(PlayerController, PropertyClass);
    PropertyWidget->AddToViewport(9998);

    Store->SetPropertyWidget(PlayerController, PropertyWidget);
    ShowMouseCursor(WorldContextObject, PlayerController);
}

void UALS_FunctionLibrary::ClosePropertyInspector(const UObject* WorldContextObject, APlayerController* PlayerController)
{
    if (!PlayerController)
    {
        PlayerController = GetPlayerControllerFromContext(WorldContextObject);
    }

    if (!PlayerController) 
    {
        UE_LOG(LogBlueprint, Warning, TEXT("No Player Controller found. Failed to Close PropertyInspector"));
        return;
    }

    auto* Store = PlayerController->GetWorld()->GetSubsystem<UALSStorageSubsystem>();
    if (UALS_PropertyUMG* Prop = Store->GetPropertyWidget(PlayerController))
    {
        if (Prop->IsInViewport())
        {
            Prop->RemoveFromParent();
            Store->RemovePropertyWidget(PlayerController);
        }

        bool CanHideCursor = true;
        if (!Store->GetLogWidget(PlayerController))
        {
            HideMouseCursor(WorldContextObject, PlayerController);
        }
    }
}

void UALS_FunctionLibrary::ShowMouseCursor(const UObject* WorldContextObject, APlayerController* PlayerController)
{
    if (!UALS_Settings::Get()->bOverrideInputMode) return;

#if PLATFORM_WINDOWS|PLATFORM_LINUX|PLATFORM_MAC
    if (!PlayerController)
    {
        PlayerController = GetPlayerControllerFromContext(WorldContextObject);
    }

    if (!PlayerController)
    {
        UE_LOG(LogBlueprint, Warning, TEXT("No Player Controller found. Failed to Show cursor"));
        return;
    }

    if (!PlayerController->bShowMouseCursor || !PlayerController->IsLookInputIgnored())
    {
        PlayerController->bShowMouseCursor = true;
        PlayerController->SetIgnoreLookInput(true);
        PlayerController->SetInputMode(FInputModeGameAndUI());
    }
#endif
}

void UALS_FunctionLibrary::HideMouseCursor(const UObject* WorldContextObject, APlayerController* PlayerController)
{
    if (!UALS_Settings::Get()->bOverrideInputMode) return;

#if PLATFORM_WINDOWS|PLATFORM_LINUX|PLATFORM_MAC
    if (!PlayerController)
    {
        PlayerController = GetPlayerControllerFromContext(WorldContextObject);
    }

    if (!PlayerController)
    {
        UE_LOG(LogBlueprint, Warning, TEXT("No Player Controller found. Failed to Hide cursor"));
        return;
    }

    if (PlayerController->bShowMouseCursor || PlayerController->IsLookInputIgnored())
    {
        PlayerController->bShowMouseCursor = false;
        PlayerController->SetIgnoreLookInput(false);
        PlayerController->SetInputMode(FInputModeGameOnly());
    }
#endif
}

bool UALS_FunctionLibrary::LogMessageToFile(const UObject* WorldContextObject, const FString& SourceID, const FString& Message, ELogSeverity LogSeverity)
{
    if (!WorldContextObject)
    {
        UE_LOG(LogTemp, Error, TEXT("No WorldContextObject Provided. File logging cannot be created."));
        return false;
    }

    return UALS_FileLog::CreateMessageLog(WorldContextObject, SourceID, Message, LogSeverity);
}

DEFINE_FUNCTION(UALS_FunctionLibrary::execConvertToString)
{
    Stack.MostRecentProperty = nullptr;
    Stack.MostRecentPropertyAddress = nullptr;

    Stack.StepCompiledIn<FProperty>(nullptr);

    FProperty* ValueProperty = Stack.MostRecentProperty;
    const void* ValuePtr = Stack.MostRecentPropertyAddress;
    P_FINISH;

    if (!ValueProperty || !ValuePtr)
    {
        *static_cast<FString*>(RESULT_PARAM) = TEXT("Invalid Property. Self cannot be accessed, Try Self->GetDisplayName");
        return;
    }

    FStringBuilderBase OutValue;
    OutValue.Appendf(TEXT(""));

    UALS_Globals::ConvertToString_Property(ValueProperty, ValuePtr, OutValue);
    *static_cast<FString*>(RESULT_PARAM) = OutValue.ToString();
}