// Fill out your copyright notice in the Description page of Project Settings.

#include "SHTNPlannerEditor.h"

#include "Modules/ModuleManager.h"

#include "DetailCustomizations/UserDefinedWorldStateDetails.h"
#include "DetailCustomizations/UserDefinedOperatorDetails.h"
#include "PropertyEditorModule.h"

DEFINE_LOG_CATEGORY(SHTNPlannerEditor);

#define LOCTEXT_NAMESPACE "FSHTNPlannerEditor"

void FSHTNPlannerEditor::StartupModule()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomPropertyTypeLayout("UserDefinedWorldState", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FUserDefinedWorldStateDetails::MakeInstance));
	PropertyModule.RegisterCustomPropertyTypeLayout("UserDefinedOperator", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FUserDefinedOperatorDetails::MakeInstance));
}

void FSHTNPlannerEditor::ShutdownModule()
{
	// Unregister the details customization
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomPropertyTypeLayout("UserDefinedWorldState");
		PropertyModule.UnregisterCustomPropertyTypeLayout("UserDefinedOperator");
		PropertyModule.NotifyCustomizationModuleChanged();
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSHTNPlannerEditor, SHTNPlannerEditor);