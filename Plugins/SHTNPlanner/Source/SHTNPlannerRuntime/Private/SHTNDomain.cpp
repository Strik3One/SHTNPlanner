// Fill out your copyright notice in the Description page of Project Settings.


#include "SHTNDomain.h"

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
			for (const FName& Task : Method.Tasks)
			{
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
