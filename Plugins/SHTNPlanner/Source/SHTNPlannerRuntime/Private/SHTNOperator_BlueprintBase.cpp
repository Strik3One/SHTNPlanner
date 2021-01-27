// Fill out your copyright notice in the Description page of Project Settings.


#include "SHTNOperator_BlueprintBase.h"

USHTNOperator_BlueprintBase::USHTNOperator_BlueprintBase()
{
	bActivated = false;
}

void USHTNOperator_BlueprintBase::Init(UWorld * InWorld, FName & ElementCDOName)
{
	SetWorld(InWorld);
	CDOName = ElementCDOName;
	SetProtectedWorldState = NewObject<USHTNBlackboardGetWrapper>(this, TEXT("SetProtectedWorldState"));
}

void USHTNOperator_BlueprintBase::ReceiveAbort_Implementation(AAIController* OwnerController, const uint8 OperatorParam)
{
	Result = ESHTNOperatorResult::Aborted;
}

UWorld * USHTNOperator_BlueprintBase::GetWorld() const
{
	return World;
}

bool USHTNOperator_BlueprintBase::CheckConditions_Implementation(USHTNBlackboardGetWrapper * WorldState, const uint8 OperatorParam, const bool bIsPlanning)
{
	return true;
}

float USHTNOperator_BlueprintBase::GetScore_Implementation(USHTNBlackboardGetWrapper* WorldState, const uint8 OperatorParam)
{
	// If not implemented assume score is 0
	return 0.f;
}