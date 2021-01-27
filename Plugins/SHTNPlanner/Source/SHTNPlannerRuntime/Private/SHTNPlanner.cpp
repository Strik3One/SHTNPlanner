#include "SHTNPlanner.h"
#include "SHTNOperator_BlueprintBase.h"

//---------------------------------//
//	SHTNPlan
//--------------------------------//

void FSHTNPlan::Set(const FSHTNDomain& Domain, const TArray<FName>& NameSequence, int32 InCycles)
{
	TaskNames = NameSequence;
	Cycles = InCycles;

	TaskSequence.Reset();

	for (const FName Name : NameSequence)
	{
		if (Domain.IsPrimitiveTask(Name))
		{
			const FSHTNPrimitiveTask& Task = Domain.GetPrimitiveTask(Name);
			TaskSequence.Push(Task);
		}
	}
}

//---------------------------------//
//	SHTNPlanner
//--------------------------------//

FSHTNPlanner::FSHTNPlanner()
{

}

bool FSHTNPlanner::CreatePlan(FSHTNDomain & Domain, UWorldStateComponent* WorkingWorldState, FSHTNPlan & OutPlan)
{
	// Reset everything that is needed and copy the IntialWordstate to the CurrentState.
	OutPlan.Reset();
	Reset();
	new(&CurrentState) FSHTNDecompState(*WorkingWorldState);

	WorldState = WorkingWorldState;

	// Begin at the root task of the domain.
	CurrentState.TasksToProcess.Push(Domain.RootTaskName);

	// As we decompose states and rollback to composites. We don't want to search the same method again, so it's important to increment this at decomposition
	int32 NextMethod = 0;

	int32 PlanCycles = 0;

	while (CurrentState.TasksToProcess.Num() > 0 && PlanCycles++ < Domain.MaxPlanCycles)
	{
		//++PlanCycles;

		const FName CurrentTaskName = CurrentState.TasksToProcess.Pop(false);

		// If the current method is a Composite task, we should decompose the task into it's subtask.
		// We start with the first method, and through decomposition and restoring upon failed conditions we will explore all methods.
		if (Domain.IsCompositeTask(CurrentTaskName))
		{
			FSHTNCompositeTask& CompositeTask = Domain.GetCompositeTask(CurrentTaskName);

			Domain.SortMethodsByTaskType(CompositeTask, WorldState);

			if (NextMethod < CompositeTask.Methods.Num())
			{
				const FSHTNMethod& Method = CompositeTask.Methods[NextMethod];

				RecordDecomposition(CurrentTaskName, NextMethod);

				NextMethod = 0;

				// Add the tasks from the last one to the first as we use pop the last element off the array each cycle.
				for (int32 TaskIndex = Method.Tasks.Num() - 1; TaskIndex >= 0; --TaskIndex)
				{
					CurrentState.TasksToProcess.Push(Method.Tasks[TaskIndex]);
				}
			}
			else if (CanRollBack())
			{
				// If there wasn't a method found and there are decomposed states in the stack (CanRollBack() returns true) we will rollback to the last known state
				RestoreDecomposition();

				// Increment the NextMethod to make sure we don't fall back into the same method and end up in an infinite loop
				NextMethod = GetMethodIndex() + 1;
				CurrentState.TasksToProcess.Push(GetActiveTask());
			}
			else
			{
				break;
			}
		}
		else if (Domain.IsPrimitiveTask(CurrentTaskName))
		{
			// If the CurrentTask is a primitive, we need to check its conditions against the current WorldState.
			// If the conditions return true we will apply the effects and add the task to the plan
			const FSHTNPrimitiveTask& PrimitiveTask = Domain.GetPrimitiveTask(CurrentTaskName);

			if (PrimitiveTask.ActionPtr->CheckCondition(WorldState, PrimitiveTask.Parameter, true))
			{

				PrimitiveTask.ActionPtr->ApplyEffects(WorldState, PrimitiveTask.Parameter, true);
				
				AddToPlan(CurrentTaskName);
			}
			else if (CanRollBack())
			{
				// If there wasn't a method found and there are decomposed states in the stack (CanRollBack() returns true) we will rollback to the last known state
				RestoreDecomposition();
				// Increment the NextMethod to make sure we don't fall back into the same method and end up in an infinite loop
				NextMethod = GetMethodIndex() + 1;
				CurrentState.TasksToProcess.Push(GetActiveTask());
			}
			else
			{
				break;
			}
		}
	}

	bool bPlanningSuccessful = (CurrentState.TasksToProcess.Num() == 0);
	if (bPlanningSuccessful)
	{
		OutPlan.Set(Domain, GetPlan(), PlanCycles);
	}

	return bPlanningSuccessful;
}
