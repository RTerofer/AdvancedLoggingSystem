//Copyright © 2025 RTerofer. All Rights Reserved.

#include "ALS_Task.h"
#include "ALS_Globals.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"

UALS_Task::UALS_Task()
{
    NodeName = "Print (ALS)"; 
    TextLocation.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UALS_Task, TextLocation), AActor::StaticClass());
    TextLocation.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UALS_Task, TextLocation), USceneComponent::StaticClass());
    TextLocation.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UALS_Task, TextLocation));
}

EBTNodeResult::Type UALS_Task::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    FName KeyName = PrintBBKey.SelectedKeyName;

    if (!BlackboardComp || !KeyName.IsValid() || !bEnabled)
    {
        return EBTNodeResult::Succeeded;
    }

    const uint8* RawData = BlackboardComp->GetKeyRawData(KeyName);
    FString Message = RawData ? BlackboardComp->DescribeKeyValue(KeyName, EBlackboardDescription::KeyWithValue) : "NULL";
    FString CallerStr = FString::Printf(TEXT("[%s]"), *CallerName);

    if (!CallerName.IsEmpty())
    {
        Message = FString::Printf(TEXT("%s %s"), *CallerStr, *Message);
    }

    FVector TextLoc;

    if (UObject* TextLocationObj = BlackboardComp->GetValueAsObject(TextLocation.SelectedKeyName))
    {
        if (AActor* Actor = Cast<AActor>(TextLocationObj))
        {
            TextLoc = Actor->GetActorLocation() + OffsetLocation;
        }

        if (USceneComponent* Scene = Cast<USceneComponent>(TextLocationObj))
        {
            TextLoc = Scene->GetComponentLocation() + OffsetLocation;
        }
    }

    AAIController* AICon = OwnerComp.GetAIOwner();
    AActor* Context = AICon ? Cast<AActor>(AICon->GetPawn()) : nullptr;
    FString SourceID = FString::Printf(TEXT("BT%s::%d"), *BlackboardComp->GetName(), this->GetUniqueID());
    FPrintConfig PrintConfig = FPrintConfig(TextColor, TextDuration, LogSeverity, EPrintMode::ScreenAndLog);

    bool InitiateFileLog = false;
    if (Message != PrevMessage || !UALS_Settings::Get()->LogUniqueMsgsForBT)
    {
        PrevMessage = Message;
        InitiateFileLog = true;
    }

    if (bPrintToWorld)
    {
        UALS_Globals::DrawALS(Message, nullptr, TextLoc, PrintConfig, Context, SourceID, InitiateFileLog);
    }
    else
    {
        UALS_Globals::PrintALS(Message, PrintConfig, Context, SourceID, InitiateFileLog);
    }

    return EBTNodeResult::Succeeded;
}

FString UALS_Task::GetStaticDescription() const
{
    FString KeyStr = FString::Printf(TEXT("Key: %s\nDuration: %.2f"), *PrintBBKey.SelectedKeyName.ToString(), TextDuration);
    FString CallerStr = FString::Printf(TEXT("\nCaller: %s"), *CallerName);

    if (!CallerName.IsEmpty())
    {
        KeyStr = FString::Printf(TEXT("%s%s"), *KeyStr, *CallerStr);
    }

    return KeyStr;
}
