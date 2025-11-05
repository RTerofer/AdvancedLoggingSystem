//Copyright © 2025 RTerofer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ALS_Definitions.h"
#include "BehaviorTree/BTService.h"
#include "ALS_Service.generated.h"

UCLASS(meta = (DisplayName = "Print Service"))
class ALS_API UALS_AI_Service : public UBTService
{
    GENERATED_BODY()

public:
    UALS_AI_Service();

protected:
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
    virtual FString GetStaticDescription() const override;

    virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

    virtual void OnGameplayTaskActivated(UGameplayTask& Task) override {};
    virtual void OnGameplayTaskDeactivated(UGameplayTask& Task) override {};

public:
    //Toggle to disable or enable this print
    UPROPERTY(EditAnywhere, Category = "Print Service")
    bool bEnabled = true;

    //Caller Name (Optional to identify from where the print is being called)
    UPROPERTY(EditAnywhere, Category = "Print Service")
    FString CallerName;

    //Key to Print
    UPROPERTY(EditAnywhere, Category = "Print Service")
    FBlackboardKeySelector PrintBBKey;

    // Print Text Color
    UPROPERTY(EditAnywhere, Category = "Print Service")
    FColor TextColor = FColor::Green;

    // Print Key
    UPROPERTY(EditAnywhere, Category = "Print Service")
    FName Key = TEXT("-1");

    // Log Severity
    UPROPERTY(EditAnywhere, Category = "Print Service")
    ELogSeverity LogSeverity = ELogSeverity::Info;

    // Print To World
    UPROPERTY(EditAnywhere, Category = "Print Service")
    bool bPrintToWorld = false;

    // Text Location - Actor / Vector Key
    UPROPERTY(EditAnywhere, Category = "Print Service", meta = (EditCondition = "bPrintToWorld"))
    FBlackboardKeySelector TextLocation;

    // Optional: Will result in TextLocation + OffsetLocation
    UPROPERTY(EditAnywhere, Category = "Print Service", meta = (EditCondition = "bPrintToWorld"))
    FVector OffsetLocation = FVector::ZeroVector;

private:
    FString PrevMessage;
};
