//Copyright © 2025 RTerofer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ALS_Definitions.h"
#include "Blueprint/UserWidget.h"
#include "ALS_LogsUMG.generated.h"

class UALS_LogMsgObject;
class UALS_LogContextObject;

DECLARE_DELEGATE_OneParam(FOnGetLogsCompletedNative, const TArray<FLogEntries>&);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnGetLogsCompletedDynamic, const TArray<UALS_LogMsgObject*>&, MessageObjects, bool, IsSuccess, FString, OutMessage);

UCLASS(meta = (DisplayName = "ALS LogsViewer"))
class ALS_API UALS_LogsUMG : public UUserWidget
{
    GENERATED_BODY()

protected:
    TArray<FString> StoredSessions;

protected:
    TSharedPtr<bool> CurrentCancelToken;

    virtual void NativeConstruct() override;

    UFUNCTION(BlueprintCallable, Category = "ALS LogsViewer")
    bool IsRuntime() const;

    UFUNCTION(BlueprintCallable, Category = "ALS LogsViewer")
    FString GetLatestSession(const FString& InstanceName) const;

    UFUNCTION(BlueprintCallable, Category = "ALS LogsViewer")
    FString GetLatestInstance() const;

    UFUNCTION(BlueprintCallable, Category = "ALS LogsViewer")
    FString GetCurrentInstance() const;

    UFUNCTION(BlueprintCallable, Category = "ALS LogsViewer")
    bool GetAllInstances(TArray<FString>& OutInstances, FString& OutMessage);

    UFUNCTION(BlueprintCallable, Category = "ALS LogsViewer")
    bool GetAllSessions(const bool IgnoreSizeCheck, const FString& Instance, TArray<FString>& OutSessions, FString& OutMessage);

    UFUNCTION(BlueprintCallable, Category = "ALS LogsViewer")
    bool GetAllContexts(const FString& Instance, const FString& SessionID, TArray<FContextEntries>& OutContexts, FString& OutMessage);

    void GetFilteredLogs(
        const bool& Descending,
        const bool& bIsBatch,
        const FString& Instance, 
        const FString& SessionID, 
        const FString& Context, 
        const FString& SearchMessage, 
        const FString& SearchLevel, 
        FOnGetLogsCompletedNative OnGetLogsCompleted
    );

    bool GetFileContent(const FString Instance, FString& OutContent, FString& OutMessage, bool IgnoreSizeCheck = true);
    

// Objects Helper Functions
protected:
    UFUNCTION(BlueprintCallable, Category = "ALS LogsUMG")
    void GetMessageObjects(
        const UALS_LogContextObject* ContextObject, 
        const bool& bDescending, 
        const bool& bIsBatch, 
        FOnGetLogsCompletedDynamic OnGetLogsCompleted
    );

    UFUNCTION(BlueprintCallable, Category = "ALS LogsUMG")
    void SetContextObjects(
        UListView* ContextList,
        const TArray<FContextEntries>& Entries,
        const FString& Instance,
        const FString& SessionID,
        const FString& SearchContext,
        const FString& SearchMessage, 
        const FString& SearchLevel
    );
};