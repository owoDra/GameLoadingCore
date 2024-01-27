// Copyright (C) 2024 owoDra

#pragma once

#include "Modules/ModuleManager.h"

class FPreLoadingScreen;

/**
 * Module for load screen functionality
 */
class FGCPreLoadingModule : public IModuleInterface
{
protected:
	typedef FGCPreLoadingModule ThisClass;

public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	bool IsGameModule() const override;

private:
	/**
	 *  Run when PreLoadingScreenManager is destroyed
	 */
	void OnPreLoadingScreenManagerCleanUp();

private:
	//
	// Reference to the class that manages the load screen
	//
	TSharedPtr<FPreLoadingScreen> PreLoadingScreen;

};
