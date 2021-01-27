// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SHTNDomain.h"
#include "Engine/World.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "SHTNOperator_BlueprintBase.generated.h"


class AAIController;

UENUM()
enum class ESHTNOperatorResult : uint8
{
	InProgress,
	Success,
	Failed,
	Aborted
};

UCLASS()
class SHTNPLANNERRUNTIME_API USHTNBlackboardGetWrapper : public UObject
{

	GENERATED_BODY()

public:

	USHTNBlackboardGetWrapper() :
		Blackboard(nullptr)
	{}

	UFUNCTION(BlueprintCallable, Category = "AI|Components|Blackboard")
		UObject* GetValueAsObject(const FName KeyName) const
	{
		return Blackboard->GetValueAsObject(KeyName);
	}

	UFUNCTION(BlueprintCallable, Category = "AI|Components|Blackboard")
		UClass* GetValueAsClass(const FName KeyName) const
	{
		return Blackboard->GetValueAsClass(KeyName);
	}

	UFUNCTION(BlueprintCallable, Category = "AI|Components|Blackboard")
		uint8 GetValueAsEnum(const FName KeyName) const
	{
		return Blackboard->GetValueAsEnum(KeyName);
	}

	UFUNCTION(BlueprintCallable, Category = "AI|Components|Blackboard")
		int32 GetValueAsInt(const FName KeyName) const
	{
		return Blackboard->GetValueAsInt(KeyName);
	}

	UFUNCTION(BlueprintCallable, Category = "AI|Components|Blackboard")
		float GetValueAsFloat(const FName KeyName) const
	{
		return Blackboard->GetValueAsFloat(KeyName);
	}

	UFUNCTION(BlueprintCallable, Category = "AI|Components|Blackboard")
		bool GetValueAsBool(const FName KeyName) const
	{
		return Blackboard->GetValueAsBool(KeyName);
	}

	UFUNCTION(BlueprintCallable, Category = "AI|Components|Blackboard")
		FString GetValueAsString(const FName KeyName) const
	{
		return Blackboard->GetValueAsString(KeyName);
	}

	UFUNCTION(BlueprintCallable, Category = "AI|Components|Blackboard")
		FName GetValueAsName(const FName KeyName) const
	{
		return Blackboard->GetValueAsName(KeyName);
	}

	UFUNCTION(BlueprintCallable, Category = "AI|Components|Blackboard")
		FVector GetValueAsVector(const FName KeyName) const
	{
		return Blackboard->GetValueAsVector(KeyName);
	}

	UFUNCTION(BlueprintCallable, Category = "AI|Components|Blackboard")
		FRotator GetValueAsRotator(const FName KeyName) const
	{
		return Blackboard->GetValueAsRotator(KeyName);
	}

	UFUNCTION(BlueprintCallable, Category = "AI|Components|Blackboard")
		bool GetLocationFromEntry(const FName KeyName, FVector& ResultLocation) const
	{
		return Blackboard->GetLocationFromEntry(KeyName, ResultLocation);
	}

protected:
	
	friend class USHTNOperator_BlueprintBase;

	UBlackboardComponent* Blackboard;

};



UCLASS(Blueprintable, Abstract)
class SHTNPLANNERRUNTIME_API USHTNOperator_BlueprintBase : public UObject
{
	GENERATED_BODY()
	
protected:

	bool bActivated;
	ESHTNOperatorResult Result;

	UFUNCTION(BlueprintNativeEvent, Category = "SHTN")
		bool CheckConditions(USHTNBlackboardGetWrapper* WorldState, const uint8 OperatorParam, const bool bIsPlanning);

	UFUNCTION(BlueprintNativeEvent, Category = "SHTN")
		float GetScore(USHTNBlackboardGetWrapper* WorldState, const uint8 OperatorParam);

	UFUNCTION(BlueprintImplementableEvent, Category = "SHTN")
		void ReceiveInitializeAction(AAIController* OwnerController, const uint8 OperatorParam);

	UFUNCTION(BlueprintImplementableEvent, Category = "SHTN")
		void ReceiveExecuteAction(AAIController* OwnerController, const uint8 OperatorParam, const float DeltaTime);

	UFUNCTION(BlueprintNativeEvent, Category = "SHTN")
		void ReceiveAbort(AAIController* OwnerController, const uint8 OperatorParam);

	UFUNCTION(BlueprintCallable, Category = "SHTN")
		void FinishExecute(bool bSuccess) { Result = bSuccess ? ESHTNOperatorResult::Success : ESHTNOperatorResult::Failed; }

	UFUNCTION(BlueprintCallable, Category = "SHTN")
		void FinishAbort() { Result = ESHTNOperatorResult::Aborted; }

	UWorld* World;

	UPROPERTY()
	USHTNBlackboardGetWrapper* SetProtectedWorldState;

public:

	USHTNOperator_BlueprintBase();

	void Init(UWorld* InWorld, FName& ElementCDOName);

	void SetWorld(UWorld* InWorld) { World = InWorld; }

	virtual UWorld* GetWorld() const override;

	 float GetScores(UBlackboardComponent* Blackboard, const uint8 OperatorParam)
	{
		SetProtectedWorldState->Blackboard = Blackboard;
		return GetScore(SetProtectedWorldState, OperatorParam);
	}

	bool CheckCondition(UBlackboardComponent* Blackboard, const uint8 OperatorParam, const bool bIsPlanning = false)
	{
		SetProtectedWorldState->Blackboard = Blackboard;
		return CheckConditions(SetProtectedWorldState, OperatorParam, bIsPlanning);
	}

	UFUNCTION(BlueprintImplementableEvent, Category = "SHTN")
	void ApplyEffects(UBlackboardComponent* WorldState, const uint8 OperatorParam, const bool IsPlanning);

	void Initialize(AAIController* OwnerController, const uint8 OperatorParam)
	{
		bActivated = true;
		Result = ESHTNOperatorResult::InProgress;
		ReceiveInitializeAction(OwnerController, OperatorParam);
	}

	void Execute(AAIController* OwnerController, const uint8 OperatorParam, const float DeltaTime)
	{
		Result = ESHTNOperatorResult::InProgress;
		ReceiveExecuteAction(OwnerController, OperatorParam, DeltaTime);
	}

	void Abort(AAIController* OwnerController, const uint8 OperatorParam)
	{
		Terminate();
		ReceiveAbort(OwnerController, OperatorParam);
	}

	void Terminate() { bActivated = false; }
	ESHTNOperatorResult GetResult() { return Result; }
	bool IsActivated() const { return bActivated; }

	FName CDOName;
};
