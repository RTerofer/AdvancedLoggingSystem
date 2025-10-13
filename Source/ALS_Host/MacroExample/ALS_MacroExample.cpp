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
	// Custom Config
	FPrintConfig MyConfig = FPrintConfig(
		FColor::Blue,          // Color
		4.0f,                  // Duration
		ELogSeverity::Info,    // Severity
		EPrintMode::ScreenOnly // Print Mode
	);

	// Config + Message
	PrintCustom(MyConfig, "Print With Custom Config");

	// Color + Duration + Message. [Use in Static Functions]
	JustPrint(FColor::Yellow, 2.0f, "Just Print");

	// LogSeverity + Message. [Use in Static Functions]
	JustLog(ELogSeverity::Warning, "Just Log");

	// Preset + Context + Message 
	PrintWithCtx(EPrintPreset::PrintInfo, this, "Print With Preset & Context");
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
	TMap<int32, FString> MyMap;


	// Print Info
	PrintInfo("Location:", GetActorLocation());

	// Print Warn
	PrintWarn("Warn:", this, "has low health");

	//Print Error
	PrintError("Error: No Valid elements: ", ObjArray.Num());

	// Log Info
	LogInfo("My Struct:", FPlayerData(), "\nMyEnum:", EConsumables::Banana);

	// Log Warn
	LogWarn("HitActor: ", HitActor, "\nForwardVec:", HitActor->GetActorForwardVector());

	// Log Error
	LogError((MyMap.Find(1) ? "Value Found" : "Couldn't Find Value"));

	// Print To World
	Print3D(this, "PlayerName: ", OverlappedActor);

}



