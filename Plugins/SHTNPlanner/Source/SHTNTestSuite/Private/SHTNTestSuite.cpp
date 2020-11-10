// Fill out your copyright notice in the Description page of Project Settings.

#include "SHTNTestSuite.h"

#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(SHTNTestSuite);

#define LOCTEXT_NAMESPACE "FSHTNTestSuite"
void FSHTNTestSuite::StartupModule()
{
}

void FSHTNTestSuite::ShutdownModule()
{
	
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSHTNTestSuite, SHTNTestSuite);