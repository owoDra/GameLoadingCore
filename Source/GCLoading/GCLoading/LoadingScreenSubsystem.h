// Copyright (C) 2024 owoDra

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"

#include "LoadingScreenInputPreProcessor.h"

#include "GameplayTagContainer.h"

#include "LoadingScreenSubsystem.generated.h"

class SWidget;
class UUserWidget;
class ULoadingObserver;


/**
 * Delegate notifies you that the load screen has been shown/hidden.
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FLoadingScreenVisibilityChangedDelegate, bool);


/**
 * Information on ongoing loading
 */
USTRUCT(BlueprintType)
struct FLoadingScreenInfo
{
	GENERATED_BODY()
public:
	FLoadingScreenInfo() {}

public:
	//
	// Mapping list of process name and reasons
	//
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TMap<FName, FText> Processes;

	//
	// Widget class for this loading screen
	//
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSubclassOf<UUserWidget> WidgetClass{ nullptr };

	//
	// Priority when displaying this loading screen
	//
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 ZOrder{ 0 };

	//
	// Number of seconds this load screen will remain displayed after loading is complete.
	//
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float AdditionalSec{ 0.0f };

	//
	// Whether input should be blocked during loading?
	//
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bBlockInputs{ true };

	//
	// Whether the game saves performance during loading
	//
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bSavingPerfomance{ true };

};


/**
 * Subsystem that manages the display/hide of load screens
 */
