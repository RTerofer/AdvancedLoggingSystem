//Copyright © 2025 RTerofer. All Rights Reserved.

#include "ALS_LogsUMG.h"
#include "ALS.h"
#include "ALS_Settings.h"
#include "ALS_EntryObjects.h"
#include "ALS_FileLog.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Async/ParallelFor.h"
#include "Async/Async.h"
#include "Components/ListView.h"
#include "Components/Overlay.h"
#include "HAL/PlatformFileManager.h"
#include "Kismet/GameplayStatics.h"
#include "Algo/Sort.h"

#if WITH_EDITOR
#include "Settings/LevelEditorPlaySettings.h"
#endif

struct FALSKey
{
    FString Composite;

    FALSKey(const FString& Src, const FString& Msg) : Composite(Src + TEXT("|") + Msg) {}

    friend uint32 GetTypeHash(const FALSKey& K)
    {
        return GetTypeHash(K.Composite);
    }
    friend bool operator == (const FALSKey& A, const FALSKey& B)
    {
        return A.Composite == B.Composite;
    }
};

void UALS_LogsUMG::NativeConstruct()
{
    Super::NativeConstruct();

    bool bAllowFileLog = UALS_Settings::Get()->IsFileLoggingAllowed();

    if (!bAllowFileLog)
    {
        RemoveFromParent();
    }
}

bool UALS_LogsUMG::IsRuntime() const
{
    return GetWorld()->IsGameWorld();
}

FString UALS_LogsUMG::GetLatestSession(const FString& InstanceName) const
{
    return StoredSessions.Num() > 0 ? StoredSessions.Last() : TEXT("");
}

FString UALS_LogsUMG::GetLatestInstance() const
{
    FString LogDir = UALS_Settings::Get()->FileLogRootDir.Path;
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

    auto SearchPotentialFiles = [&](FString FileName) -> FString
        {
            for (int i = 0; i < 11; i++)
            {
                FString SearchFile = FString::Printf(TEXT("%s (%d).log"), *FileName, i);
                FString PotentialCSV = LogDir / SearchFile;

                if (PlatformFile.FileExists(*PotentialCSV))
                {
                    return SearchFile;
                }         
            }

            return "";
        };

#if WITH_EDITOR
    if (ULevelEditorPlaySettings* PlaySettings = GetMutableDefault<ULevelEditorPlaySettings>()) 
    {
        EPlayNetMode EditorPlayMode;
        PlaySettings->GetPlayNetMode(EditorPlayMode);

        FString ProjectName = FString::Printf(TEXT("%s"), FApp::GetProjectName());

        FString FoundFile;
        switch (EditorPlayMode)
        {
        case EPlayNetMode::PIE_Standalone:
            FoundFile = SearchPotentialFiles(FString::Printf(TEXT("%s_Standalone"), *ProjectName));
            break;
        case EPlayNetMode::PIE_ListenServer:
            FoundFile = SearchPotentialFiles(FString::Printf(TEXT("%s_Server"), *ProjectName));
            break;
        case EPlayNetMode::PIE_Client:
            FoundFile = SearchPotentialFiles(FString::Printf(TEXT("%s_Client"), *ProjectName));
            break;
        default:
            FoundFile = SearchPotentialFiles(FString::Printf(TEXT("%s_Unknown"), *ProjectName));
            break;
        }

        if (!FoundFile.IsEmpty())
        {
            FoundFile.RemoveFromEnd(TEXT(".log"));
            return FoundFile;
        }
    }
#endif

    // Fallback if no files found from netmode
    TArray<FString> FileNames;
    IFileManager::Get().FindFiles(FileNames, *LogDir, TEXT("*.log"));

    FString LatestFile;
    FDateTime LatestTimestamp = FDateTime::MinValue();

    for (const FString& FileName : FileNames)
    {
        const FString FullPath = LogDir / FileName;
        const FDateTime FileTimestamp = PlatformFile.GetTimeStamp(*FullPath);

        if (FileTimestamp > LatestTimestamp)
        {
            LatestTimestamp = FileTimestamp;
            LatestFile = FileName;
        }
    }

    LatestFile.RemoveFromEnd(TEXT(".log"));
    return LatestFile;
}

FString UALS_LogsUMG::GetCurrentInstance() const
{
    return UALS_FileLog::GetCurrentInstance(GetWorld());
}

