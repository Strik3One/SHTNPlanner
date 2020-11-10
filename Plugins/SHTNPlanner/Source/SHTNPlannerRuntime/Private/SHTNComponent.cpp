// Fill out your copyright notice in the Description page of Project Settings.

#include "SHTNComponent.h"
#include "AIController.h"

USHTNComponent::USHTNComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	CurrentOperator = nullptr;
	bReplan = true;
}

void USHTNComponent::GeneratePlan()
{
	if (Domain.IsValid())
	{
		Planner.CreatePlan(Domain, WorldState, Plan);
		bReplan = false;

		if (Plan.TaskNames.Num() > 0)
		{
			UE_LOG(SHTNPlannerRuntime, Log, TEXT("Plan result for: %s"), *AIOwner->GetPawn()->GetActorLabel());
			UE_LOG(SHTNPlannerRuntime, Log, TEXT("----------------------------------------"));
			for (int32 TaskIndex = 0; TaskIndex < Plan.TaskNames.Num(); ++TaskIndex)
			{
				UE_LOG(SHTNPlannerRuntime, Log, TEXT("%d : %s"), TaskIndex, *Plan.TaskNames[TaskIndex].ToString());
			}
			UE_LOG(SHTNPlannerRuntime, Log, TEXT("----------------------------------------"));
		}
		else
		{
			UE_LOG(SHTNPlannerRuntime, Error, TEXT("Failed to find plan for: %s"), *AIOwner->GetPawn()->GetActorLabel());
		}
	}
	else
	{
		UE_LOG(SHTNPlannerRuntime, Error, TEXT("Invalid Domain for: %s"), *AIOwner->GetPawn()->GetActorLabel());
	}
}

void USHTNComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction)
{
	// If there are no more tasks in the plan or if a task fails, a replan will be needed
	// Abort any running tasks and set the operator to nullptr
	if (bReplan)
	{
		if (CurrentOperator)
		{
			CurrentOperator->Abort(AIOwner, CurrentTask.Parameter);
			CurrentOperator = nullptr;
		}

		GeneratePlan();
	}

	// If the CurrentOperator is not assigned we look if there are more tasks in the plan and assign the CurrentOperator to the correct one
	// Else force a replan as there are no more tasks to execute
	if (CurrentOperator == nullptr)
	{
		if (Plan.TaskSequence.Num() > 0)
		{
			CurrentTask = Plan.TaskSequence[0];
			Plan.TaskSequence.RemoveAt(0);
			CurrentOperator = SHTNOperators[(uint8)CurrentTask.ActionID];
		}
		else
		{
			
			bReplan = true;
			UE_LOG(SHTNPlannerRuntime, Log, TEXT("%s is replanning due to reaching end of plan..."), *AIOwner->GetPawn()->GetActorLabel());
			return;
		}
	}

	if (CurrentOperator)
	{
		// Perform Initialisation for the Operator, initializing the operator will set the result to InProgress
		if (!CurrentOperator->IsActivated())
		{
			CurrentOperator->Initialize(AIOwner, (uint8)CurrentTask.Parameter);
		}

		// As long as the operator is InProgress, execute and return
		// But if the result is failed, we need to initiate a replan
		if (CurrentOperator->GetResult() == ESHTNOperatorResult::InProgress)
		{
			CurrentOperator->Execute(AIOwner, (uint8)CurrentTask.Parameter, DeltaTime);
			return;
		}
		else if(CurrentOperator->GetResult() == ESHTNOperatorResult::Failed)
		{
			bReplan = true;
			UE_LOG(SHTNPlannerRuntime, Log, TEXT("%s is replanning due to failed operator result..."), *AIOwner->GetPawn()->GetActorLabel());
		}

		// This sets the activation of the operator to false - next time we need this operator it will be reintialized
		CurrentOperator->Terminate();

		// Here we will get the actual effects of this operator and apply it to the world state
		TArray<FSHTNWorldStateEffect> ActualEffects;
		CurrentOperator->GetActualEffects(ActualEffects, AIOwner);

		for (const auto& Effect : ActualEffects)
		{
			WorldState.ApplyEffect(FSHTNEffect(Effect.WSKey, Effect.Operation).SetRHSAsValue(Effect.Value));
		}

		// Setting the operator to null to start the circle of finding a new task
		CurrentOperator = nullptr;
	}
}
