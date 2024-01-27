// Copyright (C) 2024 owoDra

#include "PreLoadingScreenSlateWidget.h"

#include "Widgets/Layout/SBorder.h"


void SPreLoadingScreenWidget::AddReferencedObjects(FReferenceCollector& Collector)
{
}

FString SPreLoadingScreenWidget::GetReferencerName() const
{
	return TEXT("SPreLoadingScreenWidget");
}

void SPreLoadingScreenWidget::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor::Black)
		.Padding(0)
	];
}
