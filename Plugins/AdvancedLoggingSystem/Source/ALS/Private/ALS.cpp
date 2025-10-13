//Copyright © 2025 RTerofer. All Rights Reserved.

#include "ALS.h"
#include "ALS_Subsystem.h"
#include "ALS_FileLog.h"
#include "ALS_LogsUMG.h"
#include "ALS_PropertyUMG.h"
#include "ALS_FunctionLibrary.h"
#include "ALS_Settings.h"
#include "ALS_InputProcessor.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "Framework/Application/SlateApplication.h"
#include "Kismet/GameplayStatics.h"

#if WITH_EDITOR
#include "Settings/ProjectPackagingSettings.h"
#include "WidgetBlueprint.h"
#endif


#define LOCTEXT_NAMESPACE "FALSModule"

TSharedPtr<FALSInputProcessor> GInputProcessor = nullptr;

void FALSModule::StartupModule()
{   
    const bool bAllowFileLog = UALS_Settings::Get()->IsFileLoggingAllowed();
    const bool bAllowPropInspector = UALS_Settings::Get()->IsPropertyInspectorAllowed();

    if (!GInputProcessor.IsValid() && FSlateApplication::IsInitialized())
    {
        if (!UALS_Settings::Get()->bDisableALSBindings)
        {
            GInputProcessor = MakeShared<FALSInputProcessor>();
            FSlateApplication::Get().RegisterInputPreProcessor(GInputProcessor);
        }
    }

    if (bAllowFileLog)
    {
        IConsoleManager::Get().RegisterConsoleCommand(
            TEXT("alslogs"),
            TEXT("Opens ALS Logs Viewer at runtime"),
            FConsoleCommandWithWorldDelegate::CreateLambda([this](UWorld* World)
                {
                    ShowLogWidget(World);
                }),
            ECVF_Default
        );

        UALS_FileLog::RotateOlderLogs();
        FWorldDelegates::OnStartGameInstance.AddStatic(&UALS_FileLog::OnStartGameInstance);
    }

    if (bAllowPropInspector)
    {
        IConsoleManager::Get().RegisterConsoleCommand(
            TEXT("alsproperty"),
            TEXT("Opens ALS Property Inspector at runtime"),
            FConsoleCommandWithWorldDelegate::CreateLambda([this](UWorld* World)
                {
                    ShowPropertyWidget(World);
                }),
            ECVF_Default
        );
    }
}

void FALSModule::ShutdownModule()
{
    IConsoleManager::Get().UnregisterConsoleObject(TEXT("alslogs"));
    IConsoleManager::Get().UnregisterConsoleObject(TEXT("alsproperty"));
}

void FALSModule::ShowLogWidget(UWorld* World)
{
    if (!World) return;

    UGameInstance* GameInstance = World->GetGameInstance();
    if (!GameInstance) return;

    APlayerController* PC = GameInstance->GetFirstLocalPlayerController();
    if (!PC) return;

    UALS_FunctionLibrary::OpenLogsViewer(Cast<UObject>(GameInstance), PC);
}

void FALSModule::ShowPropertyWidget(UWorld* World)
{
    if (!World) return;

    UGameInstance* GameInstance = World->GetGameInstance();
    if (!GameInstance) return;

    APlayerController* PC = GameInstance->GetFirstLocalPlayerController();
    if (!PC) return;

    UALS_FunctionLibrary::OpenPropertyInspector(Cast<UObject>(GameInstance), PC);
}

#undef LOCTEXT_NAMESPACE
IMPLEMENT_MODULE(FALSModule, ALS)