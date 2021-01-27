// Fill out your copyright notice in the Description page of Project Settings.

#include "SHTNControllerLibrary.h"

#include "AIController.h"
#include "SHTNNetwork_BlueprintBase.h"
#include "SHTNOperator_BlueprintBase.h"
#include "SHTNComponent.h"

#include "BehaviorTree/BlackboardComponent.h"



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
	else if (HTNNetwork.GetDefaultObject()->WorldStateAsset == NULL)
	{
		UE_LOG(SHTNPlannerRuntime, Error, TEXT("RunHTNPlanner: Can't run the planner with a NULL WorldStateAsset"));
		return false;
	}

	bool bSuccess = true;

	// see if need a blackboard component at all
	UBlackboardComponent* WorldStateComp = Cast<UWorldStateComponent>(AIController->GetBlackboardComponent());
	USHTNNetwork_BlueprintBase* HTNAsset = HTNNetwork.GetDefaultObject();

	if (HTNAsset->WorldStateAsset && (WorldStateComp == nullptr || WorldStateComp->IsCompatibleWith(HTNAsset->WorldStateAsset) == false))
	{
		// Make sure we have our extended blackboard class as the blackboard component so we can access the valuememory during planning
		if (WorldStateComp == nullptr)
		{
			WorldStateComp = NewObject<UWorldStateComponent>(AIController, TEXT("WorldStateComponent"));
			WorldStateComp->RegisterComponent();
		}
		
		bSuccess = AIController->UseBlackboard(HTNAsset->WorldStateAsset, WorldStateComp);
	}

	if (bSuccess)
	{
		// See if there is already a SHTNComponent as BrainComponent 
		// If not, create a new one and assign it
		USHTNComponent* SHTNComp = Cast<USHTNComponent>(AIController->BrainComponent);
		if (SHTNComp == NULL)
		{
			UE_LOG(SHTNPlannerRuntime, Log, TEXT("RunHTNPlanner: Spawning new SHTNComponent..."));

			SHTNComp = NewObject<USHTNComponent>(AIController, TEXT("SHTNComponent"));

			SHTNComp->RegisterComponent();

			SHTNComp->OwningPawnName = AIController->GetPawn()->GetFName();

			SHTNComp->BlackboardState = Cast<UWorldStateComponent>(WorldStateComp);

			SHTNComp->WorkingWorldState = NewObject<UWorldStateComponent>(SHTNComp, TEXT("WorkingWorldState"));
			SHTNComp->WorkingWorldState->AddToRoot();

			// Dupe a new asset and disable the instance syncing so when an instance synced value is updated during planning it doesn't sync with the active blackboards.
			UBlackboardData* BBData = DuplicateObject<UBlackboardData>(WorldStateComp->GetBlackboardAsset(), SHTNComp->WorkingWorldState);
			
			for (auto& Key : BBData->Keys)
			{
				Key.bInstanceSynced = false;
			}

			SHTNComp->WorkingWorldState->InitializeBlackboard(*BBData);
			SHTNComp->WorkingWorldState->PauseObserverNotifications();
		}

		// Make sure the object was created succesfully
		check(SHTNComp != NULL);

		AIController->BrainComponent = SHTNComp;

		if (USHTNControllerLibrary::SetupSHTNComponent(SHTNComp, HTNAsset))
		{
			return true;
		}
		else
		{
			SHTNComp->UnregisterComponent();
			AIController->BrainComponent = nullptr;
			return false;
		}
	}

	return bSuccess;
}

bool USHTNControllerLibrary::SetupSHTNComponent(USHTNComponent * SHTNComp, USHTNNetwork_BlueprintBase * HTNNetwork)
{
	if (!HTNNetwork->BuildNetwork(SHTNComp))
	{
		return false;
	}
	else
	{
		SHTNComp->SHTNOperators.Empty();

		for (auto& Elem : SHTNComp->Domain.PrimitiveTasks)
		{
			//First check if this Action already has an object, if so - only set the index.
			// If not,then create the now object and set the index.
			FName ElementCDOName = Elem.Value.Action.GetDefaultObject()->GetFName();
			int32 Index = SHTNComp->SHTNOperators.IndexOfByPredicate([ElementCDOName](const USHTNOperator_BlueprintBase* Object) {return ElementCDOName == Object->CDOName; });

			if (Index >= 0)
			{
				Elem.Value.ActionPtr = SHTNComp->SHTNOperators[Index];
			}
			else
			{
				Elem.Value.ActionPtr = SHTNComp->SHTNOperators.Add_GetRef(NewObject<USHTNOperator_BlueprintBase>(SHTNComp, Elem.Value.Action));
				Elem.Value.ActionPtr->Init(SHTNComp->GetWorld(), ElementCDOName);
			}
		}

		const int32 NumKeys = SHTNComp->BlackboardState->GetNumKeys();

		for (int32 i = 0; i < NumKeys; ++i)
		{
			if (HTNNetwork->IgnoredWorldStateValues.Contains(SHTNComp->BlackboardState->GetKeyName(i)) == false)
			{
				SHTNComp->BlackboardState->RegisterObserver(i, SHTNComp, FOnBlackboardChangeNotification::CreateUObject(SHTNComp, &USHTNComponent::OnBlackboardKeyValueChange));
			}
		}

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
