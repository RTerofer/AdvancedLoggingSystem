//Copyright © 2025 RTerofer. All Rights Reserved.

#include "ALS_Globals.h"
#include "ALS_FileLog.h"
#include "GameplayTagContainer.h"

FString UALS_Globals::GetNetworkContextTag(const UObject* Context)
{
    if (Context)
    {
        if (const AActor* Actor = Cast<const AActor>(Context))
        {
            if (UWorld* World = Actor->GetWorld())
            {
                if (World)
                {
                    switch (World->GetNetMode())
                    {
                    case NM_Standalone: return TEXT("");
                    case NM_ListenServer: return TEXT("[Server] ");
                    case NM_Client: return TEXT("[Client] ");
                    case NM_DedicatedServer: return TEXT("[DedicatedServer] ");
                    }
                }
            }
        }
    }
    return TEXT("");
}

void UALS_Globals::ConvertToString_Direct(FProperty* Property, const void* ValuePtr, FStringBuilderBase& OutBuilder)
{
    FString ConvertedValue;
    Property->ExportTextItem_Direct(ConvertedValue, ValuePtr, nullptr, nullptr, PPF_None);
    OutBuilder.Append(ConvertedValue);
}

void UALS_Globals::ConvertToString_Byte(FByteProperty* ByteProperty, const void* BytePtr, FStringBuilderBase& OutBuilder)
{
    UEnum* EnumType = ByteProperty->Enum;
    uint8 ByteValue = ByteProperty->GetPropertyValue(BytePtr);

    if (EnumType)
    {
        FText EnumText = EnumType->GetDisplayNameTextByValue(ByteValue);
        OutBuilder.Append(EnumType->GetName());
        OutBuilder.Append(TEXT("::"));
        OutBuilder.Append(EnumText.ToString());
    }
    else
    {
        OutBuilder.Appendf(TEXT("%u"), ByteValue);
    }
}

void UALS_Globals::ConvertToString_Array(FScriptArrayHelper& ArrayHelper, FArrayProperty* ArrayProperty, FStringBuilderBase& OutBuilder)
{
    if (ArrayHelper.Num() == 0)
    {
        OutBuilder.Appendf(TEXT("No Valid Elements In %s"), *ArrayProperty->GetName());
        return;
    }

    for (int32 i = 0; i < ArrayHelper.Num(); i++)
    {
        void* ElementPtr = ArrayHelper.GetRawPtr(i);
        FProperty* ElementProperty = ArrayProperty->Inner;

        OutBuilder.Appendf(TEXT("\n%d -> "), i);
        ConvertToString_Property(ElementProperty, ElementPtr, OutBuilder);
    }
}

void UALS_Globals::ConvertToString_Set(FScriptSetHelper& SetHelper, FSetProperty* SetProperty, FStringBuilderBase& OutBuilder)
{
    if (SetHelper.Num() == 0)
    {
        OutBuilder.Appendf(TEXT("No Valid Elements In %s"), *SetProperty->GetName());
        return;
    }

    for (int32 i = 0; i < SetHelper.Num(); i++)
    {
        if (!SetHelper.IsValidIndex(i)) continue;

        void* ElementPtr = SetHelper.GetElementPtr(i);
        FProperty* ElementProperty = SetProperty->ElementProp;

        OutBuilder.Appendf(TEXT("\n%d -> "), i);
        ConvertToString_Property(ElementProperty, ElementPtr, OutBuilder);
    }
}

void UALS_Globals::ConvertToString_Map(FScriptMapHelper& MapHelper, FMapProperty* MapProperty, FStringBuilderBase& OutBuilder)
{
    if (MapHelper.Num() == 0)
    {
        OutBuilder.Appendf(TEXT("No Valid Elements In %s"), *MapProperty->GetName());
        return;
    }

    for (int32 i = 0; i < MapHelper.Num(); i++)
    {
        if (!MapHelper.IsValidIndex(i)) continue;

        void* KeyPtr = MapHelper.GetKeyPtr(i);
        void* ValuePtr = MapHelper.GetValuePtr(i);

        FProperty* KeyProperty = MapProperty->KeyProp;
        FProperty* ValueProperty = MapProperty->ValueProp;

        OutBuilder.Appendf(TEXT("\n%d -> [K: "), i);
        ConvertToString_Property(KeyProperty, KeyPtr, OutBuilder);
        OutBuilder.Append(TEXT(", V: "));
        ConvertToString_Property(ValueProperty, ValuePtr, OutBuilder);
        OutBuilder.Append(TEXT("]"));
    }
}