bool UALS_LogsUMG::GetAllInstances(TArray<FString>& OutInstances, FString& OutMessage)
{
    FString LogDir = UALS_Settings::Get()->FileLogRootDir.Path;
    IFileManager::Get().FindFiles(OutInstances, *LogDir, TEXT(".log"));

    if (UALS_Settings::Get()->bIncludeArchivedLogsInViewer)
    {
        FString OldLogsDir = LogDir / TEXT("ArchivedLogs");
        IFileManager::Get().FindFiles(OutInstances, *OldLogsDir, TEXT(".log"));
    }

    for (FString& FileName : OutInstances)
    {
        FileName.RemoveFromEnd(TEXT(".log"));
    }

    if (OutInstances.IsEmpty())
    {
        OutMessage = FString::Printf(TEXT("Error: No messages found in %s. \nPlease use ALS Print/Macro/Inspector/BT to log and view them here."), *UALS_Settings::Get()->FileLogRootDir.Path);
        return false;
    }

    return true;
}

bool UALS_LogsUMG::GetFileContent(const FString Instance, FString& OutContent, FString& OutMessage, bool IgnoreSizeCheck)
{
    FString LogFilePath = UALS_Settings::Get()->FileLogRootDir.Path / Instance + TEXT(".log");
    FString OldFilePath = UALS_Settings::Get()->FileLogRootDir.Path / TEXT("ArchivedLogs") / Instance + TEXT(".log");

    if (FPaths::FileExists(LogFilePath))
    {
        if (!FFileHelper::LoadFileToString(OutContent, *LogFilePath))
        {
            OutMessage = "Error: Unable to parse or access the log file.";
            return false;
        }
    }
    else if (UALS_Settings::Get()->bIncludeArchivedLogsInViewer && FPaths::FileExists(OldFilePath))
    {
        if (!FFileHelper::LoadFileToString(OutContent, *OldFilePath))
        {
            OutMessage = "Error: Unable to parse or access the archived log file.";
            return false;
        }

        LogFilePath = OldFilePath;
    }
    else
    {
        OutMessage = "Error: Unable to find the Instance file. Please check if the file is present or has proper read permissions.";
        return false;
    }

    if (!IgnoreSizeCheck)
    {
        int32 GetFileSize;
        bool IsLarger = UALS_FileLog::IsFileBigger(LogFilePath, GetFileSize);

        if (IsLarger)
        {
            OutMessage =
                FString::Printf(
                    TEXT("Warning: File size is %d MB. Processing such a large file may consume more memory.\nCaution: Do you still wish to parse the selected instance?"),
                    GetFileSize
                );
            return false;
        }
    }

    return true;
}

bool UALS_LogsUMG::GetAllSessions(const bool IgnoreSizeCheck, const FString& Instance, TArray<FString>& OutSessions, FString& OutMessage)
{
    FString FileContent;
    if (!GetFileContent(Instance, FileContent, OutMessage, IgnoreSizeCheck))
    {
        return false;
    }

    TArray<FString> Lines;
    FileContent.ParseIntoArray(Lines, TEXT("\n"), true);

    for (const FString& Line : Lines)
    {
        if (Line.IsEmpty()) continue;

        TArray<FString> Columns;
        Line.ParseIntoArray(Columns, TEXT("-|ALS|-"), false);

        if (Columns.Num() < 4) continue;

        FString Session = Columns[2].TrimQuotes().TrimStartAndEnd();
        if (!OutSessions.Contains(Session))
        {
            OutSessions.Add(Session);
            StoredSessions.Add(Session);
        }
    }

    if (OutSessions.IsEmpty())
    {
        OutMessage = FString::Printf(TEXT("Error: No sessions found. Please check if the file contains a session, or perform a simple ALS Print to generate one."));
        return false;
    }

    return true;
}

