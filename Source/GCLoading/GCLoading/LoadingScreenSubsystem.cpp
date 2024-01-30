// Copyright (C) 2024 owoDra

#include "LoadingScreenSubsystem.h"

#include "LoadingDeveloperSettings.h"
#include "Observer/LoadingObserver.h"
#include "GCLoadingLogs.h"

#include "Blueprint/UserWidget.h"
#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "ShaderPipelineCache.h"
#include "HAL/ThreadHeartBeat.h"
#include "PreLoadScreen.h"
#include "PreLoadScreenManager.h"
#include "Framework/Application/IInputProcessor.h"
#include "Framework/Application/SlateApplication.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LoadingScreenSubsystem)


// Initialization

void ULoadingScreenSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	InitializeObservers();
}

void ULoadingScreenSubsystem::Deinitialize()
{
	DeinitializeObservers();

	LoadingWidgetOverrides.Empty();
	LoadingScreenInfos.Empty();
	PendingAddLoadingTags.Empty();
	PendingRemoveLoadingTags.Empty();

	bLoadingWidgetDisplayed = false;

	ClearInputBlockCount();
	ClearSavingPerformanceCount();
	RemoveAllWidgets();
}

bool ULoadingScreenSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	// Only clients have loading screens

	const auto* GameInstance{ CastChecked<UGameInstance>(Outer) };
	const auto bIsServerWorld{ GameInstance->IsDedicatedServerInstance() };
	
	return !bIsServerWorld;
}


// Tick

void ULoadingScreenSubsystem::Tick(float DeltaTime)
{
	TickObservers(DeltaTime);

	if (!IsShowingInitialLoadingScreen())
	{
		UpdateLoadingWidgets();
	}
}

ETickableTickType ULoadingScreenSubsystem::GetTickableTickType() const
{
	return ETickableTickType::Conditional;
}

bool ULoadingScreenSubsystem::IsTickable() const
{
	return !HasAnyFlags(RF_ClassDefaultObject);
}

TStatId ULoadingScreenSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(ULoadingScreenSubsystem, STATGROUP_Tickables);
}

UWorld* ULoadingScreenSubsystem::GetTickableGameObjectWorld() const
{
	return GetGameInstance()->GetWorld();
}


// Loading Observer

void ULoadingScreenSubsystem::InitializeObservers()
{
	auto* LocalGameInstance{ GetGameInstance() };
	const auto* DevSettings{ GetDefault<ULoadingDeveloperSettings>() };
	const auto ObserverClassPaths{ DevSettings->ObserverClassesToEnable };

	for (const auto& ClassPath : ObserverClassPaths)
	{
		auto* OverserClass{ ClassPath.IsValid() ? ClassPath.TryLoadClass<ULoadingObserver>() : nullptr};
		auto* NewObserver{ OverserClass ? NewObject<ULoadingObserver>(LocalGameInstance, OverserClass) : nullptr };

		if (NewObserver)
		{
			NewObserver->InitializeObserver(LocalGameInstance, this);

			ActiveObservers.Emplace(NewObserver);
		}
	}
}

void ULoadingScreenSubsystem::DeinitializeObservers()
{
	for (const auto& Observer : ActiveObservers)
	{
		if (Observer)
		{
			Observer->DeinitializeObserver();
		}
	}
}

void ULoadingScreenSubsystem::TickObservers(float DeltaTime)
{
	for (const auto& Observer : ActiveObservers)
	{
		if (Observer)
		{
			Observer->Tick(DeltaTime);
		}
	}
}


// Loading Widget Override

void ULoadingScreenSubsystem::AddLoadingWidgetOverride(FGameplayTag LoadingTypeTag, TSubclassOf<UUserWidget> WidgetClass)
{
	if (!LoadingTypeTag.IsValid())
	{
		UE_LOG(LogGameCore_LoadingScreen, Error, TEXT("Tried to add LoadingWidgetOverride with invalid tag"));
		return;
	}

	if (!WidgetClass)
	{
		UE_LOG(LogGameCore_LoadingScreen, Error, TEXT("Tried to add LoadingWidgetOverride with invalid class"));
		return;
	}

	LoadingWidgetOverrides.Emplace(LoadingTypeTag, WidgetClass);
}