bool UALS_Globals::FormatInlineStruct(const UScriptStruct* StructType, const void* StructPtr, FStringBuilderBase& OutBuilder)
{
    if (StructType == TBaseStructure<FVector>::Get() ||
        StructType == TBaseStructure<FVector_NetQuantize>::Get() ||
        StructType == TBaseStructure<FVector_NetQuantize10>::Get() ||
        StructType == TBaseStructure<FVector_NetQuantize100>::Get() ||
        StructType == TBaseStructure<FVector_NetQuantizeNormal>::Get())
    {
        const FVector Vec = *static_cast<const FVector*>(StructPtr);
        OutBuilder.Appendf(TEXT("X: %.2f, Y: %.2f, Z: %.2f"), Vec.X, Vec.Y, Vec.Z);
        return true;
    }

    if (StructType == TBaseStructure<FVector2D>::Get())
    {
        const FVector2D Vec = *static_cast<const FVector2D*>(StructPtr);
        OutBuilder.Appendf(TEXT("X: %.2f, Y: %.2f"), Vec.X, Vec.Y);
        return true;
    }

    if (StructType == TBaseStructure<FVector4>::Get())
    {
        const FVector4 Vec = *static_cast<const FVector4*>(StructPtr);
        OutBuilder.Appendf(TEXT("X: %.2f, Y: %.2f, Z: %.2f, W: %.2f"), Vec.X, Vec.Y, Vec.Z, Vec.W);
        return true;
    }

    if (StructType == TBaseStructure<FRotator>::Get())
    {
        const FRotator Rot = *static_cast<const FRotator*>(StructPtr);
        OutBuilder.Appendf(TEXT("P: %.6f, Y: %.6f, R: %.6f"), Rot.Pitch, Rot.Yaw, Rot.Roll);
        return true;
    }

    if (StructType == TBaseStructure<FQuat>::Get())
    {
        const FQuat Quat = *static_cast<const FQuat*>(StructPtr);
        OutBuilder.Appendf(TEXT("X: %.6f, Y: %.6f, Z: %.6f, W: %.6f"), Quat.X, Quat.Y, Quat.Z, Quat.W);
        return true;
    }

    if (StructType == TBaseStructure<FTransform>::Get())
    {
        const FTransform Transform = *static_cast<const FTransform*>(StructPtr);
        const FVector Loc = Transform.GetLocation();
        const FRotator Rot = Transform.Rotator();
        const FVector Scale = Transform.GetScale3D();

        OutBuilder.Appendf(TEXT("[Location] X: %.3f, Y: %.3f, Z: %.3f -- [Rotation] P: %.6f, Y: %.6f, R: %.6f -- [Scale] X: %.3f, Y: %.3f, Z: %.3f"),
            Loc.X, Loc.Y, Loc.Z, Rot.Pitch, Rot.Yaw, Rot.Roll, Scale.X, Scale.Y, Scale.Z);
        return true;
    }

    if (StructType == TBaseStructure<FLinearColor>::Get())
    {
        const FLinearColor Color = *static_cast<const FLinearColor*>(StructPtr);
        OutBuilder.Appendf(TEXT("R: %.2f, G: %.2f, B: %.2f, A: %.2f"), Color.R, Color.G, Color.B, Color.A);
        return true;
    }

    if (StructType == TBaseStructure<FColor>::Get())
    {
        const FColor Color = *static_cast<const FColor*>(StructPtr);
        OutBuilder.Appendf(TEXT("R: %d, G: %d, B: %d, A: %d"), Color.R, Color.G, Color.B, Color.A);
        return true;
    }

    if (StructType == TBaseStructure<FIntPoint>::Get())
    {
        const FIntPoint Point = *static_cast<const FIntPoint*>(StructPtr);
        OutBuilder.Appendf(TEXT("X: %d, Y: %d"), Point.X, Point.Y);
        return true;
    }

    if (StructType == TBaseStructure<FIntVector>::Get())
    {
        const FIntVector Vec = *static_cast<const FIntVector*>(StructPtr);
        OutBuilder.Appendf(TEXT("X: %d, Y: %d, Z: %d"), Vec.X, Vec.Y, Vec.Z);
        return true;
    }

    if (StructType->GetName().Contains(TEXT("IntVector2")))
    {
        const FIntVector2& V = *reinterpret_cast<const FIntVector2*>(StructPtr);
        OutBuilder.Appendf(TEXT("X: %d, Y: %d"), V.X, V.Y);
        return true;
    }

    if (StructType->GetName().Contains(TEXT("IntVector4")))
    {
        const FIntVector4& V = *reinterpret_cast<const FIntVector4*>(StructPtr);
        OutBuilder.Appendf(TEXT("X: %d, Y: %d, Z: %d, W: %d"), V.X, V.Y, V.Z, V.W);
        return true;
    }

    if (StructType->GetName().Contains(TEXT("UintVector2")))
    {
        const FUintVector2& V = *reinterpret_cast<const FUintVector2*>(StructPtr);
        OutBuilder.Appendf(TEXT("X: %u, Y: %u"), V.X, V.Y);
        return true;
    }

    if (StructType->GetName().Contains(TEXT("UintVector3")))
    {
        const FUintVector3& V = *reinterpret_cast<const FUintVector3*>(StructPtr);
        OutBuilder.Appendf(TEXT("X: %u, Y: %u, Z: %u"), V.X, V.Y, V.Z);
        return true;
    }

    if (StructType->GetName().Contains(TEXT("UintVector4")))
    {
        const FUintVector4& V = *reinterpret_cast<const FUintVector4*>(StructPtr);
        OutBuilder.Appendf(TEXT("X: %u, Y: %u, Z: %u, W: %u"), V.X, V.Y, V.Z, V.W);
        return true;
    }

    if (StructType->GetName().Contains(TEXT("IntRect")))
    {
        const FIntRect R = *static_cast<const FIntRect*>(StructPtr);
        OutBuilder.Appendf(TEXT("Min:(%d,%d)  Max:(%d,%d)"),
            R.Min.X, R.Min.Y, R.Max.X, R.Max.Y);
        return true;
    }

    if (StructType->GetName().Contains(TEXT("BoxSphereBounds")))
    {
        const FBoxSphereBounds* B = reinterpret_cast<const FBoxSphereBounds*>(StructPtr);

        OutBuilder.Appendf(TEXT("Origin:[%.2f %.2f %.2f]  Extent:[%.2f %.2f %.2f]  Radius:%.2f"),
            B->Origin.X, B->Origin.Y, B->Origin.Z,
            B->BoxExtent.X, B->BoxExtent.Y, B->BoxExtent.Z,
            B->SphereRadius);
        return true;
    }
    if (StructType == TBaseStructure<FCollisionProfileName>::Get())
    {
        const FCollisionProfileName& N = *static_cast<const FCollisionProfileName*>(StructPtr);
        OutBuilder.Append(N.Name.ToString());
        return true;
    }

    if (StructType == TBaseStructure<FSoftObjectPath>::Get())
    {
        const FSoftObjectPath& P = *static_cast<const FSoftObjectPath*>(StructPtr);
        OutBuilder.Append(P.IsNull() ? TEXT("Null SoftObjectPath") : P.ToString());
        return true;
    }

    if (StructType == TBaseStructure<FSoftClassPath>::Get())
    {
        const FSoftClassPath& P = *static_cast<const FSoftClassPath*>(StructPtr);
        OutBuilder.Append(P.IsNull() ? TEXT("Null SoftClassPath") : P.ToString());
        return true;
    }

    if (StructType == TBaseStructure<FPrimaryAssetId>::Get())
    {
        const FPrimaryAssetId& Id = *static_cast<const FPrimaryAssetId*>(StructPtr);
        OutBuilder.Append(Id.IsValid() ? Id.ToString() : TEXT("Invalid PrimaryAssetId"));
        return true;
    }

    if (StructType == TBaseStructure<FPrimaryAssetType>::Get())
    {
        const FPrimaryAssetType& Ty = *static_cast<const FPrimaryAssetType*>(StructPtr);
        OutBuilder.Append(Ty.ToString());
        return true;
    }

    if (StructType == TBaseStructure<FGameplayTag>::Get())
    {
        const FGameplayTag& Tag = *static_cast<const FGameplayTag*>(StructPtr);
        OutBuilder.Append(Tag.IsValid() ? Tag.ToString() : TEXT("Invalid GameplayTag"));
        return true;
    }

    if (StructType == TBaseStructure<FGameplayTagContainer>::Get())
    {
        const FGameplayTagContainer& Tags = *static_cast<const FGameplayTagContainer*>(StructPtr);
        FString Tmp = Tags.ToStringSimple(true);
        OutBuilder.Append(Tmp.IsEmpty() ? TEXT("Empty TagContainer") : Tmp);
        return true;
    }

    if (StructType == TBaseStructure<FFloatRange>::Get())
    {
        const FFloatRange& R = *static_cast<const FFloatRange*>(StructPtr);
        OutBuilder.Appendf(TEXT("[%.3f – %.3f]"), R.GetLowerBoundValue(), R.GetUpperBoundValue());
        return true;
    }

    if (StructType == TBaseStructure<FInt32Range>::Get())
    {
        const FInt32Range& R = *static_cast<const FInt32Range*>(StructPtr);
        OutBuilder.Appendf(TEXT("[%d – %d]"), R.GetLowerBoundValue(), R.GetUpperBoundValue());
        return true;
    }

    if (StructType == TBaseStructure<FDateTime>::Get())
    {
        const FDateTime DT = *static_cast<const FDateTime*>(StructPtr);

        if (DT.GetTicks() == 0)
        {
            OutBuilder.Append(DT.ToString());
        }
        else
        {
            FString Base = DT.ToFormattedString(TEXT("%d-%m-%y %H:%M:%S"));
            int32 MS = DT.GetMillisecond();
            OutBuilder.Appendf(TEXT("%s.%03d"), *Base, MS);
        }
        return true;
    }

    if (StructType->GetName().Contains("Timespan"))
    {
        const FTimespan TS = *static_cast<const FTimespan*>(StructPtr);

        if (TS.GetTicks() == 0)
        {
            OutBuilder.Append(TS.ToString());
        }
        else
        {
            FString Base = FString::Printf(TEXT("%d - %02d:%02d:%02d"),
                TS.GetDays(), TS.GetHours(), TS.GetMinutes(), TS.GetSeconds());
            int32 MS = static_cast<int32>(TS.GetTotalMilliseconds()) % 1000;
            OutBuilder.Appendf(TEXT("%s.%03d"), *Base, MS);
        }
        return true;
    }

    if (StructType == TBaseStructure<FGuid>::Get())
    {
        const FGuid Guid = *static_cast<const FGuid*>(StructPtr);
        OutBuilder.Append(Guid.ToString());
        return true;
    }

    return false;
}

