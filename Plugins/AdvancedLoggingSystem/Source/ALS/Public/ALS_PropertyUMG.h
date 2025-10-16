//Copyright © 2025 RTerofer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ALS_Definitions.h"
#include "UObject/UnrealType.h"
#include "Blueprint/UserWidget.h"
#include "Components/ListView.h"
#include "Components/ExpandableArea.h"
#include "ALS_PropertyUMG.generated.h"

class UALS_PropWorldObject;
class UALS_PropVarObject;
class UALS_PropMsgObject;

UCLASS(meta = (DisplayName = "ALS PropertyInspector"))
class ALS_API UALS_PropertyUMG : public UUserWidget
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ALS PropertyInspector")
    FString CurrentMessage = "CurrentMessage";

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ALS PropertyInspector")
    FString PreviousMessage = "PreviousMessage";

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ALS PropertyInspector")
    UExpandableArea* PropertyWindow = nullptr;

protected:

    TArray<AActor*> GetAllActorsInWorld();
    TMap<UObject*, TArray<FProperty*>> GetAllPropertiesOfObject(UALS_PropWorldObject* PropWorldObject, const FString& FilterProperty, const bool& bIsInherited);

    virtual void NativeConstruct() override;
    
    UFUNCTION(BlueprintCallable, Category = "ALS PropertyInspector")
    void GetPropertySettings(float& OutTimer, FSlateColor& OutContextColor, FSlateColor& OutPropertyColor, FSlateColor& OutMessageColor);

    UFUNCTION(BlueprintCallable, Category = "ALS PropertyInspector")
    void SetWorldObjects(UListView* InObjectList, UListView* InMessageList, const FString& FilterObject, const FString& FilterProperty);

    UFUNCTION(BlueprintCallable, Category = "ALS PropertyInspector")
    void SetVarObjects(UALS_PropWorldObject* PropWorldObject, UListView* InPropertyList, UExpandableArea* PropertyExpand, const bool& bIsInherited);

    UFUNCTION(BlueprintCallable, Category = "ALS PropertyInspector")
    bool SetMsgObject(UALS_PropVarObject* PropVarObject);

    UFUNCTION(BlueprintCallable, Category = "ALS PropertyInspector")
    UALS_PropMsgObject* DoesMsgObject(UALS_PropVarObject* PropVarObject);
};

