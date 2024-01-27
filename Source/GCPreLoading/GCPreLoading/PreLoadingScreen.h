// Copyright (C) 2024 owoDra

#pragma once

#include "PreLoadScreenBase.h"

class SWidget;


/**
 * Class that manages the load screen
 */
class FPreLoadingScreen : public FPreLoadScreenBase
{
private:
    TSharedPtr<SWidget> EngineLoadingWidget;

public:
	virtual void Init() override;
    virtual EPreLoadScreenTypes GetPreLoadScreenType() const override { return EPreLoadScreenTypes::EngineLoadingScreen; }
    virtual TSharedPtr<SWidget> GetWidget() override { return EngineLoadingWidget; }

};
