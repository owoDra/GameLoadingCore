// Copyright (C) 2024 owoDra

#pragma once

#include "Engine/DeveloperSettings.h"

#include "GameplayTagContainer.h"

#include "LoadingDeveloperSettings.generated.h"


/**
 * Definition data of widgets to be displayed for loading type
 */
USTRUCT(BlueprintType)
struct FLoadingScreenDefinition
{
	GENERATED_BODY()
public:
	FLoadingScreenDefinition() {}

public:
	//
	// Widgets to display
	//
	UPROPERTY(EditAnywhere, meta = (MetaClass = "/Script/UMG.UserWidget"))
	FSoftClassPath WidgetClass;

	//
	// Priority of widgets to display
	//
	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.00))
	int32 ZOrder{ 100 };

	//
	// Number of seconds to continue displaying after loading ends
	//
	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.00))
	float AdditionalSecs{ 2.0f };

	//
	// Whether input should be blocked during loading?
	//
	UPROPERTY(EditAnywhere)
	bool bBlockInputs{ true };

	//
	// Whether the game saves performance during loading
	//
	UPROPERTY(EditAnywhere)
	bool bSavingPerfomance{ true };

};


/**
 * Settings for a loading screen system.
 */
UCLASS(Config = "Game", Defaultconfig, meta = (DisplayName = "Game Loading Core"))
class ULoadingDeveloperSettings : public UDeveloperSettings
{
public:
	GENERATED_BODY()
public:
	ULoadingDeveloperSettings();


	///////////////////////////////////////////////
	// LoadingScreen
public:
	//
	// Mapping list of widgets to display against loading type
	//
	UPROPERTY(Config, EditAnywhere, Category = "LoadingScreen", meta = (ForceInlineRow, ShowOnlyInnerProperties, TitleProperty = "WidgetClass", Categories = "LoadingType"))
	TMap<FGameplayTag, FLoadingScreenDefinition> LoadingScreenDefinitions;
	
	//
	// Class list of loading observers to be used
	//
	UPROPERTY(Config, EditAnywhere, Category = "LoadingScreen", meta = (MetaClass = "/Script/GCLoading.LoadingObserver"))
	TArray<FSoftClassPath> ObserverClassesToEnable;

public:
	//
	// After the actual loading is completed in the test play in the editor, do you want to show an additional loading screen?
	//
	UPROPERTY(Config, EditAnywhere, Category = "LoadingScreen|Editor")
	bool bShouldHoldLoadingScreenAdditionalSecsInEditor{ false };

	//
	// Forcing a Tick when the load screen is displayed during test play in the editor?
	//
	UPROPERTY(Config, EditAnywhere, Category= "LoadingScreen|Editor")
	bool bForceTickLoadingScreenInEditor{ true };

};

