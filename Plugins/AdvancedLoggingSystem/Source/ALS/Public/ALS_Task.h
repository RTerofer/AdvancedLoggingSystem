//Copyright © 2025 RTerofer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ALS_Definitions.h"
#include "BehaviorTree/BTTaskNode.h"
#include "ALS_Task.generated.h"

// Forward Declarations
class UBlackboardComponent;

UCLASS(meta = (DisplayName = "Print Task"))
class ALS_API UALS_Task : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UALS_Task();

protected:
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual FString GetStaticDescription() const override;

    virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

    virtual void OnGameplayTaskActivated(UGameplayTask& Task) override {};
    virtual void OnGameplayTaskDeactivated(UGameplayTask& Task) override {};

public:
    //Toggle to disable or enable this print
    UPROPERTY(EditAnywhere, Category = "Print Task")
    bool bEnabled = true;

    //Caller Name (Optional to identify from where the print is being called)
    UPROPERTY(EditAnywhere, Category = "Print Task")
    FString CallerName;

    //Key to Print
    UPROPERTY(EditAnywhere, Category = "Print Task")
    FBlackboardKeySelector PrintBBKey;

    // Print Text Color
    UPROPERTY(EditAnywhere, Category = "Print Task")
    FColor TextColor = FColor::Green;

    // Print Text Duration
    UPROPERTY(EditAnywhere, Category = "Print Task")
    float TextDuration = 2.0f;

    // Print Key
    UPROPERTY(EditAnywhere, Category = "Print Task")
    FName Key = TEXT("-1");

    // Log Severity
    UPROPERTY(EditAnywhere, Category = "Print Task")
    ELogSeverity LogSeverity = ELogSeverity::Info;

    // Print To World
    UPROPERTY(EditAnywhere, Category = "Print Task")
    bool bPrintToWorld = false;

    // Text Location - Actor / Vector Key
    UPROPERTY(EditAnywhere, Category = "Print Task", meta = (EditCondition = "bPrintToWorld"))
    FBlackboardKeySelector TextLocation;

    // Optional: Will result in TextLocation + OffsetLocation
    UPROPERTY(EditAnywhere, Category = "Print Task", meta = (EditCondition = "bPrintToWorld"))
    FVector OffsetLocation = FVector::ZeroVector;

private:
    FString PrevMessage;

};
