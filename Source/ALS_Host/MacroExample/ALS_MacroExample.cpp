//Copyright © 2025 RTerofer. All Rights Reserved.

#include "ALS_MacroExample.h"
#include "Kismet/GameplayStatics.h"
#include "ALS_Macro.h"

AALS_MacroExample::AALS_MacroExample()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AALS_MacroExample::BeginPlay()
{
	Super::BeginPlay();
}

void AALS_MacroExample::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}



void AALS_MacroExample::MacroCustomExample()
{
	// DEFINE CONFIG - (Key, Duration, Color, LogSeverity, PrintMode)
	FPrintConfig MyPrintConfig(NAME_None, 4.0f, FColor::Blue, ELogSeverity::Info, EPrintMode::ScreenOnly);

	// USE CUSTOM CONFIG - (PrintConfig, Args...)
	PrintCustom(MyPrintConfig, "Hello World");

	// USE EXISTING PRESET - (Preset, Args...)
	PrintPreset(EPrintPreset::PrintInfo, "Hello World");

	// USE IN STATIC FUNCTION FOR PRINT AND LOG - (Key, Duration, Color, Args...)
	JustPrint(NAME_None, 2.0f, FColor::Yellow, "Hello World");

	// USE IN STATIC FUNCTION FOR LOG - (LogSeverity, Args...)
	JustLog(ELogSeverity::Warning, "Hello World");
}



void AALS_MacroExample::MacroPresetExample()
{
	// MAP of Consumable and its health potion
	TMap<EConsumables, float> ConsumableMap =
	{
		{ EConsumables::Apple, 30.0f },
		{ EConsumables::Banana, 50.0f }
	};

	// UENUM()
	EConsumables EquippedConsumable = EConsumables::Banana;  

	// USTRUCT()
	FPlayerData PlayerData = FPlayerData("RTerofer", 15.7f);  

	// Print Info
	PrintInfo("All Consumables: ", ConsumableMap);

	// Print Warn
	PrintWarn(PlayerData.PlayerName, " has low health: ", PlayerData.Health);

	// Print Error
	PrintError("Cherry Not Found: ", ConsumableMap.Find(EConsumables::Cherry));

	// Print 3D
	Print3D(GetActorLocation(), "Player Locate: ", this);
}



void AALS_MacroExample::MacroCustomFile()
{
	ALSFileLog("File Message From CPP", ELogSeverity::Info);
}



void AALS_MacroExample::MacroSomeExample()
{
	TArray<int32> ObjArray;
	AActor* HitActor = this;
	AActor* OverlappedActor = this;
	TMap<EConsumables, int32> MyConsumableMap;
	TWeakObjectPtr<AActor> WeakObj = HitActor;


	// Print and Log Info - (Args...)
	PrintInfo("Location:", GetActorLocation());

	// Print and Log Warn - (Args...)
	PrintWarn("Consumable is added to inventory: ", EConsumables::Banana);

	//Print and Log Error - (Args...)
	PrintError("Error: WeakObjPtr is not valid", WeakObj);

	// Log Info - (Args...)
	LogInfo("My Struct:", FPlayerData(), "\nMyEnum:", EConsumables::Banana);

	// Log Warn - (Args...)
	LogWarn("HitActor: ", HitActor, "\nForwardVec: ", HitActor->GetActorForwardVector());

	// Log Error  - (Args...)
	LogError((MyConsumableMap.Find(EConsumables::Apple) ? "Found" : "NotFound"));

	// Print To World and Log - (Location, Args...)
	Print3D(GetActorLocation(), "PlayerName: ", OverlappedActor);

}



