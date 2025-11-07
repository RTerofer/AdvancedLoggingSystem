//Copyright © 2025 RTerofer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "string"
#include "vector"
#include "utility"
#include "type_traits"
#include "ALS_FileLog.h"
#include "ALS_Definitions.h"
#include "ALS_Settings.h"
#include "DrawDebugHelpers.h"
#include "GameplayTagContainer.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

template <typename, typename = std::void_t<>>
struct TIsUStruct : std::false_type {};
template <typename T>
struct TIsUStruct<T, std::void_t<decltype(T::StaticStruct)>> : std::true_type {};
template <typename T>
constexpr bool TIsUStruct_V = TIsUStruct<T>::value;


class ALS_API UALS_Globals
{
public:
    static FString GetNetworkContextTag(const UObject* Context);
    static void ConvertToString_Property(FProperty* Property, const void* ValuePtr, FStringBuilderBase& OutBuilder);

private:
    static void ConvertToString_Direct(FProperty* Property, const void* ValuePtr, FStringBuilderBase& OutBuilder);
    static void ConvertToString_Byte(FByteProperty* ByteProperty, const void* BytePtr, FStringBuilderBase& OutBuilder);
    static void ConvertToString_Array(FScriptArrayHelper& ArrayHelper, FArrayProperty* ArrayProperty, FStringBuilderBase& OutBuilder);
    static void ConvertToString_Set(FScriptSetHelper& SetHelper, FSetProperty* SetProperty, FStringBuilderBase& OutBuilder);
    static void ConvertToString_Map(FScriptMapHelper& MapHelper, FMapProperty* MapProperty, FStringBuilderBase& OutBuilder);
    static bool FormatInlineStruct(const UScriptStruct* StructType, const void* StructPtr, FStringBuilderBase& OutBuilder);
    static void ConvertToString_Struct(const UScriptStruct* StructType, const void* StructPtr, FStringBuilderBase& OutBuilder);

    //---------------------------------------------------------------------------------------------------------------------------------

public:
    static void LogOutput(const FString& Value, ELogSeverity Level);
    static void GetContextAndNetwork(const UObject* InContext, FString& OutCaller, FString& OutNetwork);

public:
    static void PrintALS(
        const FString& Value, 
        const FPrintConfig& PrintConfig, 
        const UObject* Context, 
        const FString& SourceID,
        bool InitiateFileLog = true
    );

    static void DrawALS(
        const FString& Value, 
        const UObject* BaseObject, 
        const FVector& TextLocation, 
        const FPrintConfig& PrintConfig, 
        const UObject* Context, 
        const FString& SourceID,
        bool InitiateFileLog = true
    );

    static inline FString GetDisplayNameSafe(UObject* Object)
    {
        return IsValid(Object) ? UKismetSystemLibrary::GetDisplayName(Object) : TEXT("Null Object");
    }

//---------------------------------------------------------------------------------------------------------------------------------

    template<typename> struct TALS_IsWeakObjectPtr : std::false_type {};
    template<typename U> struct TALS_IsWeakObjectPtr<TWeakObjectPtr<U>> : std::true_type {};

    template<typename> struct TALS_IsSubclassOf : std::false_type {};
    template<typename U> struct TALS_IsSubclassOf<TSubclassOf<U>> : std::true_type {};

    template<typename> struct TALS_IsSoftClassPtr : std::false_type {};
    template<typename U> struct TALS_IsSoftClassPtr<TSoftClassPtr<U>> : std::true_type {};

    template<typename> struct TALS_IsSoftObjectPtr : std::false_type {};
    template<typename U> struct TALS_IsSoftObjectPtr<TSoftObjectPtr<U>> : std::true_type {};

    template<typename> struct TALS_IsSharedPtr : std::false_type {};
    template<typename U> struct TALS_IsSharedPtr<TSharedPtr<U>> : std::true_type {};

