// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "WorldStateDebugTool.generated.h"

class USHTNComponent;
class FWorldStateDebugDetails;

UCLASS()
class UWorldStateDebugTool : public UObject
{
public:

	GENERATED_BODY()

	UWorldStateDebugTool();

	void OnCreation(FWorldStateDebugDetails* DetailBuilder);

	void ReceivePostPIEStarted(bool bIsSimulating);
	void ReceiveEndPIE(bool bIsSimulating);

	void FindActiveComponents();

	UWorld* PlayingWorld;
	bool bIsPlayingWorld;

	USHTNComponent* SelectedComponent;
	int32 CurrentIndex;

	TArray<USHTNComponent*> HTNComponents;

	FWorldStateDebugDetails* DetailBuilderPtr;

	FDelegateHandle OnNewPlanMadeHandle;

	bool bSelectInPIE;

protected:

	bool bCreated;
};
