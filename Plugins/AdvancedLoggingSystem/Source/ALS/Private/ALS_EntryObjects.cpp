//Copyright © 2025 RTerofer. All Rights Reserved.

#include "ALS_EntryObjects.h"

// Log Message Object
void UALS_LogMsgObject::SetMessageEntry(const FLogEntries& LogEntry, const bool& bIsBatch)
{
    Message = FText::FromString(LogEntry.Message);
    DateTime = FText::FromString(LogEntry.DateTime);
    PeriodMessage = FText::FromString(bIsBatch ? LogEntry.PeriodMessage : TEXT(""));
    Level = LogEntry.Level;
};

// Log Context Object
void UALS_LogContextObject::SetContextEntry(
    const FContextEntries& ContextEntry, 
    const FString& InInstance, 
    const FString& InSession, 
    const FString& InSearchMessage, 
    const FString& InSearchLevel, 
    UListView* InContextList
)
{
    ContextText = ContextEntry.ContextText;
    NetworkText = ContextEntry.NetworkText;
    Context = ContextEntry.ContextRaw;
    Instance = InInstance;
    SessionID = InSession;
    SearchMessage = InSearchMessage;
    SearchLevel = InSearchLevel;
    ContextList = InContextList;
}

// Property World Object
void UALS_PropWorldObject::HandleActorDestroyed(AActor* DestroyedActor)
{
    if (ObjectList)
    {
        if (MessageList)
        {
            TArray<UObject*> ExistingMsgObjects = MessageList->GetListItems();
            for (UObject* ExistingObj : ExistingMsgObjects)
            {
                if (UALS_PropMsgObject* MsgObj = Cast<UALS_PropMsgObject>(ExistingObj))
                {
                    if (MsgObj->VarContext == VarContext)
                    {
                        MessageList->RemoveItem(MsgObj);

                        FTSTicker::GetCoreTicker().RemoveTicker(MsgObj->TickerHandle);
                        MsgObj->MarkAsGarbage();
                    }              
                }
            }
        }

        ObjectList->RemoveItem(this);
    }
}

// Property Message Object
void UALS_PropMsgObject::StartSubscription()
{
    float RefreshTimer = UALS_Settings::Get()->RefreshTimer;

    TickerHandle = FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateUObject(this, &UALS_PropMsgObject::TickableSubscribe),
        RefreshTimer
    );
}

bool UALS_PropMsgObject::TickableSubscribe(float DeltaTime)
{
    if (!VarContext || !VarProperty || !VarOwner)
    {
        FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
        return false;
    }

    PropertyName = VarProperty->GetName();
    FStringBuilderBase OutValue;
    OutValue.Appendf(TEXT(""));

    void* ValuePtr = VarProperty->ContainerPtrToValuePtr<void>(VarOwner);
    UALS_Globals::ConvertToString_Property(VarProperty, ValuePtr, OutValue);

    FString ContextString;

    AActor* OwnerActor = Cast<AActor>(VarContext);
    UActorComponent* OwnerComp = Cast<UActorComponent>(VarOwner);

    if (OwnerComp)
    {
        ContextString = OwnerComp->GetReadableName();
    }
    else if (OwnerActor)
    {
        ContextString = OwnerActor->GetName().Replace(TEXT("_C_"), TEXT(" #"));

#if WITH_EDITOR
        if (UALS_Settings::Get()->UseActorLabel) ContextString = OwnerActor->GetActorLabel();
#endif
    }
    else
    {
		ContextString = VarOwner->GetName();
    }

    Context = FString::Printf(TEXT("[%s]"), *ContextString);
    Message = OutValue.ToString();
    Message.TrimStartInline();

    if (Message != PrevMessage || !UALS_Settings::Get()->LogUniqueMsgsForPD)
    {
        FString SourceID = FString::Printf(TEXT("%s_%d"), *VarContext->GetName(), this->GetUniqueID());

        FString NetworkName = UALS_Globals::GetNetworkContextTag(VarContext);
        FString CallerString = FString::Printf(TEXT("%s%s"), *NetworkName, *Context);

        FString LogString = FString::Printf(TEXT("%s [%s] %s"), *CallerString, *PropertyName, *Message);
        FString FileString = FString::Printf(TEXT("[%s] %s"), *PropertyName, *Message);

        UALS_Globals::LogOutput(LogString, ELogSeverity::Info);
        UALS_FileLog::CreateMessageLog(VarContext, CallerString, SourceID, "Info", FileString);

        PrevMessage = Message;
    }

    return true;
}
