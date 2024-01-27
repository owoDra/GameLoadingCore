// Copyright (C) 2024 owoDra

#include "LoadingObserver_MapLoad.h"

#include "GameplayTag/GCLoadingTags_LoadingType.h"
#include "LoadingScreenSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LoadingObserver_MapLoad)


#define LOCTEXT_NAMESPACE "LoadingScreen"

const FName ULoadingObserver_MapLoad::NAME_MapLoadingProcess("MapLoadingProcess");

ULoadingObserver_MapLoad::ULoadingObserver_MapLoad(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	LoadingMapReason = FText(LOCTEXT("MapLoadReason", "Loading World"));
}


void ULoadingObserver_MapLoad::OnInitialized()
{
	FCoreUObjectDelegates::PreLoadMapWithContext.AddUObject(this, &ThisClass::HandlePreLoadMap);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ThisClass::HandlePostLoadMap);
}

void ULoadingObserver_MapLoad::OnDeinitialize()
{
	FCoreUObjectDelegates::PreLoadMap.RemoveAll(this);
	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);

}

void ULoadingObserver_MapLoad::Tick(float DeltaTime)
{
	if (OwnerSubsystem.IsValid() && OwnerGameInstance.IsValid())
	{
		// Display load screen while WorldContext is disabled

		const auto* Context{ OwnerGameInstance->GetWorldContext() };
		if (!Context)
		{
			SetbShouldShowLoadingScreen(true);
			return;
		}

		// Show load screen while World is disabled

		auto* World{ Context->World() };
		if (!World)
		{
			SetbShouldShowLoadingScreen(true);
			return;
		}

		// Show loading screen during map loading.

		if (bIsCurrentlyLoadingMap)
		{
			SetbShouldShowLoadingScreen(true);
			return;
		}

		// Show loading screen during map movement.

		if (!Context->TravelURL.IsEmpty())
		{
			SetbShouldShowLoadingScreen(true);
			return;
		}

		// The loading screen is displayed before the game starts.

		if (!World->HasBegunPlay())
		{
			SetbShouldShowLoadingScreen(true);
			return;
		}

		// Show loading screen during map seamless travel.

		if (World->IsInSeamlessTravel())
		{
			SetbShouldShowLoadingScreen(true);
			return;
		}

		SetbShouldShowLoadingScreen(false);
	}
}


void ULoadingObserver_MapLoad::HandlePreLoadMap(const FWorldContext& WorldContext, const FString& MapName)
{
	if (WorldContext.OwningGameInstance == OwnerGameInstance)
	{
		bIsCurrentlyLoadingMap = true;
	}
}

void ULoadingObserver_MapLoad::HandlePostLoadMap(UWorld* World)
{
	if (World && (World->GetGameInstance() == OwnerGameInstance))
	{
		bIsCurrentlyLoadingMap = false;
	}
}

void ULoadingObserver_MapLoad::SetbShouldShowLoadingScreen(bool bNewValue)
{
	if (bShouldShowLoadingScreen != bNewValue)
	{
		bShouldShowLoadingScreen = bNewValue;

		if (bShouldShowLoadingScreen)
		{
			OwnerSubsystem->AddLoadingProcess(ULoadingObserver_MapLoad::NAME_MapLoadingProcess, TAG_LoadingType_Fullscreen, LoadingMapReason);
		}
		else
		{
			OwnerSubsystem->RemoveLoadingProcess(ULoadingObserver_MapLoad::NAME_MapLoadingProcess);
		}
	}
}

#undef LOCTEXT_NAMESPACE
