#pragma once

#include "CoreMinimal.h"
#include "SHTNDomain.h"

struct SHTNPLANNERRUNTIME_API FSHTNPlan
{
	TArray<FName> TaskNames;
	TArray<FSHTNPrimitiveTask> TaskSequence;
	int32 Cycles;

	void Reset()
	{
		TaskNames.Reset();
		TaskSequence.Reset();
	}

	void Set(const FSHTNDomain& Domain, const TArray<FName>& NameSequence, int32 InCycles);

	void GetNextTask(FSHTNPrimitiveTask& InTask, FName& InName)
	{
		InTask = TaskSequence[0];
		InName = TaskNames[0];

		TaskSequence.RemoveAt(0);
		TaskNames.RemoveAt(0);
	}
};

struct SHTNPLANNERRUNTIME_API FSHTNDecompState
{
	TArray<uint8> WSValueMemory;
	TArray<FName> Plan;
	TArray<FName> TasksToProcess;
	int32 NextMethod;
	FName ActiveTask;

	FSHTNDecompState()
		: NextMethod(0), ActiveTask(NAME_None)
	{
	}

	explicit FSHTNDecompState(const UWorldStateComponent& InWorldState)
		: NextMethod(0), ActiveTask(NAME_None)
	{
		Plan.Reserve(5);

		WSValueMemory = InWorldState.GetValueMemory();
	}

	FSHTNDecompState(const UWorldStateComponent& InWorldState, const TArray<FName> InPlan, const TArray<FName> InTasksToProcess, const FName CurrentTask, const int32 InNextMehod)
		: Plan(InPlan), TasksToProcess(InTasksToProcess), NextMethod(InNextMehod), ActiveTask(CurrentTask)
	{
		WSValueMemory = InWorldState.GetValueMemory();
	}
};

struct SHTNPLANNERRUNTIME_API FSHTNPlanner
{
	FSHTNPlanner();

	bool CreatePlan(FSHTNDomain& Domain, UWorldStateComponent* WorkingWorldState, FSHTNPlan& OutPlan);

protected:

	void RecordDecomposition(const FName CurrentTask, const int32 MethodIndex)
	{
		DecompStates.Push(FSHTNDecompState(*WorldState, CurrentState.Plan, CurrentState.TasksToProcess, CurrentTask, MethodIndex));
	}

	void RestoreDecomposition() 
	{ 
		CurrentState = DecompStates.Pop(false); 
		WorldState->SetValueMemory(CurrentState.WSValueMemory);
	}

	bool CanRollBack() { return DecompStates.Num() > 0; }
	int32 GetMethodIndex() const { return CurrentState.NextMethod; }
	FName GetActiveTask() const { return CurrentState.ActiveTask; }
	const TArray<FName> GetPlan() const { return CurrentState.Plan; }
	void AddToPlan(const FName CurrentTask) { CurrentState.Plan.Push(CurrentTask); }

	void Reset()
	{
		DecompStates.Reset();
	}

	FSHTNDecompState CurrentState;

	UWorldStateComponent* WorldState;
	TArray<FSHTNDecompState> DecompStates;

	TArray<FName> TasksToProcess;
};