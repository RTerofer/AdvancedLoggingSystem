//Copyright © 2025 RTerofer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "ALS_Definitions.generated.h"

inline FLogCategory<ELogVerbosity::Log, ELogVerbosity::All> LogALS(TEXT("LogALS"));

UENUM(BlueprintType, meta = (Category = "AdvancedLoggingSystem"))
enum class ELogSeverity : uint8
{
    Info    UMETA(DisplayName = "Info"),
    Warning UMETA(DisplayName = "Warning"),
    Error   UMETA(DisplayName = "Error")
};

UENUM(BlueprintType, meta = (Category = "AdvancedLoggingSystem"))
enum class EPrintMode : uint8
{
    ScreenOnly     UMETA(DisplayName = "ScreenOnly"),
    LogOnly        UMETA(DisplayName = "LogOnly"),
    ScreenAndLog   UMETA(DisplayName = "ScreenAndLog")
};

UENUM(BlueprintType, meta = (Category = "AdvancedLoggingSystem"))
enum class EPrintPreset : uint8
{
    PrintInfo       UMETA(DisplayName = "Print Info"),
    PrintWarn       UMETA(DisplayName = "Print Warn"),
    PrintError      UMETA(DisplayName = "Print Error"),
    LogInfo         UMETA(DisplayName = "Log Info"),
    LogWarn         UMETA(DisplayName = "Log Warn"),
    LogError        UMETA(DisplayName = "Log Error"),
    Print3D         UMETA(DisplayName = "Print 3D")
};

UENUM()
enum class EPinType : uint8
{
    UnConnected,
    UnHandled,
    ObjectProperty,
    OtherProperty,
};

USTRUCT(BlueprintType, meta = (Category = "AdvancedLoggingSystem"))
struct FPrintConfig
{
    GENERATED_BODY()

    // The color used for this print configuration
    UPROPERTY(EditDefaultsOnly, meta = (DisplayName = "Color", Category = "ALS Config"))
    FColor Color;

    // Duration (in seconds) the message will remain visible on screen
    UPROPERTY(EditDefaultsOnly, meta = (DisplayName = "Duration", Category = "ALS Config"))
    float Duration;

    // Duration (in seconds) the message will remain visible on screen
    UPROPERTY(EditDefaultsOnly, meta = (DisplayName = "Key", Category = "ALS Config"))
    FName Key;

    // The log level: Info, Warning, or Error
    UPROPERTY(EditDefaultsOnly, meta = (DisplayName = "Log Level", Category = "ALS Config"))
    ELogSeverity LogSeverity;

    // Print output type: screen only, log only, or both
    UPROPERTY(EditDefaultsOnly, meta = (DisplayName = "Print Mode", Category = "ALS Config"))
    EPrintMode PrintMode;
 
    FPrintConfig(
        FName InKey = NAME_None,
        float InDuration = 5.0f,
        FColor InColor = FColor::Green,
        ELogSeverity InLogSeverity = ELogSeverity::Info, 
        EPrintMode InPrintMode = EPrintMode::ScreenAndLog
    ) 
        : Color(InColor)
        , Duration(InDuration)
        , Key(InKey)
        , LogSeverity(InLogSeverity)
        , PrintMode(InPrintMode) 
    {}
};

USTRUCT(BlueprintType, meta = (Category = "AdvancedLoggingSystem"))
struct FLogEntries
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "ALS LogEntries")
    FString DateTime;

    UPROPERTY(BlueprintReadOnly, Category = "ALS LogEntries")
    FString Level;

    UPROPERTY(BlueprintReadOnly, Category = "ALS LogEntries")
    FString Message;

    UPROPERTY(BlueprintReadOnly, Category = "ALS LogEntries")
    FString Source;

    UPROPERTY(BlueprintReadOnly, Category = "ALS LogEntries")
    FString PeriodMessage;

    UPROPERTY()
    FDateTime StartTime;

    UPROPERTY()
    FDateTime EndTime;

    UPROPERTY()
    int32 Count;

    UPROPERTY()
    uint64 CycleCounter;

    FLogEntries()
        : DateTime()
        , Level()
        , Message()
        , PeriodMessage()
        , StartTime(FDateTime::MinValue())
        , EndTime(FDateTime::MinValue())
        , Count(0)
        , CycleCounter(0)
    {}

    FLogEntries(
        const FString& InLevel,
        const FString& InMessage,
        const FString& InSource,
        const FDateTime& InitialTime,
        uint64 InCycleCounter
    )
        : DateTime(InitialTime.ToFormattedString(TEXT("%d:%m:%Y %H:%M:%S")))
        , Level(InLevel)
        , Message(InMessage)
        , Source(InSource)
        , PeriodMessage(FString::Printf(TEXT("(1 time logged on %s)"), *InitialTime.ToString(TEXT("%H:%M:%S:%s"))))
        , StartTime(InitialTime)
        , EndTime(InitialTime)
        , Count(1)
        , CycleCounter(InCycleCounter)
    {}
};

USTRUCT(BlueprintType, meta = (Category = "AdvancedLoggingSystem"))
struct FContextEntries
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "ALS ContextEntries")
    FString ContextRaw;

    UPROPERTY(BlueprintReadOnly, Category = "ALS ContextEntries")
    FText ContextText;

    UPROPERTY(BlueprintReadOnly, Category = "ALS ContextEntries")
    FText NetworkText;

    FContextEntries() 
        : ContextRaw(TEXT(""))
        , ContextText(FText::GetEmpty())
        , NetworkText(FText::GetEmpty())
	{}

    FContextEntries(FString InContextRaw, FText InContextText, FText InNetwork) 
        : ContextRaw(InContextRaw)
        , ContextText(InContextText)
        , NetworkText(InNetwork)
	{}
};
