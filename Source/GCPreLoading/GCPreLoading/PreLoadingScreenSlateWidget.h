// Copyright (C) 2024 owoDra

#pragma once

#include "Widgets/SCompoundWidget.h"
#include "UObject/GCObject.h"


class SPreLoadingScreenWidget : public SCompoundWidget, public FGCObject
{
public:
	SLATE_BEGIN_ARGS(SPreLoadingScreenWidget) {}
    SLATE_END_ARGS()

public:
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override;

    void Construct(const FArguments& InArgs);

};
