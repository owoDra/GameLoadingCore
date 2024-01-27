// Copyright (C) 2024 owoDra

#include "LoadingObserver.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LoadingObserver)


ULoadingObserver::ULoadingObserver(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


void ULoadingObserver::InitializeObserver(UGameInstance* GameInstance, ULoadingScreenSubsystem* Subsystem)
{
	check(GameInstance);
	check(Subsystem);

	OwnerGameInstance = GameInstance;
	OwnerSubsystem = Subsystem;

	OnInitialized();
}

void ULoadingObserver::DeinitializeObserver()
{
	OnDeinitialize();
}
