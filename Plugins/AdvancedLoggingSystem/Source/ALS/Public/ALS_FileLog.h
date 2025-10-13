//Copyright © 2025 RTerofer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ALS_Settings.h"
#include "Engine/Engine.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/App.h"

class ALS_API UALS_FileLog
{
private:
    static inline FString CurrentSessionTime = TEXT("");

    static void InitializeSessionTime();

    static FString GetSessionTime();

    static FString EscapeForLog(const FString& InText);

    static FString UnEscapeForWidget(const FString& InText);

public:
    static void OnStartGameInstance(UGameInstance* GameInstance);

    static bool CreateMessageLog(
        const UObject* Context,
        const FString& CallerName,
        const FString& SourceID,
        const FString& LogSeverity,
        const FString& Message
    );

    static bool CreateMessageLog(
        const UObject* Context,
        const FString& SourceID,
        const FString& Message,
        const ELogSeverity& LogSeverity
    );

    static bool CreateSessionLog(const UWorld* World);

    static void RotateOlderLogs();

    static FString GetCurrentInstance(const UWorld* World);

    static bool IsFileBigger(const FString& LogFilePath, int32& OutFileSize);
};
