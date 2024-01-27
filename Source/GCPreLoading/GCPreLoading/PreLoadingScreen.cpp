// Copyright (C) 2024 owoDra

#include "PreLoadingScreen.h"

#include "PreLoadingScreenSlateWidget.h"

#include "Misc/App.h"


void FPreLoadingScreen::Init()
{
	if (!GIsEditor && FApp::CanEverRender())
	{
		EngineLoadingWidget = SNew(SPreLoadingScreenWidget);
	}
}
