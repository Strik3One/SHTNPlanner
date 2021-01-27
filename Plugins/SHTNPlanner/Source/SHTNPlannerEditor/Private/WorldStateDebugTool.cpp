// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldStateDebugTool.h"

#include "Kismet/GameplayStatics.h"
#include "Editor.h"
#include "AIController.h"
#include "SHTNComponent.h"
#include "DetailCustomizations/WorldStateDebugDetails.h"

UWorldStateDebugTool::UWorldStateDebugTool()
{
	PlayingWorld = nullptr;
	bIsPlayingWorld = false;
	CurrentIndex = -1;
}

void UWorldStateDebugTool::OnCreation(FWorldStateDebugDetails* DebugDetails)
{
	// Always save the new pointer to the details as this object get recreated
	DetailBuilderPtr = DebugDetails;

	if (bCreated)
	{
		if (SelectedComponent)
		{
			OnNewPlanMadeHandle = SelectedComponent->OnNewPlanMade.AddSP(DebugDetails, &FWorldStateDebugDetails::OnNewPlanMade);
		}

		return;
	}

	bCreated = true;

	// Bind events to the beginning and end of a PIE session in order to refresh the details
	FEditorDelegates::PostPIEStarted.AddUObject(this, &UWorldStateDebugTool::ReceivePostPIEStarted);
	FEditorDelegates::EndPIE.AddUObject(this, &UWorldStateDebugTool::ReceiveEndPIE);

	// See if there is already a world playing upon creation of this object
	if (GEditor->PlayWorld)
	{
		PlayingWorld = GEditor->PlayWorld;
		bIsPlayingWorld = true;

		FindActiveComponents();

		if (HTNComponents.Num() > 0)
		{
			SelectedComponent = HTNComponents[0];
			CurrentIndex = 0;

			OnNewPlanMadeHandle = SelectedComponent->OnNewPlanMade.AddSP(DebugDetails, &FWorldStateDebugDetails::OnNewPlanMade);
		}
	}
}

void UWorldStateDebugTool::ReceivePostPIEStarted(bool bIsSimulating)
{
	if (GEditor->PlayWorld)
	{
		PlayingWorld = GEditor->PlayWorld;
		bIsPlayingWorld = true;

		FindActiveComponents();

		if (HTNComponents.Num() > 0)
		{
			SelectedComponent = HTNComponents[0];
			CurrentIndex = 0;
		}

		DetailBuilderPtr->RefreshDetails();
	}
}

void UWorldStateDebugTool::ReceiveEndPIE(bool bIsSimulating)
{
	PlayingWorld = nullptr;
	bIsPlayingWorld = false;
	SelectedComponent = nullptr;
	HTNComponents.Empty();

	DetailBuilderPtr->RefreshDetails();
}

void UWorldStateDebugTool::FindActiveComponents()
{
	if (IsValid(PlayingWorld))
	{
		HTNComponents.Empty();
		TArray<AActor*> FoundAgents;

		// Find all AI Controllers and see if they havbe an SHTNComponent as BrainComponent
		TSubclassOf<AAIController> classToFind = AAIController::StaticClass();
		UGameplayStatics::GetAllActorsOfClass(PlayingWorld, classToFind, FoundAgents);

		for (AActor* ControllerActor : FoundAgents)
		{
			AAIController* Controller = CastChecked<AAIController>(ControllerActor);

			USHTNComponent* AgentHTNComponent = Cast<USHTNComponent>(Controller->GetBrainComponent());

			if (AgentHTNComponent)
			{
				HTNComponents.Add(AgentHTNComponent);
			}
		}
	}
}
