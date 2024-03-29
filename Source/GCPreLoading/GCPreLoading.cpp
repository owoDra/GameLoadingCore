﻿// Copyright (C) 2024 owoDra

#include "GCPreLoading.h"

#include "PreLoadingScreen.h"

#include "Misc/App.h"
#include "PreLoadScreenManager.h"

IMPLEMENT_MODULE(FGCPreLoadingModule, GCPreLoading)


void FGCPreLoadingModule::StartupModule()
{
	if (!GIsEditor && FApp::CanEverRender() && FPreLoadScreenManager::Get())
	{
		PreLoadingScreen = MakeShared<FPreLoadingScreen>();
		PreLoadingScreen->Init();

		FPreLoadScreenManager::Get()->RegisterPreLoadScreen(PreLoadingScreen);
		FPreLoadScreenManager::Get()->OnPreLoadScreenManagerCleanUp.AddRaw(this, &ThisClass::OnPreLoadingScreenManagerCleanUp);
	}
}

void FGCPreLoadingModule::ShutdownModule()
{
}

bool FGCPreLoadingModule::IsGameModule() const
{
	return true;
}


void FGCPreLoadingModule::OnPreLoadingScreenManagerCleanUp()
{
	PreLoadingScreen.Reset();
	ShutdownModule();
}
