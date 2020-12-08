// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "SHTNDomain.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(SHTNPlannerRuntime, Log, All);

struct FSHTNWorldState;

namespace FSHTNDefs
{
	typedef int32 FWSValue;
	typedef uint8 FWSKey;
	typedef uint8 FWSOperationID;
	typedef uint16 FActionID;
	typedef int32 FActionParameter;

	const FWSKey InvalidWSKey = FWSKey(MAX_uint8);
	const FWSOperationID InvalidWSOperation = FWSOperationID(MAX_uint8);
}

UENUM()
enum class ESHTNWorldStateCheck : uint8
{
	Equal,
	NotEqual,
	Less,
	LessOrEqual,
	Greater,
	GreaterOrEqual,
	IsTrue,
	IsFalse,
	
	MAX UMETA(Hidden)
};

UENUM()
enum class ESHTNWorldStateOperation : uint8
{
	Set,
	Increase,
	Decrease,

	MAX UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FSHTNCondition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESHTNWorldStateCheck Operation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 KeyLeftHand;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 KeyRightHand;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Value;

	FSHTNCondition(const uint8 InKeyLeftHand = FSHTNDefs::InvalidWSKey, const ESHTNWorldStateCheck InOperation = ESHTNWorldStateCheck::MAX)
		: Operation(InOperation), KeyLeftHand(InKeyLeftHand), KeyRightHand(FSHTNDefs::InvalidWSKey), Value(0)
	{
	}

	template<typename TAlternativeKey>
	FSHTNCondition(const TAlternativeKey InKeyLeftHand, const ESHTNWorldStateCheck InOperation = ESHTNWorldStateCheck::MAX)
		: FSHTNCondition(uint8(InKeyLeftHand), InOperation)
	{
	}

	FSHTNCondition& SetRHSAsWSKey(const FSHTNDefs::FWSKey InKeyRightHand)
	{
		KeyRightHand = InKeyRightHand;
		return *this;
	}

	template<typename TAlternativeKey>
	FSHTNCondition& SetRHSAsWSKey(const TAlternativeKey InKeyRightHand)
	{
		return SetRHSAsWSKey(FSHTNDefs::FWSKey(InKeyRightHand));
	}

	FSHTNCondition& SetRHSAsValue(const FSHTNDefs::FWSValue InValue)
	{
		Value = InValue;
		return *this;
	}

	FORCEINLINE bool IsRHSAbsolute() const { return KeyRightHand == FSHTNDefs::InvalidWSKey; }

	bool IsValid() const { return (Operation != ESHTNWorldStateCheck::MAX); }
};

USTRUCT(BlueprintType)
struct FSHTNEffect
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESHTNWorldStateOperation Operation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 KeyLeftHand;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 KeyRightHand;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Value;

	FSHTNEffect(const uint8 InKeyLeftHand = FSHTNDefs::InvalidWSKey, const ESHTNWorldStateOperation InOperation = ESHTNWorldStateOperation::MAX)
		: Operation(InOperation), KeyLeftHand(InKeyLeftHand), KeyRightHand(FSHTNDefs::InvalidWSKey), Value(0)
	{
	}

	template<typename TAlternativeKey>
	FSHTNEffect(const TAlternativeKey InKeyLeftHand, const ESHTNWorldStateOperation InOperation = ESHTNWorldStateOperation::MAX)
		: FSHTNEffect(uint8(InKeyLeftHand), InOperation)
	{
	}

	FSHTNEffect& SetRHSAsWSKey(const FSHTNDefs::FWSKey InKeyRightHand)
	{
		KeyRightHand = InKeyRightHand;
		return *this;
	}

	template<typename TAlternativeKey>
	FSHTNEffect& SetRHSAsWSKey(const TAlternativeKey InKeyRightHand)
	{
		return SetRHSAsWSKey(FSHTNDefs::FWSKey(InKeyRightHand));
	}

	FSHTNEffect& SetRHSAsValue(const FSHTNDefs::FWSValue InValue)
	{
		Value = InValue;
		return *this;
	}

	FORCEINLINE bool IsRHSAbsolute() const { return KeyRightHand == FSHTNDefs::InvalidWSKey; }

	bool IsValid() const { return (Operation != ESHTNWorldStateOperation::MAX); }
};

USTRUCT(BlueprintType)
struct SHTNPLANNERRUNTIME_API FSHTNPrimitiveTask
{
	GENERATED_BODY()

	// We call the Action ID and the parameter together the "Operator"
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 ActionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 Parameter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FSHTNEffect> Effects;

	void SetOperator(const uint8 InActionID, const uint8 InParameter)
	{
		ActionID = InActionID;
		Parameter = InParameter;
	}

