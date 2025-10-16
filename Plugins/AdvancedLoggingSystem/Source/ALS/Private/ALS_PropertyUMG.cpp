//Copyright © 2025 RTerofer. All Rights Reserved.

#include "ALS_PropertyUMG.h"
#include "ALS_Globals.h"
#include "ALS_EntryObjects.h"
#include "EngineUtils.h"    
#include "Engine/GameInstance.h"
#include "UObject/Package.h"
#include "Kismet/GameplayStatics.h"
#include "Components/ListView.h"
#include "Components/Overlay.h"
#include "Components/CanvasPanelSlot.h"


void UALS_PropertyUMG::NativeConstruct()
{
    Super::NativeConstruct();

    bool bAllowPropInspector = UALS_Settings::Get()->IsPropertyInspectorAllowed();

    if (!bAllowPropInspector)
    {
        RemoveFromParent();
    }
}

void UALS_PropertyUMG::GetPropertySettings(float& OutTimer, FSlateColor& OutContextColor, FSlateColor& OutPropertyColor, FSlateColor& OutMessageColor)
{
    OutTimer = UALS_Settings::Get()->RefreshTimer;
    OutContextColor = UALS_Settings::Get()->ContextColor;
    OutPropertyColor = UALS_Settings::Get()->PropertyColor;
    OutMessageColor = UALS_Settings::Get()->MessageColor;
}

TArray<AActor*> UALS_PropertyUMG::GetAllActorsInWorld()
{
    TArray<AActor*> GameActors;
    TArray<AActor*> EngineActors;

    for (TActorIterator<AActor> ActorIt(GetWorld(), AActor::StaticClass(), EActorIteratorFlags::SkipPendingKill | EActorIteratorFlags::AllActors); ActorIt; ++ActorIt)
    {
        UClass* ActorClass = ActorIt->GetClass();
        FString PackageName = ActorClass->GetOutermost()->GetName();

        if (PackageName.StartsWith(TEXT("/Script/")))
        {
            EngineActors.Add(*ActorIt);
        }
        else
        {
            GameActors.Add(*ActorIt);
        }
    }

    GameActors.Append(EngineActors);
    return GameActors;
}

TMap<UObject*, TArray<FProperty*>> UALS_PropertyUMG::GetAllPropertiesOfObject(UALS_PropWorldObject* PropWorldObject, const FString& FilterProperty, const bool& bIsInherited)
{
    TMap<UObject*, TArray<FProperty*>> OutProps;
    if (!PropWorldObject || !PropWorldObject->VarContext) return OutProps;

    auto GatherProps = [&](UObject* Obj) -> TArray<FProperty*>
        {
            if (!Obj) return {};

            TArray<FProperty*> FoundProperties;

            for (TFieldIterator<FProperty> It(Obj->GetClass(), EFieldIteratorFlags::IncludeSuper, EFieldIteratorFlags::DeprecatedPropertyFlags::ExcludeDeprecated); It; ++It)
            {
                FProperty* Property = *It;

                if (Property->GetFName() == TEXT("UberGraphFrame")) continue;

                if (FilterProperty.IsEmpty() || Property->GetName().Contains(FilterProperty))
                {
                    FoundProperties.Add(Property);
                }
            }

            return FoundProperties;
        };

    OutProps.Add(PropWorldObject->VarContext, GatherProps(PropWorldObject->VarContext));

    if (bIsInherited || !FilterProperty.IsEmpty())
    {
        if (AActor* Actor = Cast<AActor>(PropWorldObject->VarContext))
        {
            TArray<UActorComponent*> InnerComps; Actor->GetComponents(InnerComps);

            for (UActorComponent* InnerComp : InnerComps)
            {
                OutProps.Add(InnerComp, GatherProps(InnerComp));
            }
        }
    }

    return OutProps;
}

