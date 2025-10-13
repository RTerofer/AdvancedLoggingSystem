//Copyright © 2025 RTerofer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Modules/ModuleManager.h"

class ALS_API FALSModule : public IModuleInterface
{

protected:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

public:
    static FALSModule& Get()
    {
        return FModuleManager::LoadModuleChecked<FALSModule>("ALS");
    }

    void ShowLogWidget(UWorld* World);
    void ShowPropertyWidget(UWorld* World);
};