//Copyright © 2025 RTerofer. All Rights Reserved.

#include "ALSE_Style.h"
#include "ALSE.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyleRegistry.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FALSStyle::StyleInstance = nullptr;


void FALSStyle::Initialize()
{
    if (!StyleInstance.IsValid())
    {
		StyleInstance = MakeShareable(new FSlateStyleSet("ALSStyle"));
        FString IconPath = IPluginManager::Get().FindPlugin("AdvancedLoggingSystem")->GetBaseDir() / "Resources";

        // PROPER 24x24 TOOLBAR ICON
		StyleInstance->Set("ALS.ToolBar", new FSlateImageBrush(IconPath / "Icon48.png", FVector2D(25, 25)));
		StyleInstance->Set("ALS.Undo", new FSlateImageBrush(IconPath / "Undo50.png", FVector2D(14, 14)));

        FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance.Get());
    }
}

void FALSStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FALSStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("ALSStyle"));
	return StyleSetName;
}

void FALSStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FALSStyle::Get()
{
	return *StyleInstance;
}
