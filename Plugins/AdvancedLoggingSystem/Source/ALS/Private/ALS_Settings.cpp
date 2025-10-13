//Copyright © 2025 RTerofer. All Rights Reserved.

#include "ALS_Settings.h"

FName UALS_Settings::GetCategoryName() const
{
    return TEXT("Plugins");
}

const UALS_Settings* UALS_Settings::Get()
{ 
    return GetDefault<UALS_Settings>(); 
}

FPrintConfig UALS_Settings::GetConfigFromPreset(EPrintPreset Preset)
{
    FPrintConfig SelectedConfig;
    switch (Preset)
    {
    case EPrintPreset::PrintInfo:
        SelectedConfig = UALS_Settings::Get()->PrintInfoConfig;
        break;
    case EPrintPreset::PrintWarn:
        SelectedConfig = UALS_Settings::Get()->PrintWarnConfig;
        break;
    case EPrintPreset::PrintError:
        SelectedConfig = UALS_Settings::Get()->PrintErrorConfig;
        break;
    case EPrintPreset::LogInfo:
        SelectedConfig = UALS_Settings::Get()->LogInfoConfig;
        break;
    case EPrintPreset::LogWarn:
        SelectedConfig = UALS_Settings::Get()->LogWarnConfig;
        break;
    case EPrintPreset::LogError:
        SelectedConfig = UALS_Settings::Get()->LogErrorConfig;
        break;
    case EPrintPreset::Print3D:
        SelectedConfig = UALS_Settings::Get()->Print3DConfig;
        break;
    default:
        SelectedConfig = UALS_Settings::Get()->PrintInfoConfig;
        break;
    }

    return SelectedConfig;
}

bool UALS_Settings::IsFileLoggingAllowed()
{
#if UE_BUILD_SHIPPING
    return (UALS_Settings::Get()->bEnableFileLogInShipping && UALS_Settings::Get()->bEnableFileLog);
#else
    return UALS_Settings::Get()->bEnableFileLog;
#endif
}

bool UALS_Settings::IsPropertyInspectorAllowed()
{
#if UE_BUILD_SHIPPING
    return UALS_Settings::Get()->bEnableInspectorInShipping;
#else
    return true;
#endif
}