// Loading Processes Infos

bool ULoadingScreenSubsystem::AddLoadingProcess(FName ProcessName, FGameplayTag LoadingTypeTag, FText Reason)
{
	if (!LoadingTypeTag.IsValid())
	{
		UE_LOG(LogGameCore_LoadingScreen, Error, TEXT("An invalid loading type tag attempted to add a loading process."));
		return false;
	}

	if (Reason.IsEmpty())
	{
		UE_LOG(LogGameCore_LoadingScreen, Error, TEXT("Loading reason not set."));
		return false;
	}

	if (!ProcessName.IsValid() || ProcessName.IsNone())
	{
		UE_LOG(LogGameCore_LoadingScreen, Error, TEXT("Process Name not set."));
		return false;
	}

	// If there is already information for the same loading type, add it there

	if (auto* FoundInfo{ LoadingScreenInfos.Find(LoadingTypeTag) })
	{
		return AddLoadingProcessExistType(*FoundInfo, ProcessName, LoadingTypeTag, Reason);
	}

	// If the same loading type information is not already available, create a new one based on DevSettings

	const auto* DevSettings{ GetDefault<ULoadingDeveloperSettings>() };

	if (!DevSettings)
	{
		UE_LOG(LogGameCore_LoadingScreen, Fatal, TEXT("DevSettings is invalid"));
		return false;
	}

	const auto* Definition{ DevSettings->LoadingScreenDefinitions.Find(LoadingTypeTag) };

	if (!Definition)
	{
		UE_LOG(LogGameCore_LoadingScreen, Error, TEXT("Undefined LoadingTypeTag(%s), set from DeveloperSettings."), *LoadingTypeTag.GetTagName().ToString());
		return false;
	}

	return AddLoadingProcessNew(ProcessName, LoadingTypeTag, Reason, *Definition);
}

bool ULoadingScreenSubsystem::RemoveLoadingProcess(FName ProcessName)
{
	// Iterate LoadingScreenInfos

	for (auto& KVP : LoadingScreenInfos)
	{
		// Search for Info containing handles

		auto& Tag{ KVP.Key };
		auto& Info{ KVP.Value };
		auto& ProcessNameReasonMap{ Info.Processes };

		if (ProcessNameReasonMap.Contains(ProcessName))
		{
			UE_LOG(LogGameCore_LoadingScreen, Log, TEXT("Remove Loading process (ProcessName: %s)"), *ProcessName.ToString());

			// Remove handle items from HandleReasonMap

			ProcessNameReasonMap.Remove(ProcessName);

			// If a valid handle no longer exists at this point, it is added to PendingRemove

			if (ProcessNameReasonMap.IsEmpty())
			{
				AddTagToPendingRemoveList(Tag);
			}

			return true;
		}
	}

	return false;
}

bool ULoadingScreenSubsystem::RemoveLoadingProcessByTag(FGameplayTag LoadingTypeTag)
{
	if (LoadingScreenInfos.Contains(LoadingTypeTag))
	{
		AddTagToPendingRemoveList(LoadingTypeTag);
		return true;
	}

	return false;
}


bool ULoadingScreenSubsystem::AddLoadingProcessExistType(FLoadingScreenInfo& Info, FName ProcessName, const FGameplayTag& LoadingTypeTag, const FText& Reason)
{
	// Early out if Process Name already exist

	if (Info.Processes.Contains(ProcessName))
	{
		return false;
	}

	Info.Processes.Add(ProcessName, Reason);

	CancelPendingRemove(LoadingTypeTag);

	UE_LOG(LogGameCore_LoadingScreen, Log, TEXT("Add Loading process Exist type (Reason: %s, Handle: %s)"), *Reason.ToString(), *ProcessName.ToString());

	return true;
}

