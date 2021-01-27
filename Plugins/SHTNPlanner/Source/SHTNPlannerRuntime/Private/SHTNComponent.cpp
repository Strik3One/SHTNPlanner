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
	bIsPaused = false;
	bIsRunning = true;
	bCurrentExecutionSuccessful = true;
}

void USHTNComponent::StartLogic()
{
	if (bIsRunning)
	{
		UE_LOG(SHTNPlannerRuntime, Warning, TEXT("%s already has a running planner. Skipping logic.."), *GetName());
		return;
	}

	if (Domain.IsValid())
	{
		bIsRunning = true;
	}
}

void USHTNComponent::RestartLogic()
{
	StopLogic("Restarting");
	StartLogic();
}

void USHTNComponent::StopLogic(const FString & Reason)
{
	if (!bIsRunning)
	{
		UE_LOG(SHTNPlannerRuntime, Warning, TEXT("%s doesnt have a running planner. Skipping logic.."), *GetName());
		return;
	}

	if (CurrentOperator)
	{
		CurrentOperator->Abort(AIOwner, CurrentTask.Parameter);
		CurrentOperator = nullptr;
	}

	bReplan = true;
	bIsRunning = false;
}

void USHTNComponent::PauseLogic(const FString & Reason)
{
	UE_LOG(SHTNPlannerRuntime, Warning, TEXT("Execution updates: PAUSED (%s)"), *Reason);
	bIsPaused = true;
}

EAILogicResuming::Type USHTNComponent::ResumeLogic(const FString & Reason)
{
	const EAILogicResuming::Type SuperResumeResult = Super::ResumeLogic(Reason);

	bIsPaused = false;
	
	return SuperResumeResult;
}

bool USHTNComponent::IsRunning() const
{
	return !bIsPaused && bIsRunning;
}

bool USHTNComponent::IsPaused() const
{
	return bIsPaused;
}

void USHTNComponent::Cleanup()
{
	StopLogic("Cleanup");
	SHTNOperators.Empty();
}

void USHTNComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	WorkingWorldState->RemoveFromRoot();
}

EBlackboardNotificationResult USHTNComponent::OnBlackboardKeyValueChange(const UBlackboardComponent& Blackboard, FBlackboard::FKey ChangedKeyID)
{
	if (Blackboard.GetUniqueID() == BlackboardState->GetUniqueID())
	{
		bReplan = true;
		return EBlackboardNotificationResult::ContinueObserving;
	}

	return EBlackboardNotificationResult::RemoveObserver;

}

void USHTNComponent::GeneratePlan()
{
	if (Domain.IsValid())
	{
		WorkingWorldState->SetValueMemory(BlackboardState->GetValueMemory());
		Planner.CreatePlan(Domain, WorkingWorldState, Plan);
		DebugTaskNames = Plan.TaskNames;

		OnNewPlanMade.Broadcast();
	
		bReplan = false;

		if (Plan.TaskNames.Num() > 0)
		{
			UE_LOG(SHTNPlannerRuntime, Log, TEXT("Plan result for: %s with %i cycles"), *AIOwner->GetPawn()->GetActorLabel(), Plan.Cycles);
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

			//TODO: More detailed errors on failed plan
		}
	}
	else
	{
		UE_LOG(SHTNPlannerRuntime, Error, TEXT("Invalid Domain for: %s"), *AIOwner->GetPawn()->GetActorLabel());
	}
}

void USHTNComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction)
{
	// If not running or paused return immedeatly
	if (!bIsRunning || bIsPaused)
	{
		return;
	}

	// If there are no more tasks in the plan or if a task fails, a replan will be needed
	// Abort any running tasks and set the operator to nullptr
	if (bReplan)
	{
		if (CurrentOperator)
		{
			// First check if the Operator has somehow succeeded whilst we are requesting a replan
			// This can occure if the operator succeeds in the same frame as it wants to replan
			// But also when the user decides the task succeeded whilst aborting it
			// If this is the case we still want to apply the effects as the user is expecting these to be applied upon calling FinishExecute(bSuccess = true)
			if (CurrentOperator->GetResult() == ESHTNOperatorResult::Success)
			{
				// This sets the activation of the operator to false - next time we need this operator it will be reintialized
				CurrentOperator->Terminate();

				BlackboardState->PauseObserverNotifications();
				CurrentOperator->ApplyEffects(BlackboardState, CurrentTask.Parameter, false);
				BlackboardState->ResumeObserverNotifications(false);

				CurrentOperator = nullptr;
			}
			else
			{
				CurrentOperator->Abort(AIOwner, CurrentTask.Parameter);

				if (CurrentOperator->GetResult() != ESHTNOperatorResult::Aborted)
				{
					return;
				}

				CurrentOperator = nullptr;
			}
		}

		GeneratePlan();
	}

	// If the CurrentOperator is not assigned we look if there are more tasks in the plan and assign the CurrentOperator to the correct one
	// Else force a replan as there are no more tasks to execute
	if (CurrentOperator == nullptr)
	{
		if (Plan.TaskSequence.Num() > 0)
		{
			Plan.GetNextTask(CurrentTask, CurrentTaskName);
			CurrentOperator = CurrentTask.ActionPtr;

			if (!CurrentOperator->CheckCondition(BlackboardState, (uint8)CurrentTask.Parameter))
			{
				bReplan = true;
				CurrentOperator = nullptr;
				UE_LOG(SHTNPlannerRuntime, Log, TEXT("%s is replanning due failed condition at runtime..."), *AIOwner->GetPawn()->GetActorLabel());
				return;
			}
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
			bCurrentExecutionSuccessful = true;
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
			bCurrentExecutionSuccessful = false;
			UE_LOG(SHTNPlannerRuntime, Log, TEXT("%s is replanning due to failed operator result of: %s with parameter %i"), *AIOwner->GetPawn()->GetActorLabel(), *CurrentOperator->CDOName.ToString(), CurrentTask.Parameter);
		}

		// This sets the activation of the operator to false - next time we need this operator it will be reintialized
		CurrentOperator->Terminate();

		if (bCurrentExecutionSuccessful)
		{
			//Pause observers and apply effectsm then resume observing without broadcasting the changed values in the effects.
			BlackboardState->PauseObserverNotifications();
			CurrentOperator->ApplyEffects(BlackboardState, CurrentTask.Parameter, false);
			BlackboardState->ResumeObserverNotifications(false);
		}

		// Setting the operator to null to start the circle of finding a new task
		CurrentOperator = nullptr;
	}
}