void UALS_Globals::ConvertToString_Struct(const UScriptStruct* StructType, const void* StructPtr, FStringBuilderBase& OutBuilder)
{
    if (!StructType || !StructPtr)
    {
        OutBuilder.Append(TEXT("Invalid Struct"));
        return;
    }

    if (FormatInlineStruct(StructType, StructPtr, OutBuilder))
    {
        return;
    }

    bool bHasValues = false;

    for (TFieldIterator<FProperty> PropIt(StructType); PropIt; ++PropIt)
    {
        FProperty* Property = *PropIt;
        const void* ValuePtr = Property->ContainerPtrToValuePtr<const void>(StructPtr);

        OutBuilder.Append(TEXT("\n"));
        OutBuilder.Append(Property->GetAuthoredName());
        OutBuilder.Append(TEXT(":- "));
        ConvertToString_Property(Property, ValuePtr, OutBuilder);
        bHasValues = true;
    }

    if (!bHasValues)
    {
        OutBuilder.Appendf(TEXT("Unable to convert %s, Try to Break the struct & Print each property"), *StructType->GetName());
    }
}

void UALS_Globals::ConvertToString_Property(FProperty* Property, const void* ValuePtr, FStringBuilderBase& OutBuilder)
{
    if (!Property)
    {
        OutBuilder.Append(TEXT("[Invalid Property]"));
        return;
    }

    if (FByteProperty* ByteProperty = CastField<FByteProperty>(Property))
    {
        ConvertToString_Byte(ByteProperty, ValuePtr, OutBuilder);
    }
    else if (FStructProperty* StructProp = CastField<FStructProperty>(Property))
    {
        ConvertToString_Struct(StructProp->Struct, ValuePtr, OutBuilder);
    }
    else if (FArrayProperty* ArrayProp = CastField<FArrayProperty>(Property))
    {
        FScriptArrayHelper ArrayHelper(ArrayProp, ValuePtr);
        ConvertToString_Array(ArrayHelper, ArrayProp, OutBuilder);
    }
    else if (FSetProperty* SetProp = CastField<FSetProperty>(Property))
    {
        FScriptSetHelper SetHelper(SetProp, ValuePtr);
        ConvertToString_Set(SetHelper, SetProp, OutBuilder);
    }
    else if (FMapProperty* MapProp = CastField<FMapProperty>(Property))
    {
        FScriptMapHelper MapHelper(MapProp, ValuePtr);
        ConvertToString_Map(MapHelper, MapProp, OutBuilder);
    }
    else if (FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Property))
    {
        UObject* ObjectValue = ObjectProperty->GetObjectPropertyValue(ValuePtr);
        OutBuilder.Append(ObjectValue ? ObjectValue->GetName() : TEXT("null_object"));
    } 
    else if (FSoftObjectProperty* SoftProp = CastField<FSoftObjectProperty>(Property))
    {
        FSoftObjectPtr SoftObject = SoftProp->GetPropertyValue(ValuePtr);
        OutBuilder.Append(SoftObject.IsValid() ? SoftObject.Get()->GetName() : TEXT("null_soft_object"));
    }
    else if (FInterfaceProperty* InterfaceProp = CastField<FInterfaceProperty>(Property))
    {
        FScriptInterface ScriptInterface = InterfaceProp->GetPropertyValue(ValuePtr);
        UObject* Obj = ScriptInterface.GetObject();
        OutBuilder.Append(Obj ? Obj->GetName() : TEXT("null_interface"));
    }
    else if (FTextProperty* TextProp = CastField<FTextProperty>(Property))
    {
        const FText* TextValue = TextProp->GetPropertyValuePtr(ValuePtr);
        OutBuilder.Append(TextValue->ToString());
    }
    else
    {
        ConvertToString_Direct(Property, ValuePtr, OutBuilder);
    }
}


