//Copyright © 2025 RTerofer. All Rights Reserved.

#include "ALS_InputProcessor.h"
#include "ALS_FunctionLibrary.h"
#include "ALS_Settings.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "InputCoreTypes.h"
#include "Framework/Application/SlateApplication.h"

FALSInputProcessor::FALSInputProcessor()
{
    LogsChord = UALS_Settings::Get()->LogToggleChord;
    PropertyChord = UALS_Settings::Get()->PropertyToggleChord;
}

bool FALSInputProcessor::HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& KeyEvent)
{
    UWorld* World = nullptr;

    TSharedPtr<SWindow> ActiveWindow = SlateApp.GetActiveTopLevelWindow();
    for (const FWorldContext& Context : GEngine->GetWorldContexts())
    {
        if (UWorld* TargetWorld = Context.World())
        {
            if (TargetWorld->IsGameWorld() && TargetWorld->GetNetMode() != ENetMode::NM_DedicatedServer)
            {
                if (UGameViewportClient* VP = GEngine->GameViewportForWorld(TargetWorld))
                {
                    TSharedPtr<SWindow> VPWindow = VP->GetWindow();
                    if (VPWindow.IsValid() && ActiveWindow.IsValid() && (VPWindow == ActiveWindow))
                    {
                        World = TargetWorld;
                        break;
                    }
                }
            }
        }
    }

    if (!World)
    {
        World = GEngine->GetCurrentPlayWorld();
    }

    if (!World) return false;

    UGameInstance* GameInstance = World->GetGameInstance();
    if (!GameInstance) return false;

    const int32 UserIdx = KeyEvent.GetUserIndex();
    APlayerController* PC = GameInstance->GetLocalPlayerByIndex(UserIdx) ? GameInstance->GetLocalPlayerByIndex(UserIdx)->GetPlayerController(World) : nullptr;
    if (!PC) return false;

    if (!UALS_Settings::Get()->bDisableALSBindings)
    {
        if (DoesChordMatch(LogsChord, KeyEvent))
        {
            UALS_FunctionLibrary::OpenLogsViewer(Cast<UObject>(GameInstance), PC);
            return true;
        }

        if (DoesChordMatch(PropertyChord, KeyEvent))
        {
            UALS_FunctionLibrary::OpenPropertyInspector(Cast<UObject>(GameInstance), PC);
            return true;
        }
    }

    return false;
}

bool FALSInputProcessor::DoesChordMatch(const FInputChord& InChord, const FKeyEvent& InEvent) const
{
    return InEvent.GetKey() == InChord.Key
        && InEvent.IsControlDown() == InChord.bCtrl
        && InEvent.IsAltDown() == InChord.bAlt
        && InEvent.IsShiftDown() == InChord.bShift
        && InEvent.IsCommandDown() == InChord.bCmd;
}
