//Copyright © 2025 RTerofer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ALS_Subsystem.generated.h"

class UALS_LogsUMG;
class UALS_PropertyUMG;

UCLASS()
class ALS_API UALSStorageSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Logs
    UALS_LogsUMG* GetLogWidget(APlayerController* PlayerController) const;
    void SetLogWidget(APlayerController* PlayerController, UALS_LogsUMG* LogsWidget);
    void RemoveLogWidget(APlayerController* PlayerController);

    // Properties
    UALS_PropertyUMG* GetPropertyWidget(APlayerController* PlayerController) const;
    void SetPropertyWidget(APlayerController* PlayerController, UALS_PropertyUMG* PropertyWidget);
    void RemovePropertyWidget(APlayerController* PlayerController);

private:
    TMap<TWeakObjectPtr<APlayerController>, UALS_LogsUMG*> LogsMap;
    TMap<TWeakObjectPtr<APlayerController>, UALS_PropertyUMG*> PropsMap;
};