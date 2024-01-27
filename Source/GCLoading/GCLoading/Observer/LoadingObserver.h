// Copyright (C) 2024 owoDra

#pragma once

#include "UObject/Object.h"

#include "LoadingObserver.generated.h"

class UGameInstance;
class ULoadingScreenSubsystem;


/**
 * Base class for monitoring the processing of the possibility that the loading screen needs to be displayed 
 * and automatically showing and hiding the loading screen
 */
UCLASS(Abstract)
class GCLOADING_API ULoadingObserver : public UObject
{
	GENERATED_BODY()
public:
	ULoadingObserver(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	UPROPERTY(Transient)
	TWeakObjectPtr<UGameInstance> OwnerGameInstance{ nullptr };

	UPROPERTY(Transient)
	TWeakObjectPtr<ULoadingScreenSubsystem> OwnerSubsystem{ nullptr };

public:
	void InitializeObserver(UGameInstance* GameInstance, ULoadingScreenSubsystem* Subsystem);
	void DeinitializeObserver();

protected:
	virtual void OnInitialized() {}
	virtual void OnDeinitialize() {}

public:
	virtual void Tick(float DeltaTime) {}

};