//---------------------------------------------------------------------------------------------------------------------------------

void UALS_Globals::LogOutput(const FString& Value, ELogSeverity Level)
{
    switch (Level)
    {
    case ELogSeverity::Info:
        UE_LOG(LogALS, Display, TEXT("%s"), *Value);
        break;

    case ELogSeverity::Warning:

        UE_LOG(LogALS, Warning, TEXT("%s"), *Value);
        break;

    case ELogSeverity::Error:
        UE_LOG(LogALS, Error, TEXT("%s"), *Value);
        break;
    }
}

void UALS_Globals::GetContextAndNetwork(const UObject* InContext, FString& OutCaller, FString& OutNetwork)
{
    if (InContext)
    {
        OutCaller = InContext->GetName().Replace(TEXT("_C_"), TEXT(" #"));
        OutNetwork = GetNetworkContextTag(InContext);

        OutCaller = FString::Printf(TEXT("%s[%s]"), *OutNetwork, *OutCaller);
    }
    else
    {
        OutCaller = FString(TEXT("[NoContext]"));
    }

    return;
}

void UALS_Globals::PrintALS(
    const FString& Value, 
    const FPrintConfig& PrintConfig, 
    const UObject* Context, 
    const FString& SourceID,
    bool InitiateFileLog
)
{
    FColor PrintColor = PrintConfig.Color;

    FString Caller;
    FString Network;
    GetContextAndNetwork(Context, Caller, Network);

    FString ValueWithContextAndNetwork = FString::Printf(TEXT("%s %s"), *Caller, *Value);
    FString ValueWithNetwork = FString::Printf(TEXT("%s%s"), *Network, *Value);

    FString Screen = UALS_Settings::Get()->bShowCallerName ? ValueWithContextAndNetwork : ValueWithNetwork;
    FString Log = ValueWithContextAndNetwork;

    if (PrintConfig.PrintMode == EPrintMode::ScreenOnly || PrintConfig.PrintMode == EPrintMode::ScreenAndLog)
    {
        uint64 InnerKey = -1;
        if (PrintConfig.Key != NAME_None)
        {
            InnerKey = GetTypeHash(PrintConfig.Key);
        }

        GEngine->AddOnScreenDebugMessage(InnerKey, PrintConfig.Duration, PrintColor, *Screen);
    }

    if (InitiateFileLog)
    {
        if (PrintConfig.PrintMode == EPrintMode::LogOnly || PrintConfig.PrintMode == EPrintMode::ScreenAndLog)
        {
            LogOutput(Log, PrintConfig.LogSeverity);
        }

        UALS_FileLog::CreateMessageLog(Context, SourceID, Value, PrintConfig.LogSeverity);      
    }
}