bool ULoadingScreenSubsystem::AddLoadingProcessNew(FName ProcessName, const FGameplayTag& LoadingTypeTag, const FText& Reason, const FLoadingScreenDefinition& Def)
{
	// Select Widget class

	auto WidgetClass{ LoadingWidgetOverrides.FindRef(LoadingTypeTag) };
	if (!WidgetClass)
	{
		const auto ClassPath{ Def.WidgetClass };

		WidgetClass = ClassPath.IsValid() ? ClassPath.TryLoadClass<UUserWidget>() : nullptr;
	}
	check(WidgetClass);

	// Build Loading process info

	auto NewInfo{ FLoadingScreenInfo()};
	NewInfo.Processes.Add(ProcessName, Reason);
	NewInfo.WidgetClass = WidgetClass;
	NewInfo.ZOrder = Def.ZOrder;
	NewInfo.AdditionalSec = Def.AdditionalSecs;
	NewInfo.bBlockInputs = Def.bBlockInputs;
	NewInfo.bSavingPerfomance = Def.bSavingPerfomance;

	// Add to list

	LoadingScreenInfos.Add(LoadingTypeTag, NewInfo);

	AddTagToPendingAddList(LoadingTypeTag);

	UE_LOG(LogGameCore_LoadingScreen, Log, TEXT("Add Loading process new (Reason: %s, Handle: %s)"), *Reason.ToString(), *ProcessName.ToString());

	return true;
}


const FText& ULoadingScreenSubsystem::GetLoadingReasonFromName(FName ProcessName) const
{
	// Iterate LoadingScreenInfos

	for (auto& KVP : LoadingScreenInfos)
	{
		// Search for Info containing handles

		auto& Info{ KVP.Value };
		auto& ProcessNameReasonMap{ Info.Processes };

		if (auto* Reason{ ProcessNameReasonMap.Find(ProcessName) })
		{
			return *Reason;
		}
	}

	return FText::GetEmpty();
}

TArray<FText> ULoadingScreenSubsystem::GetLoadingReasonsFromTag(FGameplayTag LoadingTypeTag) const
{
	TArray<FText> Reasons;

	if (auto* Info{ LoadingScreenInfos.Find(LoadingTypeTag) })
	{
		Info->Processes.GenerateValueArray(Reasons);
	}

	return Reasons;
}


// Loading Widget

void ULoadingScreenSubsystem::AddTagToPendingAddList(const FGameplayTag& Tag)
{
	PendingAddLoadingTags.Emplace(Tag);

	CancelPendingRemove(Tag);
}

void ULoadingScreenSubsystem::AddTagToPendingRemoveList(const FGameplayTag& Tag)
{
	const auto& CurrentTime{ FPlatformTime::Seconds() };

	PendingRemoveLoadingTags.Emplace(Tag, CurrentTime);
}

void ULoadingScreenSubsystem::CancelPendingRemove(const FGameplayTag& Tag)
{
	PendingRemoveLoadingTags.Remove(Tag);
}


void ULoadingScreenSubsystem::UpdateLoadingWidgets()
{
	// Process Pending Add

	if (!PendingAddLoadingTags.IsEmpty())
	{
		const auto bForceTick{ !GIsEditor || GetDefault<ULoadingDeveloperSettings>()->bForceTickLoadingScreenInEditor };

		for (auto It{ PendingAddLoadingTags.CreateIterator() }; It; ++It)
		{
			const auto& Tag{ *It };

			if (ProcessPendingAddTag(Tag, bForceTick))
			{
				It.RemoveCurrent();
			}
		}
	}

	// Process Pending Remove

	if (!PendingRemoveLoadingTags.IsEmpty())
	{
		const auto bCanHoldLoadingScreen{ !GIsEditor || GetDefault<ULoadingDeveloperSettings>()->bShouldHoldLoadingScreenAdditionalSecsInEditor };
		const auto CurrentTime{ FPlatformTime::Seconds() };

		for (auto It{ PendingRemoveLoadingTags.CreateIterator() }; It; ++It)
		{
			const auto& Tag{ It->Key };
			const auto& StartTime{ It->Value };

			if (ProcessPendingRemoveTag(Tag, StartTime, CurrentTime, bCanHoldLoadingScreen))
			{
				// Delete Info at this time as well.

				LoadingScreenInfos.Remove(Tag);

				It.RemoveCurrent();
			}
		}
	}

	// Update and broadcast condition

	const auto bNewLoadingScreenDisplayed{ !ShowingWidgets.IsEmpty() };

	if (bLoadingWidgetDisplayed != bNewLoadingScreenDisplayed)
	{
		bLoadingWidgetDisplayed = bNewLoadingScreenDisplayed;

		OnLoadingScreenVisibilityChanged.Broadcast(bLoadingWidgetDisplayed);
	}
}