	// This template function makes it easier to set the action parameter with any type of enum
	template<typename A, typename B = uint8>
	void SetOperator(const A InActionID, const B InParameter = B(0))
	{
		SetOperator(uint8(InActionID), uint8(InParameter));
	}

	void AddEffect(const FSHTNEffect& Effect) { Effects.Add(Effect); }
};

USTRUCT(BlueprintType)
struct SHTNPLANNERRUNTIME_API FSHTNMethod
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FSHTNCondition> Conditions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> Tasks;

	FSHTNMethod()
	{}

	FSHTNMethod(const FSHTNCondition& Condition)
	{
		Conditions.Add(Condition);
	}

	FSHTNMethod(const TArray<FSHTNCondition>& InConditions)
		: Conditions(InConditions)
	{}

	void AddTask(const FName& TaskName) { Tasks.Add(TaskName); }
};

USTRUCT(BlueprintType)
struct SHTNPLANNERRUNTIME_API FSHTNCompositeTask
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FSHTNMethod> Methods;

	int32 FindSatisfiedMethod(const FSHTNWorldState& WorldState, const int32 StartIndex = 0) const;

	FSHTNMethod& AddMethod()
	{
		return Methods[Methods.Add(FSHTNMethod())];
	}

	FSHTNMethod& AddMethod(const FSHTNCondition& Condition)
	{
		return Methods[Methods.Add(FSHTNMethod(Condition))];
	}

	FSHTNMethod& AddMethod(const TArray<FSHTNCondition>& Conditions)
	{
		return Methods[Methods.Add(FSHTNMethod(Conditions))];
	}
};

USTRUCT(BlueprintType)
struct SHTNPLANNERRUNTIME_API FSHTNDomain
{
	GENERATED_BODY()

	FSHTNDomain();
	~FSHTNDomain();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RootTaskName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, FSHTNCompositeTask> CompositeTasks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, FSHTNPrimitiveTask> PrimitiveTasks;
	

	FSHTNPrimitiveTask& AddPrimitiveTask(const FName& TaskName)
	{
		bIsValid = false;
		return PrimitiveTasks.FindOrAdd(TaskName);
	}

	FSHTNCompositeTask& AddCompositeTask(const FName& TaskName)
	{
		bIsValid = false;
		return CompositeTasks.FindOrAdd(TaskName);
	}

	const FSHTNPrimitiveTask& GetPrimitiveTask(const FName& TaskName) const
	{
		return *PrimitiveTasks.Find(TaskName);
	}

	const FSHTNCompositeTask& GetCompositeTask(const FName& TaskName) const
	{
		return *CompositeTasks.Find(TaskName);
	}

	bool IsPrimitiveTask(const FName& TaskName) const
	{
		return PrimitiveTasks.Find(TaskName) != nullptr;
	}

	bool IsCompositeTask(const FName& TaskName) const
	{
		return CompositeTasks.Find(TaskName) != nullptr;
	}

	bool Validate(bool bDebug = true);

	bool IsValid() { return bIsValid; }

	void SetRootName(const FName& InRootName) { RootTaskName = InRootName; }
	
protected:

	bool bIsValid;
};

struct SHTNPLANNERRUNTIME_API FSHTNWorldState
{
	FSHTNWorldState();
	~FSHTNWorldState();

	void Init(const uint32 NewWorldStateSize = 64);

	bool CheckCondition(const FSHTNCondition& Condition) const;
	FORCEINLINE bool CheckConditions(const TArray<FSHTNCondition>& Conditions) const
	{
		for (int32 ConditionIndex = 0; ConditionIndex < Conditions.Num(); ++ConditionIndex)
		{
			if (CheckCondition(Conditions[ConditionIndex]) == false)
			{
				return false;
			}
		}
		return true;
	}

	void ApplyEffect(const FSHTNEffect& Effect);
	FORCEINLINE void ApplyEffects(const TArray<FSHTNEffect>& Effects)
	{
		for (int32 EffectIndex = 0; EffectIndex < Effects.Num(); ++EffectIndex)
		{
			ApplyEffect(Effects[EffectIndex]);
		}
	}

	bool GetValue(const FSHTNDefs::FWSKey Key, FSHTNDefs::FWSValue& OutValue) const;
	FSHTNDefs::FWSValue GetValueUnsafe(const FSHTNDefs::FWSKey Key) const
	{
		return Values[Key];
	}

	bool SetValue(const FSHTNDefs::FWSKey Key, const FSHTNDefs::FWSValue InValue);
	void SetValueUnsafe(const FSHTNDefs::FWSKey Key, const FSHTNDefs::FWSValue InValue)
	{
		Values[Key] = InValue;
	}

	void Shrink() { Values.Shrink(); }

protected:
	TArray<FSHTNDefs::FWSValue> Values;
};