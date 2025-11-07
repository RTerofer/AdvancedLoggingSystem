//Copyright © 2025 RTerofer. All Rights Reserved.

#pragma once

#include "ALS_Globals.h"
#include "ALS_Settings.h"

// --> This is the unique source id such as (MyActor.cpp:145). Useful to batch unique message in LogViewer when similar messages are called.
#define SOURCE_ID (FString(FPaths::GetCleanFilename(__FILE__)) + TEXT(":") + FString::FromInt(__LINE__))

// -- Get Configs --
#define GETCONFIG(Config) UALS_Settings::GetConfigFromPreset(Config)

#define PrintInfoPreset GETCONFIG(EPrintPreset::PrintInfo)
#define PrintWarnPreset GETCONFIG(EPrintPreset::PrintWarn)
#define PrintErrorPreset GETCONFIG(EPrintPreset::PrintError)
#define LogInfoPreset GETCONFIG(EPrintPreset::LogInfo)
#define LogWarnPreset GETCONFIG(EPrintPreset::LogWarn)
#define LogErrorPreset GETCONFIG(EPrintPreset::LogError)
#define Print3DPreset GETCONFIG(EPrintPreset::Print3D)

//----------------------------------------------------------------------------------------------------------------------
//                ALS C++ MACROS
//----------------------------------------------------------------------------------------------------------------------


// -- Print & Log Macros --  Example:- PrintInfo("Overlapped: ", ActorPointer, " IsMovable: ", RootComp->Mobility);
//----------------------------------------------------------------------------------------------------------------------

#define PrintInfo(...)  UALS_Globals::PrintALSCPP(PrintInfoPreset, this, SOURCE_ID, __VA_ARGS__)
#define PrintWarn(...)  UALS_Globals::PrintALSCPP(PrintWarnPreset, this, SOURCE_ID, __VA_ARGS__)
#define PrintError(...)  UALS_Globals::PrintALSCPP(PrintErrorPreset, this, SOURCE_ID, __VA_ARGS__)



// -- Log Macros --  Example:- LogInfo("MyStruct: ", Struct, "MyArray: ", Array);
//----------------------------------------------------------------------------------------------------------------------

#define LogInfo(...)  UALS_Globals::PrintALSCPP(LogInfoPreset, this, SOURCE_ID, __VA_ARGS__)
#define LogWarn(...)  UALS_Globals::PrintALSCPP(LogWarnPreset, this, SOURCE_ID, __VA_ARGS__)
#define LogError(...) UALS_Globals::PrintALSCPP(LogErrorPreset, this, SOURCE_ID, __VA_ARGS__)



// -- Print To World Macros --  Example:- Print3D(HitLocation, "HitBy: ", EnemyActor);
//----------------------------------------------------------------------------------------------------------------------

#define Print3D(Location, ...) UALS_Globals::DrawALSCPP(Print3DPreset, this, SOURCE_ID, Location, __VA_ARGS__)



// -- CUSTOM MACROS --  Example:- JustPrint(FColor::Blue, "Your Message From Static Function");
//----------------------------------------------------------------------------------------------------------------------

#define PrintCustom(PrintConfig, ...) UALS_Globals::PrintALSCPP(PrintConfig, this, SOURCE_ID, __VA_ARGS__)

#define LogCustom(PrintConfig, ...) UALS_Globals::PrintALSCPP(PrintConfig, this, SOURCE_ID, __VA_ARGS__)

#define Print3DCustom(PrintConfig, Location, ...) UALS_Globals::DrawALSCPP(PrintConfig, this, SOURCE_ID, Location, __VA_ARGS__)

#define PrintPreset(Preset, ...)  UALS_Globals::PrintALSCPP(GETCONFIG(Preset), this, SOURCE_ID, __VA_ARGS__)

#define LogPreset(Preset, ...)  UALS_Globals::PrintALSCPP(GETCONFIG(Preset), this, SOURCE_ID, __VA_ARGS__)

#define JustPrint(Key, Duration, Color, ...)  UALS_Globals::PrintALSCPP(FPrintConfig(Key, Duration, Color, ELogSeverity::Info, EPrintMode::ScreenOnly), nullptr, SOURCE_ID, __VA_ARGS__)

#define JustLog(Level, ...) UALS_Globals::PrintALSCPP(FPrintConfig(NAME_None, 0.0f, FColor::White, Level, EPrintMode::LogOnly), nullptr, SOURCE_ID, __VA_ARGS__)

#define ALSFileLog(Message, LogSeverity)  UALS_FileLog::CreateMessageLog(this, SOURCE_ID, Message, LogSeverity)

#define ConvertToStringALS(Value)  UALS_Globals::ConvertToStringCPP(Value)