bool ULoadingScreenSubsystem::ProcessPendingAddTag(const FGameplayTag& Tag, bool bForceTick)
{
	auto& Info{ LoadingScreenInfos[Tag] };

	UE_LOG(LogGameCore_LoadingScreen, Log, TEXT("Load screen displayed (Tag: %s)"), *Tag.GetTagName().ToString());

	// Create Widget, if has not created
	
	TryCreateLoadingWidget(Tag, Info.WidgetClass, Info.ZOrder);

	// Update Input Block

	if (Info.bBlockInputs)
	{
		IncrementInputBlockCount();
	}

	// Update Performance Saving

	if (Info.bSavingPerfomance)
	{
		IncrementSavingPerformanceCount();
	}

	// Perform Tick processing of slate

	if (bForceTick)
	{
		FSlateApplication::Get().Tick();
	}

	return true;
}

bool ULoadingScreenSubsystem::ProcessPendingRemoveTag(const FGameplayTag& Tag, double StatTime, double CurrentTime, bool bShouldHold)
{
	auto& Info{ LoadingScreenInfos[Tag] };
	auto* LocalGameInstance{ GetGameInstance() };

	const auto HoldLoadingScreenAdditionalSecs{ bShouldHold ? Info.AdditionalSec : 0.0 };
	
	if (HoldLoadingScreenAdditionalSecs <= (CurrentTime - StatTime))
	{
		UE_LOG(LogGameCore_LoadingScreen, Log, TEXT("Load screen hidden (Tag: %s)"), *Tag.GetTagName().ToString());

		// Update Input Block

		if (Info.bBlockInputs)
		{
			DecrementInputBlockCount();
		}

		// Update Performance Saving

		if (Info.bSavingPerfomance)
		{
			DecrementSavingPerformanceCount();
		}

		// Remove from viewport

		TryRemoveLoadingWidget(Tag);

		return true;
	}

	return false;
}


void ULoadingScreenSubsystem::TryCreateLoadingWidget(const FGameplayTag& Tag, const TSubclassOf<UUserWidget>& Class, const int32& ZOrder)
{
	if (!ShowingWidgets.Contains(Tag))
	{
		auto* LocalGameInstance{ GetGameInstance() };

		if (auto* Widget{ UUserWidget::CreateWidgetInstance(*LocalGameInstance, Class, NAME_None) })
		{
			// Add to viewport

			auto SlateWidget{ Widget->TakeWidget() };

			if (auto* GameViewportClient{ LocalGameInstance->GetGameViewportClient() })
			{
				GameViewportClient->AddViewportWidgetContent(SlateWidget, ZOrder);
			}

			// Add to list

			ShowingWidgets.Add(Tag, SlateWidget.ToSharedPtr());
		}
		else
		{
			UE_LOG(LogGameCore_LoadingScreen, Error, TEXT("Failed to create widget by class(%s)"), *GetNameSafe(Class));
		}
	}
}

void ULoadingScreenSubsystem::TryRemoveLoadingWidget(const FGameplayTag& Tag)
{
	auto SlateWidget{ ShowingWidgets.FindRef(Tag) };

	if (SlateWidget.IsValid())
	{
		GEngine->ForceGarbageCollection(true);

		if (auto* GameViewportClient{ GetGameInstance()->GetGameViewportClient() })
		{
			GameViewportClient->RemoveViewportWidgetContent(SlateWidget.ToSharedRef());
		}

		SlateWidget.Reset();

		ShowingWidgets.Remove(Tag);
	}
}

