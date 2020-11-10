// Fill out your copyright notice in the Description page of Project Settings.

#include "SHTNControllerLibrary.h"

#include "AIController.h"
#include "SHTNNetwork_BlueprintBase.h"
#include "SHTNOperator_BlueprintBase.h"
#include "SHTNDomain.h"
#include "SHTNComponent.h"



bool USHTNControllerLibrary::RunHTNPlanner(AAIController* AIController, TSubclassOf<USHTNNetwork_BlueprintBase> HTNNetwork)
{
	// Do some NULL checking
	if (AIController == NULL)
	{
		UE_LOG(SHTNPlannerRuntime, Error, TEXT("RunHTNPlanner: Can't run the planner on a NULL AIController"));
		return false;
	}
	else if (HTNNetwork.GetDefaultObject() == NULL)
	{
		UE_LOG(SHTNPlannerRuntime, Error, TEXT("RunHTNPlanner: Can't run the planner on a NULL Network"));
		return false;
	}

	// See if there is already a SHTNComponent as BrainComponent 
	// If not, create a new one and assign it
	USHTNComponent* SHTNComp = Cast<USHTNComponent>(AIController->BrainComponent);
	if (SHTNComp == NULL)
	{
		UE_LOG(SHTNPlannerRuntime, Log, TEXT("RunHTNPlanner: Spawning new SHTNComponent..."));

		SHTNComp = NewObject<USHTNComponent>(AIController, TEXT("SHTNComponent"));
		SHTNComp->RegisterComponent();
	}

	AIController->BrainComponent = SHTNComp;

	// Make sure the object was created succesfully
	check(SHTNComp != NULL);
	   
	return USHTNControllerLibrary::SetupSHTNComponent(SHTNComp, HTNNetwork.GetDefaultObject());
}

void USHTNControllerLibrary::SetWorldStateAsValue(AActor * Target, uint8 WorldStateKey, int32 Value)
{
	USHTNComponent* SHTNComp = GetSHTNComponent(Target);

	if (SHTNComp == nullptr)
	{
		return;
	}

	if (SHTNComp->WorldState.SetValue(WorldStateKey, Value))
	{
		SHTNComp->bReplan = true;
	}
}

void USHTNControllerLibrary::SetWorldStateAsKeyValue(AActor * Target, uint8 WorldStateKey, uint8 KeyValue)
{
	USHTNComponent* SHTNComp = GetSHTNComponent(Target);

	if (SHTNComp == nullptr)
	{
		return;
	}

	int32 Value;
	if (!SHTNComp->WorldState.GetValue(KeyValue, Value))
	{
		return;
	}
	
	if (SHTNComp->WorldState.SetValue(WorldStateKey, Value))
	{
		SHTNComp->bReplan = true;
	}
}

void USHTNControllerLibrary::GetWorldStateValue(AActor * Target, uint8 WorldStateKey, int32 & Value, bool & bSuccess)
{
	USHTNComponent* SHTNComp = GetSHTNComponent(Target);

	if (SHTNComp == nullptr)
	{
		bSuccess = false;
		return;
	}

	bSuccess = SHTNComp->WorldState.GetValue(WorldStateKey, Value);
}

bool USHTNControllerLibrary::SetupSHTNComponent(USHTNComponent * SHTNComp, USHTNNetwork_BlueprintBase * HTNNetwork)
{
	for (const auto& OperatorClass : HTNNetwork->OperatorClasses)
	{
		SHTNComp->SHTNOperators.Add(OperatorClass.OperatorKey.KeyValue, NewObject<USHTNOperator_BlueprintBase>(SHTNComp, OperatorClass.Class));
	}

	if (!HTNNetwork->BuildNetwork(SHTNComp))
	{
		return false;
	}
	else
	{
		return SHTNComp->Domain.Validate();
	}
}

USHTNComponent* USHTNControllerLibrary::GetSHTNComponent(AActor * Actor)
{
	ensure(Actor != nullptr);

	AAIController* Controller = Cast<AAIController>(Actor);

	if (Controller == nullptr)
	{
		APawn* Pawn = Cast<APawn>(Actor);
		if (Pawn != nullptr)
		{
			Controller = Cast<AAIController>(Pawn->GetController());
		}
	}	

	if (Controller == nullptr)
	{
		UE_LOG(SHTNPlannerRuntime, Error, TEXT("%s is not an actor that has an AIController"), *Actor->GetActorLabel());
		return nullptr;
	}

	USHTNComponent* SHTNComp = Cast<USHTNComponent>(Controller->GetBrainComponent());

	if (SHTNComp == nullptr)
	{
		UE_LOG(SHTNPlannerRuntime, Error, TEXT("%s is not a controller with an SHTNComponent as BrainComponent"), *Controller->GetActorLabel());
		return nullptr;
	}

	return SHTNComp;
}
