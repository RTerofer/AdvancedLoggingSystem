//Copyright © 2025 RTerofer. All Rights Reserved.

#include "ALS_FileLog.h"
#include "ALS_Macro.h"
#include "HAL/PlatformFileManager.h"
#include "Engine/GameInstance.h"


void UALS_FileLog::OnStartGameInstance(UGameInstance* GameInstance)
{
    UWorld* World = GameInstance->GetWorld();

    if (World && World->IsGameWorld())
    {
        InitializeSessionTime();

        if (!UALS_Settings::Get()->bCreateSessionOnlyIfLogged)
        {
            CreateSessionLog(World);
        }
    }
}

bool UALS_FileLog::CreateMessageLog(const UObject* Context, const FString& CallerName, const FString& SourceID, const FString& Level, const FString& Message)
{
    bool bAllowFileLog = UALS_Settings::Get()->IsFileLoggingAllowed();
    if (!bAllowFileLog || !Context || !Context->GetWorld()) return false;

    FString LogFilePath = UALS_Settings::Get()->FileLogRootDir.Path / GetCurrentInstance(Context->GetWorld()) + TEXT(".log");

    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    if (PlatformFile.IsReadOnly(*LogFilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("Directory not writable: %s"), *LogFilePath);
        return false;
    }

    uint64 CycleCounter = FPlatformTime::Cycles64();
    FString SessionTime = GetSessionTime();
    FString DateTimeStr = FDateTime::Now()
        .ToString(TEXT("%Y.%m.%d-%H.%M.%S.%s"));
    FString SafeMessage = EscapeForLog(Message);

    FString LogEntry = FString::Printf(
        TEXT("%llu-|ALS|-%s-|ALS|-%s-|ALS|-%s-|ALS|-%s-|ALS|-%s-|ALS|-%s\n"),
        CycleCounter,
        *DateTimeStr,
        *SessionTime,
        *CallerName,
        *SourceID,
        *Level,
        *SafeMessage
    );

    const bool FileLogSuccess = FFileHelper::SaveStringToFile(
        LogEntry,
        *LogFilePath,
        FFileHelper::EEncodingOptions::AutoDetect,
        &IFileManager::Get(),
        FILEWRITE_Append
    );

    return FileLogSuccess;
}

bool UALS_FileLog::CreateMessageLog(const UObject* Context, const FString& SourceID, const FString& Message, const ELogSeverity& LogSeverity)
{
    FString Caller;
    FString Network;
    UALS_Globals::GetContextAndNetwork(Context, Caller, Network);

    UEnum* LevelType = StaticEnum<ELogSeverity>();
    uint8 LevelValue = static_cast<uint8>(LogSeverity);
    FString Severity = LevelType->GetNameStringByValue(LevelValue);

    return UALS_FileLog::CreateMessageLog(Context, Caller, SourceID, Severity, Message);
}

bool UALS_FileLog::CreateSessionLog(const UWorld* World)
{
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("No Valid World Found: Can't create Session log. Provide a valid World Context"));
        return false;
    }

    FString LogFilePath = UALS_Settings::Get()->FileLogRootDir.Path / GetCurrentInstance(World) + TEXT(".log");

    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    if (PlatformFile.IsReadOnly(*LogFilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("Directory not writable: %s"), *LogFilePath);
        return false;
    }

    uint64 CycleCounter = FPlatformTime::Cycles64();
    FString SessionTime = GetSessionTime();
    FString DateTimeStr = FDateTime::Now().ToString(TEXT("%Y.%m.%d-%H.%M.%S.%s"));

    FString LogEntry = FString::Printf(
        TEXT("%llu-|ALS|-%s-|ALS|-%s-|ALS|-[SESSION CREATED]-|ALS|-[Created a safe Play Session]\n"),
        CycleCounter,
        *DateTimeStr,
        *SessionTime
    );

    const bool FileLogSuccess = FFileHelper::SaveStringToFile(
        LogEntry,
        *LogFilePath,
        FFileHelper::EEncodingOptions::AutoDetect,
        &IFileManager::Get(),
        FILEWRITE_Append
    );

    if (!FileLogSuccess)
    {
        UE_LOG(LogTemp, Error, TEXT("File to create file logging. Check if the file has write permissions"));
    }

    return FileLogSuccess;
}