bool UALS_LogsUMG::GetAllContexts(const FString& Instance, const FString& SessionID, TArray<FContextEntries>& OutContexts, FString& OutMessage)
{
    FString FileContent;
    if (!GetFileContent(Instance, FileContent, OutMessage))
    {
        return false;
    }
    
    TArray<FString> Lines;
    FileContent.ParseIntoArray(Lines, TEXT("\n"), true);

    TArray<FString> ContextRawUnique;

    for (int32 i = Lines.Num() - 1; i >= 0; i--)
    {
        const FString& Line = Lines[i];
        if (Line.IsEmpty()) continue;

        TArray<FString> Columns;
        Line.ParseIntoArray(Columns, TEXT("-|ALS|-"), false);
        if (Columns.Num() < 7) continue;

        FString LoggedSession = Columns[2].TrimQuotes().TrimStartAndEnd();
        FString ContextRaw = Columns[3].TrimQuotes().TrimStartAndEnd();

        if (LoggedSession == SessionID && !ContextRawUnique.Contains(ContextRaw))
        {
            FText JustContext;
            FText NetworkText = FText::FromString(TEXT(""));

            if (ContextRaw.Contains(TEXT("] [")))
            {
                int32 NetworkEnd = ContextRaw.Find(TEXT("]"));
                JustContext = FText::FromString(ContextRaw.RightChop(NetworkEnd).Replace(TEXT("["), TEXT("")).Replace(TEXT("]"), TEXT("")));

                if (NetworkEnd > 0)
                {
                    NetworkText = FText::FromString(ContextRaw.Mid(1, NetworkEnd - 1));
                    FContextEntries ContextEntry(ContextRaw, JustContext, NetworkText);
                    OutContexts.Add(ContextEntry);
                    ContextRawUnique.Add(ContextRaw);
                }
            }
            else
            {
                JustContext = FText::FromString(ContextRaw.Replace(TEXT("["), TEXT("")).Replace(TEXT("]"), TEXT("")));
                FContextEntries ContextEntry(ContextRaw, JustContext, NetworkText);
                OutContexts.Add(ContextEntry);
                ContextRawUnique.Add(ContextRaw);
            }
        }
    }

    if (OutContexts.IsEmpty())
    {
        OutMessage = FString::Printf(TEXT("Error: No ALS logs found for this session"));
        return false;
    }

    OutContexts.RemoveAll([](const FContextEntries& Entry)
        {
            return Entry.ContextRaw.Contains(TEXT("[SESSION CREATED]"));
        });

    return true;
}

void UALS_LogsUMG::GetFilteredLogs(
    const bool& Descending,
    const bool& bIsBatch,
    const FString& Instance, 
    const FString& SessionID, 
    const FString& Context, 
    const FString& SearchMessage, 
    const FString& SearchLevel, 
    FOnGetLogsCompletedNative OnGetLogsCompleted
)
{
    if (CurrentCancelToken.IsValid())
    {
        *CurrentCancelToken = true;
    }

    CurrentCancelToken = MakeShared<bool>(false);
    TSharedPtr<bool> CancelToken = CurrentCancelToken;
    TWeakObjectPtr<UALS_LogsUMG> ThisWidget = this;

    FString FileContent;
    FString OutMessage;
    if (!GetFileContent(Instance, FileContent, OutMessage))
    {
        UE_LOG(LogTemp, Error, TEXT("%s"), *OutMessage);
        return;
    }

    TArray<FString> Lines;
    FileContent.ParseIntoArray(Lines, TEXT("\n"), true);

    Async(EAsyncExecution::ThreadPool, [=]()
        {
            if (*CancelToken || !ThisWidget.IsValid()) return;

            auto GetColumnValue = [&](TArray<FString> Cols, int32 ColIndex) -> FString
                {
                    FString Value = Cols[ColIndex].TrimQuotes().TrimStartAndEnd();
                    if (ColIndex == 6)
                    {
                        Value = Value.TrimStart().ReplaceEscapedCharWithChar().Replace(TEXT("-c|c-"), TEXT(","));
                    }
                    return Value;
                };

            TArray<FLogEntries> LocalEntries;
            LocalEntries.Reserve(Lines.Num());

            FCriticalSection Mutex;

            ParallelFor(Lines.Num(), [&](int32 i)
                {
                    if (*CancelToken || !ThisWidget.IsValid()) return;

                    const FString& Line = Lines[i];
                    if (Line.IsEmpty()) return;

                    TArray<FString> Cols;
                    Line.ParseIntoArray(Cols, TEXT("-|ALS|-"), false);
                    if (Cols.Num() < 7) return;

                    const uint64 LoggedCycle = FCString::Strtoui64(*GetColumnValue(Cols, 0), nullptr, 10);
                    const FString LoggedTime = GetColumnValue(Cols, 1);
                    const FString LoggedSession = GetColumnValue(Cols, 2);
                    const FString LoggedContext = GetColumnValue(Cols, 3);
                    const FString LoggedSource = GetColumnValue(Cols, 4);
                    const FString LoggedLevel = GetColumnValue(Cols, 5);
                    const FString LoggedMessage = GetColumnValue(Cols, 6);

                    if (LoggedSession != SessionID || LoggedContext != Context) return;

                    if ((!SearchMessage.IsEmpty() && !LoggedMessage.Contains(SearchMessage)) ||
                        (!SearchLevel.Contains(TEXT("All Levels")) && LoggedLevel != SearchLevel))
                    {
                        return;
                    }

                    FDateTime ParsedTime;
                    if (!FDateTime::Parse(LoggedTime, ParsedTime)) return;

                    {
                        FScopeLock Lock(&Mutex);
                        LocalEntries.Add(FLogEntries(LoggedLevel, LoggedMessage, LoggedSource, ParsedTime, LoggedCycle));
                    }   
                });

            if (bIsBatch)
            {
                TMap<FALSKey, FLogEntries> UniqueMap;
                UniqueMap.Reserve(LocalEntries.Num());

                for (FLogEntries& E : LocalEntries)
                {
                    FALSKey Key(E.Source, E.Message);

                    FLogEntries* Found = UniqueMap.Find(Key);
                    if (Found)
                    {
                        Found->Count++;
                        Found->StartTime = FMath::Min(Found->StartTime, E.StartTime);
                        Found->EndTime = FMath::Max(Found->EndTime, E.EndTime);
                        Found->CycleCounter = FMath::Min<uint64>(Found->CycleCounter, E.CycleCounter);
                    }
                    else
                    {
                        UniqueMap.Add(Key, MoveTemp(E));
                    }
                }

                LocalEntries.Reset();
                LocalEntries.Reserve(UniqueMap.Num());

                for (TPair<FALSKey, FLogEntries>& Pair : UniqueMap)
                {
                    FLogEntries& Entry = Pair.Value;

                    if (Entry.Count > 1)
                    {
                        Entry.PeriodMessage = FString::Printf(
                            TEXT("(%d times logged from %s to %s)"),
                            Entry.Count,
                            *Entry.StartTime.ToString(TEXT("%H:%M:%S")),
                            *Entry.EndTime.ToString(TEXT("%H:%M:%S")));
                    }

                    Entry.DateTime = Entry.StartTime.ToFormattedString(TEXT("%d:%m:%Y %H:%M:%S"));

                    LocalEntries.Add(MoveTemp(Entry));
                }
            }

            auto ByCycle = [](const FLogEntries& A, const FLogEntries& B)
                {
                    return A.CycleCounter < B.CycleCounter;
                };

            Algo::Sort(LocalEntries, ByCycle); 

            if (Descending)
            {
                Algo::Reverse(LocalEntries);
            }

            LocalEntries.Shrink();           

            AsyncTask(ENamedThreads::GameThread, [=]()
                {
                    OnGetLogsCompleted.ExecuteIfBound(LocalEntries);
                });
        });

    return;
}

