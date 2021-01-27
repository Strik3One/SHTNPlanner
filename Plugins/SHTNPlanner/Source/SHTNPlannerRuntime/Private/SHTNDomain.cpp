// Fill out your copyright notice in the Description page of Project Settings.


#include "SHTNDomain.h"
#include "SHTNOperator_BlueprintBase.h"

DEFINE_LOG_CATEGORY(SHTNPlannerRuntime);

FSHTNDomain::FSHTNDomain()
{
	RootTaskName = NAME_None;
	bIsValid = false;
}

FSHTNDomain::~FSHTNDomain()
{
}

bool FSHTNDomain::Validate(bool bDebug)
{
	bool bValid = true;

	if (!IsCompositeTask(RootTaskName))
	{
		bValid = false;

		if (bDebug)
		{
			UE_LOG(SHTNPlannerRuntime, Error, TEXT("The root task %s is not a composite task"), *RootTaskName.ToString());
		}
	}

	TArray<FSHTNCompositeTask> CompositeTaskArray;
	CompositeTasks.GenerateValueArray(CompositeTaskArray);

	// Checks if every tasks in every method of every composite task is a valid task in the network. 
	for (const FSHTNCompositeTask& CompositeTask : CompositeTaskArray)
	{
		for (const FSHTNMethod& Method : CompositeTask.Methods)
		{
			bool FirstTask = true;

			for (const FName& Task : Method.Tasks)
			{
				if (FirstTask)
				{
					FirstTask = false;

					if (CompositeTask.Type == ESHTNCompositeType::Scored)
					{
						if (!IsPrimitiveTask(Task))
						{
							bValid = false;
							UE_LOG(SHTNPlannerRuntime, Error, TEXT("Task %s is expected to be primitive due to owning Composite being of type 'Scored'"), *Task.ToString());

							continue;
						}
					}
				}

				if (!(IsPrimitiveTask(Task) || IsCompositeTask(Task)))
				{
					bValid = false;

					if (bDebug)
					{
						UE_LOG(SHTNPlannerRuntime, Error, TEXT("Task %s does not exist in the network"), *Task.ToString());
					}
				}
			}
		}
	}

	bIsValid = bValid;

	return bValid;
}

void FSHTNDomain::SortMethodsByTaskType(FSHTNCompositeTask& Task, UWorldStateComponent* WorldState)
{
	if (Task.Type == ESHTNCompositeType::Default)
	{
		return;
	}

	for (const FSHTNMethod& Method : Task.Methods)
	{
		FSHTNPrimitiveTask& PrimitiveTask = GetPrimitiveTask(Method.Tasks[0]);
		PrimitiveTask.Score = PrimitiveTask.ActionPtr->GetScores(WorldState, PrimitiveTask.Parameter);
	}

	Task.Methods.Sort([this](const FSHTNMethod LHS, const FSHTNMethod RHS) {
		const FSHTNPrimitiveTask& LHSPrimitiveTask = GetPrimitiveTask(LHS.Tasks[0]);
		const FSHTNPrimitiveTask& RHSPrimitiveTask = GetPrimitiveTask(RHS.Tasks[0]);

		return LHSPrimitiveTask.Score > RHSPrimitiveTask.Score;
	});
}

int32 FSHTNCompositeTask::FindSatisfiedMethod(const FSHTNWorldState & WorldState, const int32 StartIndex) const
{
	for (int32 MethodIndex = StartIndex; MethodIndex < Methods.Num(); MethodIndex++)
	{
		if (WorldState.CheckConditions(Methods[MethodIndex].Conditions))
		{
			return MethodIndex;
		}
	}
	return INDEX_NONE;
}

//---------------------------------//
//	WorldState
//--------------------------------//

FSHTNWorldState::FSHTNWorldState()
{
}

FSHTNWorldState::~FSHTNWorldState()
{
}

void FSHTNWorldState::Init(const uint32 NewWorldStateSize)
{
	Values.Reset(NewWorldStateSize);
	Values.AddZeroed(NewWorldStateSize);
}

bool FSHTNWorldState::CheckCondition(const FSHTNCondition & Condition) const
{
	ESHTNWorldStateCheck CheckType = ESHTNWorldStateCheck(Condition.Operation);

	FSHTNDefs::FWSValue LHSValue = Values[Condition.KeyLeftHand];
	FSHTNDefs::FWSValue RHSValue = Condition.IsRHSAbsolute() ? Condition.Value : Values[Condition.KeyRightHand];

	switch (CheckType)
	{
	case ESHTNWorldStateCheck::Equal:
		return LHSValue == RHSValue;

	case ESHTNWorldStateCheck::NotEqual:
		return LHSValue != RHSValue;

	case ESHTNWorldStateCheck::Greater:
		return LHSValue > RHSValue;

	case ESHTNWorldStateCheck::GreaterOrEqual:
		return LHSValue >= RHSValue;

	case ESHTNWorldStateCheck::Less:
		return LHSValue < RHSValue;

	case ESHTNWorldStateCheck::LessOrEqual:
		return LHSValue <= RHSValue;

	case ESHTNWorldStateCheck::IsTrue:
		return LHSValue != 0;

	case ESHTNWorldStateCheck::IsFalse:
		return LHSValue == 0;

	default:
		UE_LOG(SHTNPlannerRuntime, Warning, TEXT("Default case hit in CheckCondition"));
		return false;
	}
}

bool FSHTNWorldState::CheckConditions(const TArray<FSHTNCondition>& Conditions) const
{
	bool bCurrentResult = true;

	for (int32 ConditionIndex = 0; ConditionIndex < Conditions.Num(); ++ConditionIndex)
	{
		bool bConditionResult = CheckCondition(Conditions[ConditionIndex]);
		
		if (ConditionIndex == 0)
		{
			bCurrentResult = bConditionResult;
		}
		else
		{
			if (Conditions[ConditionIndex - 1].LogicalRelationToNextCondition == ESHTNLogicalOperators::AND)
			{
				bCurrentResult = bCurrentResult && bConditionResult;
			}
			else // Will be OR
			{
				bCurrentResult = bConditionResult || bConditionResult;
			}
		}
	}
	return bCurrentResult;
}

void FSHTNWorldState::ApplyEffect(const FSHTNEffect & Effect)
{
	ESHTNWorldStateOperation EffectType = ESHTNWorldStateOperation(Effect.Operation);

	FSHTNDefs::FWSValue RHSValue = Effect.IsRHSAbsolute() ? Effect.Value : Values[Effect.KeyRightHand];

	switch (EffectType)
	{
	case ESHTNWorldStateOperation::Set:
		Values[Effect.KeyLeftHand] = RHSValue;
		return;

	case ESHTNWorldStateOperation::Increase:
		Values[Effect.KeyLeftHand] += RHSValue;
		return;

	case ESHTNWorldStateOperation::Decrease:
		Values[Effect.KeyLeftHand] -= RHSValue;
		return;

	default:
		UE_LOG(SHTNPlannerRuntime, Warning, TEXT("Default case hit in ApplyEffect"));
		return;
	}
}

bool FSHTNWorldState::GetValue(const FSHTNDefs::FWSKey Key, FSHTNDefs::FWSValue & OutValue) const
{
	if (Key < FSHTNDefs::FWSKey(Values.Num()))
	{
		OutValue = Values[Key];
		return true;
	}

	return false;
}

bool FSHTNWorldState::SetValue(const FSHTNDefs::FWSKey Key, const FSHTNDefs::FWSValue InValue)
{
	if (Key < FSHTNDefs::FWSKey(Values.Num()))
	{
		Values[Key] = InValue;
		return true;
	}

	return false;
}