    template<typename> struct TALS_IsSharedRef : std::false_type {};
    template<typename U> struct TALS_IsSharedRef<TSharedRef<U>> : std::true_type {};

    template<typename> struct TALS_IsUniquePtr : std::false_type {};
    template<typename U> struct TALS_IsUniquePtr<TUniquePtr<U>> : std::true_type {};

    template<typename> struct TALS_IsOptional : std::false_type {};
    template<typename U> struct TALS_IsOptional<TOptional<U>> : std::true_type {};

    template<typename> struct TIsStdVector : std::false_type {};
    template<typename T, typename A> struct TIsStdVector<std::vector<T, A>> : std::true_type {};

    template<typename> struct TIsEnumAsByte : std::false_type {};
    template<typename E> struct TIsEnumAsByte<TEnumAsByte<E>> : std::true_type {};
    
    template<typename> struct TIsUnderlyingEnum;
    template<typename E> struct TIsUnderlyingEnum<TEnumAsByte<E>> { using Type = E; };

    template<typename> struct TIsStdPair : std::false_type {};
    template<typename K, typename V> struct TIsStdPair<std::pair<K, V>> : std::true_type {};

public:
    template<typename T>
    static FString ConvertToStringCPP(const T& InValue)
    {
        using U = std::decay_t<T>;

        constexpr bool bIsEnum = std::is_enum_v<U>;
        constexpr bool bIsEnumAsByte = TIsEnumAsByte<U>::value;
        constexpr bool bIsArray = TIsTArray<U>::Value;
        constexpr bool bIsSet = TIsTSet<U>::Value;
        constexpr bool bIsMap = TIsTMap<U>::Value;
        constexpr bool bIsPair = TIsStdPair<U>::value;
        constexpr bool bIsStruct = TIsUStruct_V<U>;
        constexpr bool bIsStdVector = TIsStdVector<U>::value;
        constexpr bool bIsBool = std::is_same_v<U, bool>;
        constexpr bool bIsArithmetic = std::is_arithmetic_v<U> && !bIsBool;
        constexpr bool bIsCString = std::is_same_v<U, const TCHAR*> || std::is_same_v<U, TCHAR*>;
        constexpr bool bIsStdString = std::is_same_v<U, std::string> || std::is_same_v<U, std::wstring>;
        constexpr bool bIsFString = std::is_same_v<U, FString>;
        constexpr bool bIsFName = std::is_same_v<U, FName>;
        constexpr bool bIsFText = std::is_same_v<U, FText>;
        constexpr bool bIsCharArray = std::is_array_v<T> && std::is_same_v<std::remove_extent_t<T>, char>;
        constexpr bool bIsString = bIsCString || bIsStdString || bIsFString || bIsFName || bIsFText || bIsCharArray;

        // Smart Pointer Conversions
        constexpr bool bIsWeakPtr = TALS_IsWeakObjectPtr<U>::value;
        constexpr bool bIsSubclass = TALS_IsSubclassOf<U>::value;
        constexpr bool bIsSoftClass = TALS_IsSoftClassPtr<U>::value;
        constexpr bool bIsSoftObject = TALS_IsSoftObjectPtr<U>::value;
        constexpr bool bIsSharedPtr = TALS_IsSharedPtr<U>::value;
        constexpr bool bIsSharedRef = TALS_IsSharedRef<U>::value;
        constexpr bool bIsUniquePtr = TALS_IsUniquePtr<U>::value;

        if constexpr (bIsEnum)
        {
            if (const UEnum* E = StaticEnum<U>())
            {
                int64 V = static_cast<int64>(InValue);
                return E->GetName() + TEXT("::") + E->GetDisplayNameTextByValue(V).ToString();
            }

            using Underlying = std::underlying_type_t<U>;
            return FString::Printf(TEXT("%d"), static_cast<Underlying>(InValue));
        }
        else if constexpr (bIsEnumAsByte)
        {
            using EnumType = typename TIsUnderlyingEnum<U>::Type;
            EnumType RawEnum = InValue.GetValue();
            uint8 Val = static_cast<uint8>(RawEnum);

            if (const UEnum* E = StaticEnum<EnumType>())
            {
                return E->GetNameStringByValue(Val);
            }

            return FString::Printf(TEXT("%u"), Val);
        }
        else if constexpr (bIsArray || bIsSet || bIsMap)
        {
            FStringBuilderBase Builder;
            if (InValue.Num() == 0) return TEXT("Empty");

            int32 Index = 0;
            for (auto& Elem : InValue)
            {
                if constexpr (bIsMap)
                {
                    Builder.Appendf(TEXT("\n%d -> [K: %s, V: %s]"), Index++, *ConvertToStringCPP(Elem.Key), *ConvertToStringCPP(Elem.Value));
                }
                else
                {
                    Builder.Appendf(TEXT("\n%d -> %s"), Index++, *ConvertToStringCPP(Elem));
                }
            }
            return Builder.ToString();
        }
        else if constexpr (bIsPair)
        {
            return FString::Printf(TEXT("[first: %s, second: %s]"), *ConvertToStringCPP(InValue.first), *ConvertToStringCPP(InValue.second));
        }
        else if constexpr (bIsStdVector)
        {
            FStringBuilderBase Builder;
            if (InValue.empty()) return TEXT("Empty std::vector");

            int32 Index = 0;
            for (auto& Elem : InValue)
            {
                Builder.Appendf(TEXT("\n%d -> %s"), Index++, *ConvertToStringCPP(Elem));
            }
            return Builder.ToString();
        }
        else if constexpr (bIsStruct)
        {
            if constexpr (TModels<CStaticStructProvider, T>::Value)
            {
                if (const UScriptStruct* SS = Cast<UScriptStruct>(T::StaticStruct()))
                {
                    FStringBuilderBase Builder;
                    ConvertToString_Struct(SS, &InValue, Builder);
                    return Builder.ToString();
                }
            }
            return TEXT("Invalid Struct");
        }
        else if constexpr (bIsBool)
        {
            return InValue ? TEXT("True") : TEXT("False");
        }
        else if constexpr (bIsArithmetic)
        {
            if constexpr (std::is_floating_point_v<U>)
                return FString::Printf(TEXT("%f"), static_cast<double>(InValue));
            else
                return FString::Printf(TEXT("%lld"), static_cast<long long>(InValue));
        }
        else if constexpr (bIsString)
        {
            if constexpr (bIsFString)        return InValue;
            else if constexpr (bIsFName)     return InValue.ToString();
            else if constexpr (bIsFText)     return InValue.ToString(); 
            else if constexpr (bIsCString)   return FString(InValue);
            else if constexpr (bIsStdString) return FString(InValue.c_str());
            else return FString(InValue);
        }
        else if constexpr (std::is_same_v<U, FRotator>)
        {
            return FString::Printf(TEXT("P: %f, Y: %f, R: %f"),
                InValue.Pitch, InValue.Yaw, InValue.Roll);
        }
        else if constexpr (std::is_same_v<U, FVector>)
        {
            return FString::Printf(TEXT("X: %f, Y: %f, Z: %f"),
                InValue.X, InValue.Y, InValue.Z);
        }
        else if constexpr (std::is_same_v<U, FVector2D>)
        {
            return FString::Printf(TEXT("X: %f, Y: %f"),
                InValue.X, InValue.Y);
        }
        else if constexpr (std::is_same_v<U, FVector4>)
        {
            return FString::Printf(TEXT("X: %f, Y: %f, Z: %f, W: %f"),
                InValue.X, InValue.Y, InValue.Z, InValue.W);
        }
        else if constexpr (std::is_same_v<U, FTransform>)
        {
            const auto& Tm = InValue;
            auto Loc = Tm.GetLocation();
            auto Rot = Tm.Rotator();
            auto Scale = Tm.GetScale3D();
            return FString::Printf(
                TEXT("[Location] %.3f,%.3f,%.3f -- [Rotation] %.6f,%.6f,%.6f -- [Scale] %.3f,%.3f,%.3f"),
                Loc.X, Loc.Y, Loc.Z,
                Rot.Pitch, Rot.Yaw, Rot.Roll,
                Scale.X, Scale.Y, Scale.Z
            );
        }
        else if constexpr (std::is_same_v<U, FQuat>)
        {
            return FString::Printf(TEXT("X: %f, Y: %f, Z: %f, W: %f"), InValue.X, InValue.Y, InValue.Z, InValue.W);
        }
        else if constexpr (std::is_same_v<U, FMatrix>)
        {
            FString S;
            for (int R = 0; R < 4; ++R)
                S += FString::Printf(TEXT("[ %f, %f, %f, %f ]\n"), InValue.M[R][0], InValue.M[R][1], InValue.M[R][2], InValue.M[R][3]);
            return TEXT("FMatrix:\n") + S;
        }
        else if constexpr (std::is_same_v<U, FPlane>)
        {
            return FString::Printf(TEXT("X: %f, Y: %f, Z: %f, W: %f"),
                InValue.X, InValue.Y, InValue.Z, InValue.W);
        }
        else if constexpr (std::is_same_v<U, FBox>)
        {
            return FString::Printf(TEXT("Min: %s, Max: %s"),
                *ConvertToStringCPP(InValue.Min),
                *ConvertToStringCPP(InValue.Max));
        }
        else if constexpr (std::is_same_v<U, FBox2D>)
        {
            return FString::Printf(TEXT("Min: %s, Max: %s"),
                *ConvertToStringCPP(InValue.Min),
                *ConvertToStringCPP(InValue.Max));
        }
        else if constexpr (std::is_same_v<U, FColor>)
        {
            return FString::Printf(TEXT("R: %d, G: %d, B: %d, A: %d"),
                InValue.R, InValue.G, InValue.B, InValue.A);
        }
        else if constexpr (std::is_same_v<U, FLinearColor>)
        {
            return FString::Printf(TEXT("R: %f, G: %f, B: %f, A: %f"),
                InValue.R, InValue.G, InValue.B, InValue.A);
        }
        else if constexpr (std::is_same_v<U, FIntPoint>)
        {
            return FString::Printf(TEXT("X: %d, Y: %d"),
                InValue.X, InValue.Y);
        }
        else if constexpr (std::is_same_v<U, FIntVector>)
        {
            return FString::Printf(TEXT("X: %d, Y: %d, Z: %d"),
                InValue.X, InValue.Y, InValue.Z);
        }
        else if constexpr (std::is_same_v<U, FIntVector2>)
        {
            return FString::Printf(TEXT("X: %d, Y: %d"), InValue.X, InValue.Y);
        }
        else if constexpr (std::is_same_v<U, FIntVector4>)
        {
            return FString::Printf(TEXT("X: %d, Y: %d, Z: %d, W: %d"),
                                   InValue.X, InValue.Y, InValue.Z, InValue.W);
        }
        else if constexpr (std::is_same_v<U, FUintVector2> || std::is_same_v<U, FUint32Vector2>)
        {
            return FString::Printf(TEXT("X: %u, Y: %u"), InValue.X, InValue.Y);
        }
        else if constexpr (std::is_same_v<U, FUintVector3> || std::is_same_v<U, FUint32Vector3>)
        {
            return FString::Printf(TEXT("X: %u, Y: %u, Z: %u"), InValue.X, InValue.Y, InValue.Z);
        }
        else if constexpr (std::is_same_v<U, FUintVector4> || std::is_same_v<U, FUint32Vector4>)
        {
            return FString::Printf(TEXT("X: %u, Y: %u, Z: %u, W: %u"),
                                   InValue.X, InValue.Y, InValue.Z, InValue.W);
        }
        else if constexpr (std::is_same_v<U, FIntRect>)
        {
            return FString::Printf(TEXT("Min:(%d,%d)  Max:(%d,%d)"),
                                   InValue.Min.X, InValue.Min.Y, InValue.Max.X, InValue.Max.Y);
        }
        else if constexpr (std::is_same_v<U, FUintRect>)
        {
            return FString::Printf(TEXT("Min:(%u,%u)  Max:(%u,%u)"),
                                   InValue.Min.X, InValue.Min.Y, InValue.Max.X, InValue.Max.Y);
        }
        else if constexpr (std::is_same_v<U, FBoxSphereBounds>)
        {
            return FString::Printf(TEXT("Origin:%s  Extent:%s  Radius:%f"),
                   *ConvertToStringCPP(InValue.Origin),
                   *ConvertToStringCPP(InValue.BoxExtent),
                   InValue.SphereRadius);
        }
        else if constexpr (std::is_same_v<U, FCompactBoxSphereBounds>)
        {
            return FString::Printf(TEXT("Origin:%s  Extent:%s  Radius:%f"),
                *ConvertToStringCPP(InValue.Origin),
                *ConvertToStringCPP(InValue.BoxExtent),
                InValue.SphereRadius);
        }
        else if constexpr (std::is_same_v<U, FCollisionProfileName>)
        {
            return InValue.Name.ToString();
        }
        else if constexpr (std::is_same_v<U, FTimerHandle>)
        {
            return InValue.IsValid() ? FString::Printf(TEXT("TimerHandle(%llu)"), InValue.GetHandle()) : TEXT("Invalid TimerHandle");
        }
        else if constexpr (std::is_same_v<U, FSoftObjectPath>)
        {
            return InValue.IsNull() ? TEXT("Null SoftObjectPath") : InValue.ToString();
        }
        else if constexpr (std::is_same_v<U, FSoftClassPath>)
        {
            return InValue.IsNull() ? TEXT("Null SoftClassPath")  : InValue.ToString();
        }
        else if constexpr (std::is_same_v<U, FPrimaryAssetId>)
        {
            return InValue.IsValid() ? InValue.ToString() : TEXT("Invalid PrimaryAssetId");
        }
        else if constexpr (std::is_same_v<U, FPrimaryAssetType>)
        {
            return InValue.ToString();
        }
        else if constexpr (std::is_same_v<U, FGameplayTag>)
        {
            return InValue.IsValid() ? InValue.ToString() : TEXT("Invalid GameplayTag");
        }
        else if constexpr (std::is_same_v<U, FGameplayTagContainer>)
        {
            FString Out;
            InValue.ToStringSimple(true);
            return Out.IsEmpty() ? TEXT("Empty TagContainer") : Out;
        }
        else if constexpr (std::is_same_v<U, FFloatRange>)
        {
            return FString::Printf(TEXT("[%.3f – %.3f]"),
                   InValue.GetLowerBoundValue(), InValue.GetUpperBoundValue());
        }
        else if constexpr (std::is_same_v<U, FInt32Range>)
        {
            return FString::Printf(TEXT("[%d – %d]"),
                   InValue.GetLowerBoundValue(), InValue.GetUpperBoundValue());
        }
        else if constexpr (TALS_IsOptional<U>::value)
        {
            return InValue.IsSet() ? ConvertToStringCPP(*InValue) : TEXT("Unset Optional");
        }
        else if constexpr (std::is_same_v<U, FRandomStream>)
        {
            return FString::Printf(TEXT("%d"), InValue.GetInitialSeed());
        }
        else if constexpr (std::is_same_v<U, FGuid>)
        {
            return InValue.ToString();
        }
        else if constexpr (std::is_same_v<U, FDateTime>)
        {
            FString Base = InValue.ToString(TEXT("%Y-%m-%d %H:%M:%S"));
            int32 MS = InValue.GetMillisecond();
            return FString::Printf(TEXT("%s.%03d"), *Base, MS);
        }
        else if constexpr (std::is_same_v<U, FTimespan>)
        {
            int32 D = InValue.GetDays();
            int32 H = InValue.GetHours();
            int32 M = InValue.GetMinutes();
            int32 S = InValue.GetSeconds();
            int32 MS = static_cast<int32>(InValue.GetTotalMilliseconds()) % 1000;
            return FString::Printf(TEXT("%dD %02d:%02d:%02d.%03d"), D, H, M, S, MS);
        }
        else if constexpr (std::is_pointer_v<U> && std::is_base_of_v<UObject, std::remove_pointer_t<U>>)
        {
			return GetDisplayNameSafe(InValue);
        }
        else if constexpr (std::is_base_of_v<UObject, std::remove_reference_t<U>>)
        {
            return GetDisplayNameSafe(&InValue);
        }
        else if constexpr (std::is_pointer_v<U> && std::is_arithmetic_v<std::remove_pointer_t<U>>)
        {
            return InValue ? ConvertToStringCPP(*InValue) : TEXT("Null Pointer");
        }
        else if constexpr (std::is_pointer_v<U>)
        {
            return InValue ? ConvertToStringCPP(*InValue) : TEXT("Null Pointer");
        }
        else if constexpr (bIsWeakPtr || bIsSubclass || bIsSharedPtr || bIsSharedRef || bIsUniquePtr)
        {
            return GetDisplayNameSafe(InValue.Get());
        }
        else if constexpr (bIsSoftClass)
        {
            return InValue.IsValid() ? *InValue.GetAssetName() : TEXT("Null TSoftClassPtr");
        }
        else if constexpr (bIsSoftObject)
        {
            return InValue.IsValid() ? *InValue.GetAssetName() : TEXT("Null TSoftObjectPtr");
        }
        else
        {
            static_assert(!std::is_same_v<T, T>, "Converting this type to string failed. Add conversion for this type in ALS_Globals.h");
        }
    }