void UALS_Globals::DrawALS(
    const FString& Value, 
    const UObject* BaseObject, 
    const FVector& TextLocation, 
    const FPrintConfig& PrintConfig, 
    const UObject* Context, 
    const FString& SourceID,
    bool InitiateFileLog)
{
    UWorld* World = nullptr;

    if (Context)
    {
        World = Context->GetWorld();
    }

    if (!World && GEngine)
    {
        World = GEngine->GetCurrentPlayWorld();
        UE_LOG(LogALS, Warning, TEXT("PrintDraw: No valid world found from Context. Tried to use CurrentPlayWorld."));
    }   

    if (!World)
    {
        UE_LOG(LogALS, Error, TEXT("PrintDraw: No valid world found. Skipping debug draw."));
        return;
    }

    FVector FinalLocation = TextLocation;

    if (BaseObject)
    {
        if (const AActor* DebugActor = Cast<AActor>(BaseObject))
        {
            FinalLocation = DebugActor->GetActorLocation() + FinalLocation;
        }
        else if (const USceneComponent* SceneComp = Cast<USceneComponent>(BaseObject))
        {
            FinalLocation = SceneComp->GetComponentLocation() + FinalLocation;
        }
    }

    FString Caller;
    FString Network;
    GetContextAndNetwork(Context, Caller, Network);

    FString ValueWithContextAndNetwork = FString::Printf(TEXT("%s %s"), *Caller, *Value);
    FString ValueWithNetwork = FString::Printf(TEXT("%s%s"), *Network, *Value);

    FString Screen = UALS_Settings::Get()->bShowCallerName ? ValueWithContextAndNetwork : ValueWithNetwork;
    FString Log = ValueWithContextAndNetwork;

    DrawDebugString(
        World,
        FinalLocation,
        Screen,
        nullptr,
        PrintConfig.Color,
        PrintConfig.Duration,
        true
    );

    if (InitiateFileLog)
    {
        if (PrintConfig.PrintMode == EPrintMode::LogOnly || PrintConfig.PrintMode == EPrintMode::ScreenAndLog)
        {
            LogOutput(Log, PrintConfig.LogSeverity);
        }

        UALS_FileLog::CreateMessageLog(Context, SourceID, Value, PrintConfig.LogSeverity);
    }
}