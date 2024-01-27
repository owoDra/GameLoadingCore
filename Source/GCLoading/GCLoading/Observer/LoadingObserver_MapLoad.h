// Copyright (C) 2024 owoDra

#pragma once

#include "Observer/LoadingObserver.h"

#include "LoadingObserver_MapLoad.generated.h"


/**
 * Loading observer class to monitor map loads, transitions, and starts
 */
UCLASS(meta = (DisplayName = "Map Load Observer"))
class GCLOADING_API ULoadingObserver_MapLoad : public ULoadingObserver
{
	GENERATED_BODY()
public:
	ULoadingObserver_MapLoad(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	static const FName NAME_MapLoadingProcess;

protected:
	virtual void OnInitialized() override;
	virtual void OnDeinitialize() override;

public:
	virtual void Tick(float DeltaTime) override;


protected:
	//
	// Whether the map is currently loading or not
	//
	UPROPERTY(Transient)
	bool bIsCurrentlyLoadingMap{ false };

	//
	// Whether the map is currently loading or not
	//
	UPROPERTY(Transient)
	bool bShouldShowLoadingScreen{ false };

	//
	// Whether the map is currently loading or not
	//
	UPROPERTY()
	FText LoadingMapReason;

protected:
	void HandlePreLoadMap(const FWorldContext& WorldContext, const FString& MapName);
	void HandlePostLoadMap(UWorld* World);

	void SetbShouldShowLoadingScreen(bool bNewValue);

};
