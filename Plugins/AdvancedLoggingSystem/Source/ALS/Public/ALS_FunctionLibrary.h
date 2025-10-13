//Copyright © 2025 RTerofer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UObject/UnrealType.h"
#include "ALS_Definitions.h"
#include "ALS_FunctionLibrary.generated.h"

UCLASS()
class ALS_API UALS_FunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(
        BlueprintCallable, 
        CustomThunk, 
        meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, BlueprintInternalUseOnly = "true", DevelopmentOnly,
        CustomStructureParam = "Value1,Value2,Value3"), Category = "AdvancedLoggingSystem|Intermediate")
    static void PrintHelperONE(
        const UObject* WorldContextObject,
        const FLinearColor Color,
        const float Duration,
        const EPrintMode PrintMode,
        const ELogSeverity LogSeverity,
        const bool DrawDebug,
        const FVector TextLocation,
        const UObject* BaseObject,
        const FString SourceID,
        const TArray<EPinType>& PinTypes,
        const TArray<FString>& UnConnectedValues,
        const int32 Value1,
        const int32 Value2,
        const int32 Value3
    );
    DECLARE_FUNCTION(execPrintHelperONE);


    UFUNCTION(
        BlueprintCallable,
        CustomThunk,
        meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, BlueprintInternalUseOnly = "true", DevelopmentOnly,
        CustomStructureParam = "Value1,Value2,Value3,Value4,Value5"), Category = "AdvancedLoggingSystem|Intermediate")
    static void PrintHelperTWO(
        const UObject* WorldContextObject,
        const FLinearColor Color,
        const float Duration,
        const EPrintMode PrintMode,
        const ELogSeverity LogSeverity,
        const bool DrawDebug,
        const FVector TextLocation,
        const UObject* BaseObject,
        const FString SourceID,
        const TArray<EPinType>& PinTypes,
        const TArray<FString>& UnConnectedValues,
        const int32 Value1,
        const int32 Value2,
        const int32 Value3,
        const int32 Value4,
        const int32 Value5
    );
    DECLARE_FUNCTION(execPrintHelperTWO);


    UFUNCTION(
        BlueprintCallable,
        CustomThunk,
        meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, BlueprintInternalUseOnly = "true", DevelopmentOnly,
        CustomStructureParam = "Value1,Value2,Value3,Value4,Value5,Value6,Value7"), Category = "AdvancedLoggingSystem|Intermediate")
    static void PrintHelperTHREE(
        const UObject* WorldContextObject,
        const FLinearColor Color,
        const float Duration,
        const EPrintMode PrintMode,
        const ELogSeverity LogSeverity,
        const bool DrawDebug,
        const FVector TextLocation,
        const UObject* BaseObject,
        const FString SourceID,
        const TArray<EPinType>& PinTypes,
        const TArray<FString>& UnConnectedValues,
        const int32 Value1,
        const int32 Value2,
        const int32 Value3,
        const int32 Value4,
        const int32 Value5,
        const int32 Value6,
        const int32 Value7
    );
    DECLARE_FUNCTION(execPrintHelperTHREE);


    UFUNCTION(
        BlueprintCallable,
        CustomThunk,
        meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, BlueprintInternalUseOnly = "true", DevelopmentOnly,
        CustomStructureParam = "Value1,Value2,Value3,Value4,Value5,Value6,Value7,Value8,Value9,Value10"), Category = "AdvancedLoggingSystem|Intermediate")
    static void PrintHelperFOUR(
        const UObject* WorldContextObject,
        const FLinearColor Color,
        const float Duration,
        const EPrintMode PrintMode,
        const ELogSeverity LogSeverity,
        const bool DrawDebug,
        const FVector TextLocation,
        const UObject* BaseObject,
        const FString SourceID,
        const TArray<EPinType>& PinTypes,
        const TArray<FString>& UnConnectedValues,
        const int32 Value1,
        const int32 Value2,
        const int32 Value3,
        const int32 Value4,
        const int32 Value5,
        const int32 Value6,
        const int32 Value7,
        const int32 Value8,
        const int32 Value9,
        const int32 Value10
    );
    DECLARE_FUNCTION(execPrintHelperFOUR);


    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", DevelopmentOnly), Category = "AdvancedLoggingSystem|Intermediate")
    static void PrintHelperEmpty();