UCLASS()
class GCLOADING_API ULoadingScreenSubsystem 
	: public UGameInstanceSubsystem
	, public FTickableGameObject
{
	GENERATED_BODY()
public:
	ULoadingScreenSubsystem() {}

	////////////////////////////////////////////////////////
	// Initialization
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;


	////////////////////////////////////////////////////////
	// Tick
public:
	virtual void Tick(float DeltaTime) override;
	virtual ETickableTickType GetTickableTickType() const override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override;
	virtual UWorld* GetTickableGameObjectWorld() const override;


	////////////////////////////////////////////////////////
	// Loading Observer
protected:
	//
	// List of currently active loading observers
	//
	UPROPERTY(Transient)
	TArray<TObjectPtr<ULoadingObserver>> ActiveObservers;

protected:
	void InitializeObservers();
	void DeinitializeObservers();
	void TickObservers(float DeltaTime);


	////////////////////////////////////////////////////////
	// Loading Widget Override
public:
	//
	// Mapping list of loading type tags and widget classes that override the widget to be displayed during loading .
	//
	UPROPERTY(Transient)
	TMap<FGameplayTag, TSubclassOf<UUserWidget>> LoadingWidgetOverrides;

public:
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Loading Screen", meta = (GameplayTagFilter = "LoadingType"))
	void AddLoadingWidgetOverride(FGameplayTag LoadingTypeTag, TSubclassOf<UUserWidget> WidgetClass);


	////////////////////////////////////////////////////////
	// Loading Processes Infos
public:
	//
	// Mapping list of handles with information on ongoing loading process
	//
	UPROPERTY(Transient)
	TMap<FGameplayTag, FLoadingScreenInfo> LoadingScreenInfos;

public:
	/**
	 * Loading screen notifies the start of the required loading process
	 * 
	 * !!!Note!!!:
	 *	Be sure to save the Handle and delete it using RemoveLoadingProcess() when the process is finished. 
	 *	Otherwise, the loading widget will remain.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Loading Screen", meta = (GameplayTagFilter = "LoadingType"))
	bool AddLoadingProcess(FName ProcessName, FGameplayTag LoadingTypeTag, FText Reason);

	/**
	 * Notifies the end of the loading process
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Loading Screen")
	bool RemoveLoadingProcess(FName ProcessName);

	/**
	 * Notifies the end of the loading process using tag
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Loading Screen", meta = (GameplayTagFilter = "LoadingType"))
	bool RemoveLoadingProcessByTag(FGameplayTag LoadingTypeTag);

protected:
	virtual bool AddLoadingProcessExistType(FLoadingScreenInfo& Info, FName ProcessName, const FGameplayTag& LoadingTypeTag, const FText& Reason);
	virtual bool AddLoadingProcessNew(FName ProcessName, const FGameplayTag& LoadingTypeTag, const FText& Reason, const FLoadingScreenDefinition& Def);

public:
	/**
	 * Get loading reason from handle
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Loading Screen")
	virtual const FText& GetLoadingReasonFromName(FName ProcessName) const;

	/**
	 * Get loading reasons from tag
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Loading Screen", meta = (GameplayTagFilter = "LoadingType"))
	virtual TArray<FText> GetLoadingReasonsFromTag(FGameplayTag LoadingTypeTag) const;


	////////////////////////////////////////////////////////
	// Loading Widgets
public:
	FLoadingScreenVisibilityChangedDelegate OnLoadingScreenVisibilityChanged;

protected:
	//
	// List of loading type tags for which the loading process is started and the loading widget needs to be added
	//
	UPROPERTY(Transient)
	TSet<FGameplayTag> PendingAddLoadingTags;

	//
	// Mapping List of loading type tags for which the loading process is finished and the loading widget needs to be removed
	// 
	// Key	 : LoadingTypeTag
	// Value : Start time of removal
	//
	UPROPERTY(Transient)
	TMap<FGameplayTag, double> PendingRemoveLoadingTags;

	//
	// Mapping list of loading widgets and their tags currently displayed
	//
	TMap<FGameplayTag, TSharedPtr<SWidget>> ShowingWidgets;

	//
	// Whether the loading screen is currently displayed or not
	//
	UPROPERTY(Transient)
	bool bLoadingWidgetDisplayed{ false };

protected:
	void AddTagToPendingAddList(const FGameplayTag & Tag);
	void AddTagToPendingRemoveList(const FGameplayTag& Tag);
	void CancelPendingRemove(const FGameplayTag& Tag);

	void UpdateLoadingWidgets();

	bool ProcessPendingAddTag(const FGameplayTag& Tag, bool bForceTick);
	bool ProcessPendingRemoveTag(const FGameplayTag& Tag, double StatTime, double CurrentTime, bool bShouldHold);

	void TryCreateLoadingWidget(const FGameplayTag& Tag, const TSubclassOf<UUserWidget>& Class, const int32& ZOrder);
	void TryRemoveLoadingWidget(const FGameplayTag& Tag);
	void RemoveAllWidgets();

	/**
	 * Returns whether or not the initialization load screen is being displayed before the normal load screen.
	 */
	bool IsShowingInitialLoadingScreen() const;

public:
	/**
	 * Get loading reasons from tag
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Loading Screen")
	virtual bool IsLoadingWidgetDisplayed() const { return bLoadingWidgetDisplayed; }


	////////////////////////////////////////////////////////
	// Input
protected:
	//
	// Processor to process input while the loading screen is displayed.
	//
	TSharedPtr<IInputProcessor> InputPreProcessor;

	//
	// Number of loading screens that need to be input blocked
	//
	UPROPERTY(Transient)
	int32 InputBlockCount{ 0 };

	//
	// Whether the input is currently blocked or not
	//
	UPROPERTY(Transient)
	bool bInputBlocked{ false };

protected:
	virtual void UpdateInputBlock();

	void IncrementInputBlockCount() { InputBlockCount++; UpdateInputBlock(); }
	void DecrementInputBlockCount() { InputBlockCount--; UpdateInputBlock(); }
	void ClearInputBlockCount() { InputBlockCount = 0; UpdateInputBlock(); }

	
	////////////////////////////////////////////////////////
	// Performace
protected:
	//
	// Number of loading screens that need to be saved performance
	//
	UPROPERTY(Transient)
	int32 SavingPerformanceCount{ 0 };

	//
	// Whether the peformance is currently saved or not
	//
	UPROPERTY(Transient)
	bool bSavingPerformance{ false };

protected:
	virtual void UpdatePerformance();

	void IncrementSavingPerformanceCount() { SavingPerformanceCount++; UpdatePerformance(); }
	void DecrementSavingPerformanceCount() { SavingPerformanceCount--; UpdatePerformance(); }
	void ClearSavingPerformanceCount() { SavingPerformanceCount = 0; UpdatePerformance(); }

};
