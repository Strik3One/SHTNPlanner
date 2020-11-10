#pragma once

#include "CoreMinimal.h"
#include "SHTNDomain.h"

struct SHTNPLANNERRUNTIME_API FSHTNPlan
{
	TArray<FName> TaskNames;
	TArray<FSHTNPrimitiveTask> TaskSequence;

	void Reset()
	{
		TaskNames.Reset();
		TaskSequence.Reset();
	}

	void Set(const FSHTNDomain& Domain, const TArray<FName>& NameSequence);
};

struct SHTNPLANNERRUNTIME_API FSHTNDecompState
{
	FSHTNWorldState WorldState;
	TArray<FName> Plan;
	int32 NextMethod;
	FName ActiveTask;

	explicit FSHTNDecompState(const FSHTNWorldState& InWorldState = FSHTNWorldState())
		: WorldState(InWorldState), NextMethod(0), ActiveTask(NAME_None)
	{
		Plan.Reserve(5);
	}

	FSHTNDecompState(const FSHTNDecompState& InDecompState, const FName CurrentTask, const int32 InNextMehod)
		: WorldState(InDecompState.WorldState), Plan(InDecompState.Plan), NextMethod(InNextMehod), ActiveTask(CurrentTask)
	{}

};

struct SHTNPLANNERRUNTIME_API FSHTNPlanner
{
	FSHTNPlanner();

	bool CreatePlan(const FSHTNDomain& Domain, const FSHTNWorldState& InitialWordState, FSHTNPlan& OutPlan);

protected:

	void RecordDecomposition(const FName CurrentTask, const int32 MethodIndex) { DecompStates.Push(FSHTNDecompState(CurrentState, CurrentTask, MethodIndex)); }
	void RestoreDecomposition() { CurrentState = DecompStates.Pop(false); }
	bool CanRollBack() { return DecompStates.Num() > 0; }
	FSHTNWorldState& CurrentWorldState() { return CurrentState.WorldState; }
	int32 GetMethodIndex() const { return CurrentState.NextMethod; }
	FName GetActiveTask() const { return CurrentState.ActiveTask; }
	const TArray<FName> GetPlan() const { return CurrentState.Plan; }
	void AddToPlan(const FName CurrentTask) { CurrentState.Plan.Push(CurrentTask); }

	void Reset()
	{
		DecompStates.Reset();
		CurrentState.Plan.Reset();
	}

	FSHTNDecompState CurrentState;
	TArray<FSHTNDecompState> DecompStates;
	TArray<FName> TasksToProcess;
};