void UALS_PropertyUMG::SetWorldObjects(UListView* InObjectList, UListView* InMessageList, const FString& FilterObject, const FString& FilterProperty)
{
    TArray<AActor*> FoundActors(GetAllActorsInWorld());
    InObjectList->ClearListItems();

    TArray<UALS_PropWorldObject*> WorldObjects;

    for (AActor* Actor : FoundActors)
    {
        FString ContextString = Actor->GetName().Replace(TEXT("_C_"), TEXT(" #"));

        #if WITH_EDITOR
            if (UALS_Settings::Get()->UseActorLabel) ContextString = Actor->GetActorLabel();       
        #endif

        if (FilterObject.IsEmpty() || ContextString.Contains(FilterObject))
        {
            UALS_PropWorldObject* WorldObject = NewObject<UALS_PropWorldObject>();
            WorldObject->VarContext = Cast<UObject>(Actor);
            WorldObject->ObjectName = ContextString;
            WorldObject->MessageList = InMessageList;
            WorldObject->ObjectList = InObjectList;
            WorldObject->FilterProperty = FilterProperty;

            Actor->OnDestroyed.AddDynamic(WorldObject, &UALS_PropWorldObject::HandleActorDestroyed);
            WorldObjects.Add(WorldObject);
        }
    }

    UObject* GameInstance = GetGameInstance();

    if (FilterObject.IsEmpty() || GameInstance->GetName().Contains(FilterObject))
    {
        UALS_PropWorldObject* WorldObject = NewObject<UALS_PropWorldObject>();
        WorldObject->VarContext = GameInstance;
        WorldObject->ObjectName = GameInstance->GetName();
        WorldObjects.Add(WorldObject);
    }

    InObjectList->SetListItems(WorldObjects);
}

void UALS_PropertyUMG::SetVarObjects(UALS_PropWorldObject* PropWorldObject, UListView* InPropertyList, UExpandableArea* PropertyExpand, const bool& bIsInherited)
{
    if (!PropWorldObject) return;

    FString FilterProperty = PropWorldObject->FilterProperty;
    TMap<UObject*, TArray<FProperty*>> MappedProperties = GetAllPropertiesOfObject(PropWorldObject, FilterProperty, bIsInherited);

    InPropertyList->ClearListItems();

    TArray<UALS_PropVarObject*> VarObjects;

    for (const auto& Pair : MappedProperties)
    {
        UALS_PropVarObject* SubHeadObject = NewObject<UALS_PropVarObject>();
        SubHeadObject->PropertyName = Pair.Key->GetFName();
        SubHeadObject->IsSubHead = true;

        VarObjects.Add(SubHeadObject);

        for (FProperty* VarProperty : Pair.Value)
        {
            UALS_PropVarObject* VarObject = NewObject<UALS_PropVarObject>();
            VarObject->VarProperty = VarProperty;
            VarObject->PropertyName = VarProperty->GetFName();
            VarObject->VarContext = PropWorldObject->VarContext;
            VarObject->VarOwner = Pair.Key;
            VarObject->MessageList = PropWorldObject->MessageList;
            VarObject->IsSubHead = false;

            VarObjects.Add(VarObject);
        }
    }

    bool IsArrayEmpty = VarObjects.IsEmpty();
    this->SetVisibility(IsArrayEmpty ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);

    bool IsExpanded = (!IsArrayEmpty && !FilterProperty.IsEmpty());
    PropertyExpand->SetIsExpanded(IsExpanded);
    PropWorldObject->bIsInHerited = bIsInherited;

    InPropertyList->SetListItems(VarObjects);
}

bool UALS_PropertyUMG::SetMsgObject(UALS_PropVarObject* PropVarObject)
{
    if (!PropVarObject) return false;

    UALS_PropMsgObject* FoundMsgObject = DoesMsgObject(PropVarObject);
    if (FoundMsgObject)
    {
        PropVarObject->MessageList->RemoveItem(FoundMsgObject);
        return false;
    }

    TArray<UALS_PropMsgObject*> PropMsgObjects;
    UALS_PropMsgObject* MsgObject = NewObject<UALS_PropMsgObject>();
    MsgObject->VarContext = PropVarObject->VarContext;
    MsgObject->VarOwner = PropVarObject->VarOwner;
    MsgObject->VarProperty = PropVarObject->VarProperty;
    MsgObject->StartSubscription();

    PropVarObject->MessageList->AddItem(MsgObject);
    return true;
}

UALS_PropMsgObject* UALS_PropertyUMG::DoesMsgObject(UALS_PropVarObject* PropVarObject)
{
    if (!PropVarObject) return nullptr;

    TArray<UObject*> ExistingMsgObjects = PropVarObject->MessageList->GetListItems();
    for (UObject* ExistingObj : ExistingMsgObjects)
    {
        if (UALS_PropMsgObject* MsgObj = Cast<UALS_PropMsgObject>(ExistingObj))
        {
            if (FName(MsgObj->PropertyName) == PropVarObject->PropertyName && MsgObj->VarOwner == PropVarObject->VarOwner)
            {
                return MsgObj;
            }
        }
    }

    return nullptr;
}

