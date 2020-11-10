// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SHTNDomain.h"
#include "SHTNOperator_BlueprintBase.generated.h"

class AAIController;

USTRUCT(BlueprintType)
struct FSHTNWorldStateEffect
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 WSKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESHTNWorldStateOperation Operation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Value;
};

UENUM()
enum class ESHTNOperatorResult : uint8
{
	InProgress,
	Success,
	Failed,
	Aborted
};


UCLASS(Blueprintable, Abstract)
class SHTNPLANNERRUNTIME_API USHTNOperator_BlueprintBase : public UObject
{
	GENERATED_BODY()
	
protected:

	bool bActivated;
	ESHTNOperatorResult Result;

	UFUNCTION(BlueprintImplementableEvent, Category = "SHTN")
		void ReceiveInitializeAction(AAIController* OwnerController, const uint8 OperatorParam);

	UFUNCTION(BlueprintImplementableEvent, Category = "SHTN")
		void ReceiveExecuteAction(AAIController* OwnerController, const uint8 OperatorParam, const float DeltaTime);

	UFUNCTION(BlueprintImplementableEvent, Category = "SHTN")
		void ReceiveAbort(AAIController* OwnerController, const uint8 OperatorParam);

	UFUNCTION(BlueprintImplementableEvent, Category = "SHTN")
		void ReceiveGetActualEffects(TArray<FSHTNWorldStateEffect>& Effects, AAIController* OwnerController);

	UFUNCTION(BlueprintCallable, Category = "SHTN")
		void FinishExecute(bool bSuccess) { Result = bSuccess ? ESHTNOperatorResult::Success : ESHTNOperatorResult::Failed; }

	UFUNCTION(BlueprintCallable, Category = "SHTN")
		void FinishAbort() { Result = ESHTNOperatorResult::Aborted; }

public:

	USHTNOperator_BlueprintBase();

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
		Result = ESHTNOperatorResult::Aborted;
		ReceiveAbort(OwnerController, OperatorParam);
	}

	void GetActualEffects(TArray<FSHTNWorldStateEffect>& Effects, AAIController* OwnerController)
	{
		ReceiveGetActualEffects(Effects, OwnerController);
	}

	void Terminate() { bActivated = false; }
	ESHTNOperatorResult GetResult() { return Result; }
	bool IsActivated() const { return bActivated; }
};
