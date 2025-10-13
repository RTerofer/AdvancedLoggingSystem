//Copyright © 2025 RTerofer. All Rights Reserved.

#include "ALS_Subsystem.h"
#include "ALS_Settings.h"
#include "ALS_FunctionLibrary.h"

void UALSStorageSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void UALSStorageSubsystem::Deinitialize()
{
    FWorldDelegates::OnWorldCleanup.RemoveAll(this);
    Super::Deinitialize();
}

UALS_LogsUMG* UALSStorageSubsystem::GetLogWidget(APlayerController* PlayerController) const
{
    return LogsMap.FindRef(PlayerController);
}
void UALSStorageSubsystem::SetLogWidget(APlayerController* PlayerController, UALS_LogsUMG* LogsWidget)
{
    LogsMap.Add(PlayerController, LogsWidget);
}
void UALSStorageSubsystem::RemoveLogWidget(APlayerController* PlayerController)
{
    LogsMap.Remove(PlayerController);
}

UALS_PropertyUMG* UALSStorageSubsystem::GetPropertyWidget(APlayerController* PlayerController) const
{
    return PropsMap.FindRef(PlayerController);
}
void UALSStorageSubsystem::SetPropertyWidget(APlayerController* PlayerController, UALS_PropertyUMG* PropertyWidget)
{
    PropsMap.Add(PlayerController, PropertyWidget);
}
void UALSStorageSubsystem::RemovePropertyWidget(APlayerController* PlayerController)
{
    PropsMap.Remove(PlayerController);
}