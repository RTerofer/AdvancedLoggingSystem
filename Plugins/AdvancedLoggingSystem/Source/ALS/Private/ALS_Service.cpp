//Copyright © 2025 RTerofer. All Rights Reserved.

#include "ALS_Service.h"
#include "ALS_Globals.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BlackboardComponent.h"

UALS_AI_Service::UALS_AI_Service()
{
    NodeName = "Print (ALS)";
    Interval = 1.0f;
    RandomDeviation = 0.0f;

    TextLocation.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UALS_AI_Service, TextLocation), AActor::StaticClass());
    TextLocation.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UALS_AI_Service, TextLocation), USceneComponent::StaticClass());
    TextLocation.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UALS_AI_Service, TextLocation));
}

void UALS_AI_Service::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    FName KeyName = PrintBBKey.SelectedKeyName;

    if (!BlackboardComp || !KeyName.IsValid() || !bEnabled)
    {
        return;
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
    FPrintConfig PrintConfig = FPrintConfig(TextColor, Interval, LogSeverity, EPrintMode::ScreenAndLog);

    bool InitiateFileLog = false;
    if (PrevMessage != Message)
    {
        PrevMessage = Message;
        InitiateFileLog = true;
    }

    if (!UALS_Settings::Get()->LogUniqueMsgsForBT)
    {
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

    return;
}

FString UALS_AI_Service::GetStaticDescription() const
{
    FString KeyStr = FString::Printf(TEXT("Key: %s\nInterval: %.2f"), *PrintBBKey.SelectedKeyName.ToString(), Interval);
    FString CallerStr = FString::Printf(TEXT("\nCaller: %s"), *CallerName);

    if (!CallerName.IsEmpty())
    {
        KeyStr = FString::Printf(TEXT("%s%s"), *KeyStr, *CallerStr);
    }

    return KeyStr;
}