private:
    static FString ProcessProperty(
        FFrame& Stack,
        TArray<EPinType>& PinTypes,
        TArray<FString>& LiteralStrings,
        int32 MaxValue
    );

    static void ProcessPrint(
        const FString& PrintString,
        const FLinearColor& PrintColor,
        const float& Duration,
        const EPrintMode& PrintMode,
        const ELogSeverity& LogSeverity,
        const bool& DrawDebug,
        const FVector& TextLocation,
        const UObject* BaseObject,
        const UObject* Context,
        const FString& SourceID
    );


//-------------------------------------------------------------------------------------------------------------------------------------------------
// Exposed Functions

public:


     /**
     Writes a message to the ALS log file, viewable via the LogsViewer. Returns if successful    
     @param WorldContextObject - Used to resolve the current world and identify the correct log instance.
     @param SourceID           - A unique identifier for the message source. This helps group similar messages from different execution paths.
     @param LogSeverity        - The severity level of the message (Info, Warning, Error).
     @param Message            - The message content to log.
     */
    UFUNCTION(BlueprintCallable,
        meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext), Category = "AdvancedLoggingSystem|LogsViewer",
        DisplayName = "Log Message To File")
    static bool LogMessageToFile(const UObject* WorldContextObject, const FString& SourceID, const FString& Message,ELogSeverity LogSeverity);

    // Converts any datatype and returns a readable string.
    UFUNCTION(BlueprintCallable, BlueprintPure, CustomThunk, meta = (CustomStructureParam = "Value"), Category = "AdvancedLoggingSystem|Convertor",
        DisplayName = "Convert To String")
    static FString ConvertToString(const int32 Value);
    DECLARE_FUNCTION(execConvertToString);

    // Opens logs viewer
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext),
        Category = "AdvancedLoggingSystem|LogsViewer", DisplayName = "Open Logs Viewer")
    static void OpenLogsViewer(const UObject* WorldContextObject, APlayerController* PlayerController);

    /// Closes logs viewer
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext),
        Category = "AdvancedLoggingSystem|LogsViewer", DisplayName = "Close Logs Viewer")
    static void CloseLogsViewer(const UObject* WorldContextObject, APlayerController* PlayerController);


    // Opens property inspector
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext),
        Category = "AdvancedLoggingSystem|PropertyInspector", DisplayName = "Open Property Inspector")
    static void OpenPropertyInspector(const UObject* WorldContextObject, APlayerController* PlayerController);

    // Closes property inspector
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext),
        Category = "AdvancedLoggingSystem|PropertyInspector", DisplayName = "Close Property Inspector")
    static void ClosePropertyInspector(const UObject* WorldContextObject, APlayerController* PlayerController);


    // Show Mouse Cursor for the Controller
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext),
        Category = "AdvancedLoggingSystem|PlayerController", DisplayName = "Show Mouse Cursor")
    static void ShowMouseCursor(const UObject* WorldContextObject, APlayerController* PlayerController);

    // Restore Mouse Cursor for the Controller
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext),
        Category = "AdvancedLoggingSystem|PlayerController", DisplayName = "Hide Mouse Cursor")
    static void HideMouseCursor(const UObject* WorldContextObject, APlayerController* PlayerController);

    // Get Controller from Context. Context will be automatically hooked up if called in UObjects.
    UFUNCTION(BlueprintCallable, BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext),
        Category = "AdvancedLoggingSystem|PlayerController", DisplayName = "Get Player Controller From Context")
    static APlayerController* GetPlayerControllerFromContext(const UObject* WorldContextObject);
};