void UALS_LogsUMG::GetMessageObjects(
    const UALS_LogContextObject* ContextObject, 
    const bool& bDescending, 
    const bool& bIsBatch, 
    FOnGetLogsCompletedDynamic OnGetLogsCompleted
)
{
    FOnGetLogsCompletedNative NativeDelegate;
    NativeDelegate.BindLambda([=](const TArray<FLogEntries>& Sorted)
        {
            TArray<UALS_LogMsgObject*> MessageObjects;
            for (const FLogEntries& Entry : Sorted)
            {
                UALS_LogMsgObject* MessageObject = NewObject<UALS_LogMsgObject>();
                MessageObject->SetMessageEntry(Entry, bIsBatch);
                MessageObjects.Add(MessageObject);
            }

            if (MessageObjects.Num() > UALS_Settings::Get()->MaxNumberOfListsToCreate)
            {
                FString OutMessage = FString::Printf(
                    TEXT("Warning: The number of logs to ungroup is large. Creating this many list entries may consume more memory.\nCaution: Would you still like to proceed with ungrouping these messages? "),
                    MessageObjects.Num());
                OnGetLogsCompleted.ExecuteIfBound(MessageObjects, false, OutMessage);
                return;
            }

            OnGetLogsCompleted.ExecuteIfBound(MessageObjects, true, TEXT("Success"));
            return;
        });

    GetFilteredLogs(
        bDescending,
        bIsBatch,
        ContextObject->Instance,
        ContextObject->SessionID,
        ContextObject->Context,
        ContextObject->SearchMessage,
        ContextObject->SearchLevel,
        NativeDelegate
    );
}

void UALS_LogsUMG::SetContextObjects(
    UListView* ContextList,
    const TArray<FContextEntries>& Entries, 
    const FString& Instance, 
    const FString& SessionID, 
    const FString& SearchContext, 
    const FString& SearchMessage, 
    const FString& SearchLevel
)
{
    TArray<UALS_LogContextObject*> ContextObjects;
    ContextList->ClearListItems();
    for (const FContextEntries& Entry : Entries)
    {
        if (Entry.ContextRaw == SearchContext || SearchContext == TEXT("All Contexts"))
        {
            UALS_LogContextObject* ContextObject = NewObject<UALS_LogContextObject>();
            ContextObject->SetContextEntry(Entry, Instance, SessionID, SearchMessage, SearchLevel, ContextList);
            ContextObjects.Add(ContextObject);
        }
    }
    ContextList->SetListItems(ContextObjects);
}


