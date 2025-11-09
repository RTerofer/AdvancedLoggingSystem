//Copyright © 2025 RTerofer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ALS_MacroExample.generated.h"

UENUM(BlueprintType)
enum class EConsumables : uint8
{
	Apple   UMETA(DisplayName = "Apple"),
	Banana  UMETA(DisplayName = "Banana"),
	Cherry  UMETA(DisplayName = "Cherry"),
};

USTRUCT(BlueprintType)
struct FPlayerData
{
	GENERATED_BODY()

	UPROPERTY()
	FString PlayerName = "DefaultName";

	UPROPERTY()
	float Health = 0.0f;

	FPlayerData() : PlayerName(""), Health(0.0f) {}

	FPlayerData(FString InName, float InHealth) : PlayerName(InName), Health(InHealth) {}
};


UCLASS(BlueprintType)
class ALS_HOST_API AALS_MacroExample : public AActor
{
	GENERATED_BODY()
	
public:	
	AALS_MacroExample();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

protected:
	UFUNCTION(BlueprintCallable, meta = (Category = "ALS_MacroExample"))
	void MacroPresetExample();

	UFUNCTION(BlueprintCallable, meta = (Category = "ALS_MacroExample"))
	void MacroCustomExample();	
	
	UFUNCTION(BlueprintCallable, meta = (Category = "ALS_MacroExample"))
	void MacroFileExample();

	UFUNCTION(BlueprintCallable, meta = (Category = "ALS_MacroExample"))
	void MacroTestingExample();
};