    template <typename... Args>
    static inline void PrintALSCPP(const FPrintConfig& PrintConfig, const UObject* Context, const FString& SourceID, Args&&... Arguments)
    {
        TArray<FString> Messages;
        Messages.Reserve(sizeof...(Arguments)); 
        (Messages.Add(ConvertToStringCPP(std::forward<Args>(Arguments))), ...);
        FString FormattedString = FString::Join(Messages, TEXT(""));
        PrintALS(FormattedString, PrintConfig, Context, SourceID);
    }

    template <typename T, typename... Args>
    static inline void DrawALSCPP(const FPrintConfig& PrintConfig, const UObject* Context, const FString& SourceID, T&& LocationArg, Args&&... Arguments)
    {
        UObject* TextObject = nullptr;
        FVector TextLocation = FVector::ZeroVector;

        if constexpr (std::is_same_v<std::decay_t<T>, FVector>)
        {
            TextLocation = LocationArg;
        }
        else if constexpr (std::is_convertible_v<std::decay_t<T>, AActor*> || std::is_convertible_v<std::decay_t<T>, USceneComponent*>)
        {
            TextObject = LocationArg;
        }
        else
        {
            static_assert([] { return false; }(),"Print3D: First argument must be an Vector, Actor or a SceneComponent");
        }

        TArray<FString> Messages;
        Messages.Reserve(sizeof...(Arguments)); 
        (Messages.Add(ConvertToStringCPP(std::forward<Args>(Arguments))), ...);
        FString FormattedString = FString::Join(Messages, TEXT(""));
        DrawALS(FormattedString, TextObject, TextLocation, PrintConfig, Context, SourceID);
    }
};