void ULoadingScreenSubsystem::RemoveAllWidgets()
{
	auto* GameViewportClient{ GetGameInstance()->GetGameViewportClient() };

	GEngine->ForceGarbageCollection(true);

	for (auto It{ ShowingWidgets.CreateIterator() }; It; ++It)
	{
		auto& SlateWidget{ It->Value };

		if (SlateWidget.IsValid())
		{
			if (GameViewportClient)
			{
				GameViewportClient->RemoveViewportWidgetContent(SlateWidget.ToSharedRef());
			}

			SlateWidget.Reset();
		}

		It.RemoveCurrent();
	}
}


bool ULoadingScreenSubsystem::IsShowingInitialLoadingScreen() const
{
	auto* PreLoadScreenManager{ FPreLoadScreenManager::Get() };
	return PreLoadScreenManager && PreLoadScreenManager->HasValidActivePreLoadScreen();
}


// Input

void ULoadingScreenSubsystem::UpdateInputBlock()
{
	const auto bNewInputBlocked{ InputBlockCount > 0 };

	if (bInputBlocked != bNewInputBlocked)
	{
		bInputBlocked = bNewInputBlocked;

		UE_LOG(LogGameCore_LoadingScreen, Log, TEXT("Input Block: %s"), bInputBlocked ? TEXT("ENABLED") : TEXT("DISABLED"));

		if (bInputBlocked)
		{
			if (!InputPreProcessor.IsValid())
			{
				InputPreProcessor = MakeShareable<FLoadingScreenInputPreProcessor>(new FLoadingScreenInputPreProcessor());
				FSlateApplication::Get().RegisterInputPreProcessor(InputPreProcessor, 0);
			}
		}
		else
		{
			if (InputPreProcessor.IsValid())
			{
				FSlateApplication::Get().UnregisterInputPreProcessor(InputPreProcessor);
				InputPreProcessor.Reset();
			}
		}
	}
}


// Performance

void ULoadingScreenSubsystem::UpdatePerformance()
{
	const auto bNewSavingPerformance{ SavingPerformanceCount > 0 };

	if (bSavingPerformance != bNewSavingPerformance)
	{
		bSavingPerformance = bNewSavingPerformance;

		UE_LOG(LogGameCore_LoadingScreen, Log, TEXT("Saving Performance: %s"), bSavingPerformance ? TEXT("ENABLED") : TEXT("DISABLED"));

		// Change shader batch mode

		FShaderPipelineCache::SetBatchMode(bSavingPerformance ? FShaderPipelineCache::BatchMode::Fast : FShaderPipelineCache::BatchMode::Background);

		// Disable world rendering

		if (auto* GameViewportClient{ GetGameInstance()->GetGameViewportClient() })
		{
			GameViewportClient->bDisableWorldRendering = bSavingPerformance;

			// Change streaming priority within a level

			if (auto* ViewportWorld{ GameViewportClient->GetWorld() })
			{
				if (auto* WorldSettings{ ViewportWorld->GetWorldSettings(false, false) })
				{
					WorldSettings->bHighPriorityLoadingLocal = bSavingPerformance;
				}
			}
		}

		// Obtain and apply HangDurationMultiplier from GConfig on the load screen

		if (bSavingPerformance)
		{
			auto HangDurationMultiplier{ 0.0 };
			if (!GConfig || !GConfig->GetDouble(TEXT("Core.System"), TEXT("LoadingScreenHangDurationMultiplier"), HangDurationMultiplier, GEngineIni))
			{
				HangDurationMultiplier = 1.0;
			}

			FThreadHeartBeat::Get().SetDurationMultiplier(HangDurationMultiplier);
			FGameThreadHitchHeartBeat::Get().SuspendHeartBeat();
		}

		// Restore the original settings

		else
		{
			FThreadHeartBeat::Get().SetDurationMultiplier(1.0);
			FGameThreadHitchHeartBeat::Get().ResumeHeartBeat();
		}
	}
}
