// Fill out your copyright notice in the Description page of Project Settings.

#include "SHTNNetwork_BlueprintBase.h"
#include "SHTNComponent.h"
#include "AIController.h"

USHTNNetwork_BlueprintBase::USHTNNetwork_BlueprintBase()
{
	MaxPlanCycles = 250;
}


bool USHTNNetwork_BlueprintBase::SetDefaultWorldState_Implementation(class AAIController* AIOwner, class APawn* Pawn, UBlackboardComponent* WorldState)
{
	return true;
}

bool USHTNNetwork_BlueprintBase::BuildHTNDomain_Implementation(FSHTNDomain & Domain)
{
	return false;
}

bool USHTNNetwork_BlueprintBase::BuildNetwork(USHTNComponent * HTNComponent)
{
	// Set the procedural WorldState values defined by the user in the Network
	SetDefaultWorldState(HTNComponent->GetAIOwner(), HTNComponent->GetAIOwner()->GetPawn(), HTNComponent->GetAIOwner()->GetBlackboardComponent());

	// Build the actual domain
	if (!BuildHTNDomain(HTNComponent->Domain))
	{
		UE_LOG(SHTNPlannerRuntime, Error, TEXT("Building of Domain in network %s returned false"), *GetName());
		return false;
	}

	HTNComponent->Domain.MaxPlanCycles = MaxPlanCycles;

	return true;
}
