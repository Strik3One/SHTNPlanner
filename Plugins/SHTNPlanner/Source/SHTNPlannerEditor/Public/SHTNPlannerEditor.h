// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Widgets/SWindow.h"

class UWorldStateDebugTool;

DECLARE_LOG_CATEGORY_EXTERN(SHTNPlannerEditor, Log, All);

class FSHTNPlannerEditor : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	void StartupModule() override;
	void ShutdownModule() override;

	static void TriggerWorldStateDebug();

	static void OnToolWindowClosed(const TSharedRef<SWindow>& Window, UWorldStateDebugTool* Instance);
	
};
