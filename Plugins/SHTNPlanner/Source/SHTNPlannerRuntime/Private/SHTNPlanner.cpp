#include "SHTNPlanner.h"

//---------------------------------//
//	SHTNPlan
//--------------------------------//

void FSHTNPlan::Set(const FSHTNDomain& Domain, const TArray<FName>& NameSequence)
{
	TaskNames = NameSequence;
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

bool FSHTNPlanner::CreatePlan(const FSHTNDomain & Domain, const FSHTNWorldState & InitialWordState, FSHTNPlan & OutPlan)
{
	// Reset everything that is needed and copy the IntialWordstate to the CurrentState.
	OutPlan.Reset();
	Reset();
	new(&CurrentState) FSHTNDecompState(InitialWordState);

	// Begin at the root task of the domain.
	TasksToProcess.Push(Domain.RootTaskName);

	// As we decompose states and rollback to composites. We don't want to search the same method again, so it's important to increment this at decomposition
	int32 NextMethod = 0;

	// As long as there still are tasks to process we haven't reached the end of a plan and we will keep planning
	while (TasksToProcess.Num() > 0)
	{

		const FName CurrentTaskName = TasksToProcess.Pop(false);

		if (Domain.IsCompositeTask(CurrentTaskName))
		{
			// If the CurrentTask is a composite we will look if this composite has any methods which conditions are met
			const FSHTNCompositeTask& CompositeTask = Domain.GetCompositeTask(CurrentTaskName);
			const int32 MethodIndex = CompositeTask.FindSatisfiedMethod(CurrentWorldState(), NextMethod);


			if (MethodIndex != INDEX_NONE)
			{
				// If there was a method found (index returned is not INDEX_NONE) we record this decomposition
				const FSHTNMethod& Method = CompositeTask.Methods[MethodIndex];
				RecordDecomposition(CurrentTaskName, MethodIndex);

				// Set the NextMethod back to 0 to make sure we start at the first method of the next composite task we encounter
				NextMethod = 0;

				// Push the tasks defined in the Method to the tasks that need to be processed
				// We loop backwards because the process list in Last in First out (LiFo). And as the method defines its tasks FiFo we need to swap it around
				for (int32 TaskIndex = Method.Tasks.Num() - 1; TaskIndex >= 0; --TaskIndex)
				{
					TasksToProcess.Push(Method.Tasks[TaskIndex]);
				}
			}
			else if (CanRollBack())
			{
				// If there wasn't a method found and there are decomposed states in the stack (CanRollBack() returns true) we will rollback to the last known state
				RestoreDecomposition();
				// Increment the NextMethod to make sure we don't fall back into the same method and end up in an infinite loop
				NextMethod = GetMethodIndex() + 1;
				TasksToProcess.Push(GetActiveTask());
			}
		}
		else if (Domain.IsPrimitiveTask(CurrentTaskName))
		{
			// If the CurrentTask is a primitive, there isn't much magic involed. Just get the task, apply the effects (for further condition checking in the plan)
			// And finally add the primitive to the Plan
			const FSHTNPrimitiveTask& PrimitiveTask = Domain.GetPrimitiveTask(CurrentTaskName);
			CurrentWorldState().ApplyEffects(PrimitiveTask.Effects);
			AddToPlan(CurrentTaskName);
		}
	}


	bool bPlanningSuccessful = (TasksToProcess.Num() == 0);
	if (bPlanningSuccessful)
	{
		OutPlan.Set(Domain, GetPlan());
	}

	return bPlanningSuccessful;
}
