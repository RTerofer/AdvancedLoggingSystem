//Copyright © 2025 RTerofer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ALS_LogsUMG.h"
#include "ALS_Globals.h"
#include "Components/ListView.h"
#include "ALS_EntryObjects.generated.h"

struct FLogEntries;
struct FContextEntries;

UCLASS(BlueprintType, meta = (DisplayName = "ALS LogsMessageObject"))
class ALS_API UALS_LogMsgObject : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION()
    void SetMessageEntry(const FLogEntries& LogEntry, const bool& bIsBatch);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "ALS LogsMessageObject")
    FText Message;

    UPROPERTY(BlueprintReadOnly, Category = "ALS LogsMessageObject")
    FText DateTime;

    UPROPERTY(BlueprintReadOnly, Category = "ALS LogsMessageObject")
    FText PeriodMessage;

    UPROPERTY(BlueprintReadOnly, Category = "ALS LogsMessageObject")
    FString Level;
};


UCLASS(BlueprintType, meta = (DisplayName = "ALS LogsContextObject"))
class ALS_API UALS_LogContextObject : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION()
    void SetContextEntry(
        const FContextEntries& ContextEntry,
        const FString& InInstance,
        const FString& InSession,
        const FString& InSearchMessage,
        const FString& InSearchLevel,
        UListView* InContextList
    );

    UPROPERTY(BlueprintReadOnly, Category = "ALS LogsContextObject")
    UListView* ContextList;

    UPROPERTY(BlueprintReadOnly, Category = "ALS LogsContextObject")
    FText ContextText;

    UPROPERTY(BlueprintReadOnly, Category = "ALS LogsContextObject")
    FText NetworkText;

    UPROPERTY(BlueprintReadOnly, Category = "ALS LogsContextObject")
    FString Context;

    UPROPERTY(BlueprintReadOnly, Category = "ALS LogsContextObject")
    FString Instance;

    UPROPERTY(BlueprintReadOnly, Category = "ALS LogsContextObject")
    FString SessionID;

    UPROPERTY(BlueprintReadOnly, Category = "ALS LogsContextObject")
    FString SearchMessage;

    UPROPERTY(BlueprintReadOnly, Category = "ALS LogsContextObject")
    FString SearchLevel;
};



// Property Objects

UCLASS(BlueprintType, meta = (DisplayName = "ALS PropertyWorldObject"))
class ALS_API UALS_PropWorldObject : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly, Category = "ALS PropertyWorldObject")
    UObject* VarContext;

    UPROPERTY(BlueprintReadOnly, Category = "ALS PropertyWorldObject")
    FString ObjectName;

    UPROPERTY(BlueprintReadOnly, Category = "ALS PropertyWorldObject")
    UListView* MessageList;

    UPROPERTY(BlueprintReadOnly, Category = "ALS PropertyWorldObject")
    UListView* ObjectList;

    UPROPERTY(BlueprintReadOnly, Category = "ALS PropertyWorldObject")
    FString FilterProperty;

    UPROPERTY(BlueprintReadWrite, Category = "ALS PropertyWorldObject")
    bool IsExpanded;

    UPROPERTY(BlueprintReadOnly, Category = "ALS PropertyWorldObject")
    bool bIsAllComponents;

    UFUNCTION()
    void HandleActorDestroyed(AActor* DestroyedActor);
};


UCLASS(BlueprintType, meta = (DisplayName = "ALS PropertyVariableObject"))
class ALS_API UALS_PropVarObject : public UObject
{
    GENERATED_BODY()

public:
    FProperty* VarProperty;
    UObject* VarContext;
    UObject* VarOwner;

    UPROPERTY(BlueprintReadOnly, Category = "ALS PropertyVariableObject")
    FName PropertyName;

    UPROPERTY(BlueprintReadOnly, Category = "ALS PropertyVariableObject")
    bool IsSubHead;

    UPROPERTY(BlueprintReadOnly, Category = "ALS PropertyVariableObject")
    UListView* MessageList;
};


UCLASS(BlueprintType, meta = (DisplayName = "ALS PropertyMessageObject"))
class ALS_API UALS_PropMsgObject : public UObject
{
    GENERATED_BODY()

private:
    bool TickableSubscribe(float DeltaTime);

public:
    void StartSubscription();


    UPROPERTY(BlueprintReadOnly, Category = "ALS PropertyMessageObject")
    FString Message;

    UPROPERTY(BlueprintReadOnly, Category = "ALS PropertyMessageObject")
    FString Context;

    UPROPERTY(BlueprintReadOnly, Category = "ALS PropertyMessageObject")
    FString PropertyName;

    FTSTicker::FDelegateHandle TickerHandle;
    FString PrevMessage;
    FProperty* VarProperty;
    UObject* VarContext;
    UObject* VarOwner;
};