FString UALS_FileLog::GetCurrentInstance(const UWorld* World)
{
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("No Valid World Found: GetCurrentInstance()"));
        return FString::Printf(TEXT("%s_NoWorldContext"), FApp::GetProjectName());
    }

    FString OutInstanceName = FString::Printf(TEXT("%s_Standalone (1)"), FApp::GetProjectName());
    int32 InstanceNumber = 0;

    for (const FWorldContext& Context : GEngine->GetWorldContexts())
    {
        if (Context.World() == World)
        {
            #if WITH_EDITOR
                InstanceNumber = Context.PIEInstance;
            #else
                static uint32 PIDNum = FPlatformProcess::GetCurrentProcessId() & 0xFFFFFFFF;
                InstanceNumber = PIDNum;
            #endif

            switch (World->GetNetMode())
            {
            case NM_Standalone:
                OutInstanceName = FString::Printf(TEXT("%s_Standalone (%d)"), FApp::GetProjectName(), InstanceNumber);
                break;
            case NM_ListenServer:
                OutInstanceName = FString::Printf(TEXT("%s_Server (%d)"), FApp::GetProjectName(), InstanceNumber);
                break;
            case NM_Client:
                OutInstanceName = FString::Printf(TEXT("%s_Client (%d)"), FApp::GetProjectName(), InstanceNumber);
                break;
            default:
                OutInstanceName = TEXT("%s_Unknown");
                break;
            }

            break;
        }
    }

    return OutInstanceName;
}

bool UALS_FileLog::IsFileBigger(const FString& LogFilePath, int32& OutFileSize)
{
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    int64 FileSizeInBytes = PlatformFile.FileSize(*LogFilePath);
    int64 MaxSizeInBytes = int64(UALS_Settings::Get()->MaxFileSizeToParse) * 1024 * 1024;

    const double FileSizeMiB = static_cast<double>(FileSizeInBytes) / (1024.0 * 1024.0);
    OutFileSize = FMath::RoundToInt(FileSizeMiB);

    if (FileSizeInBytes > MaxSizeInBytes)
    {
        return true;
    }

    return false;
}

void UALS_FileLog::InitializeSessionTime()
{
    FDateTime Now = FDateTime::Now();
    CurrentSessionTime = Now.ToString();
}

FString UALS_FileLog::GetSessionTime()
{
    if (CurrentSessionTime.IsEmpty())
    {
        InitializeSessionTime();
    }
    return CurrentSessionTime;
}

void UALS_FileLog::RotateOlderLogs()
{
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

    TArray<FString> InstanceFiles;
    FString LogDir = UALS_Settings::Get()->FileLogRootDir.Path;
    IFileManager::Get().FindFiles(InstanceFiles, *LogDir, TEXT(".log"));

    for (FString& FoundFile : InstanceFiles)
    {
        FString FoundFilePath = UALS_Settings::Get()->FileLogRootDir.Path / FoundFile;

        FDateTime FileModTime = PlatformFile.GetTimeStamp(*FoundFilePath);
        FDateTime Now = FDateTime::Now();

        double AgeInDays = (Now - FileModTime).GetTotalDays();
        int32 MaxAgeInDays = UALS_Settings::Get()->MaxFileLogAge;

        int64 FileSizeInBytes = PlatformFile.FileSize(*FoundFilePath);
        int64 MaxSizeInBytes = int64(UALS_Settings::Get()->MaxFileSizeToParse) * 1024 * 1024;

        bool bNeedsRotation = (AgeInDays >= MaxAgeInDays) || (FileSizeInBytes >= MaxSizeInBytes);

        if (bNeedsRotation)
        {
            FString OldLogsDir = LogDir / TEXT("ArchivedLogs");
            PlatformFile.CreateDirectoryTree(*OldLogsDir);

            FString BaseFilename = FPaths::GetBaseFilename(FoundFilePath, true);
            FString Timestamp = Now.ToString(TEXT("%Y-%m-%d_%H-%M-%S"));
            FString NewFilePath = FString::Printf(TEXT("%s/%s_%s.log"), *OldLogsDir, *BaseFilename, *Timestamp);

            if (!PlatformFile.MoveFile(*NewFilePath, *FoundFilePath))
            {
                UE_LOG(LogTemp, Warning,TEXT("Failed to rotate log file: Unable to move from %s to %s. Since the larger files are not rotated, Please manually rotate them once a while for the ALS Logs Viewer to perform well "), *FoundFilePath, *NewFilePath);
            }
        }
    }
}


FString UALS_FileLog::EscapeForLog(const FString& InText)
{
    FString Escaped = InText;
    Escaped.TrimStartInline();
    Escaped.ReplaceCharWithEscapedCharInline();
    Escaped.ReplaceInline(TEXT("-|ALS|-"), TEXT(""));

    return Escaped;
}


FString UALS_FileLog::UnEscapeForWidget(const FString& InText)
{
    FString UnEscaped = InText.ReplaceEscapedCharWithChar();

    return UnEscaped;
}

