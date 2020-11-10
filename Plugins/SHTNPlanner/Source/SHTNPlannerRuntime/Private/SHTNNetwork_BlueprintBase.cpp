// Fill out your copyright notice in the Description page of Project Settings.

#include "SHTNNetwork_BlueprintBase.h"
#include "SHTNComponent.h"
#include "AIController.h"

bool USHTNNetwork_BlueprintBase::ProceduralDefaultWorldState_Implementation(class AAIController* AIOwner, class APawn* Pawn, TArray<FWorldStateArrayElement>& ProceduralWorldState)
{
	return true;
}

bool USHTNNetwork_BlueprintBase::BuildHTNDomain_Implementation(FSHTNDomain & Domain)
{
	return false;
}

bool USHTNNetwork_BlueprintBase::BuildNetwork(USHTNComponent * HTNComponent)
{
	// Get the procedural WorldState values defined by the user in the Network
	TArray<FWorldStateArrayElement> ProceduralWorldState;
	ProceduralDefaultWorldState(HTNComponent->GetAIOwner(), HTNComponent->GetAIOwner()->GetPawn(), ProceduralWorldState);

	//Go through the procedural world state elements and add them to the default worldstate in case the key doesn't exist yet
	//Else just change the value of the key
	for (const FWorldStateArrayElement& Element : ProceduralWorldState)
	{
		// This value is default assigned to an element meaning that if we accidentally add an empty value to the array - we don't want to do anything with it
		if (Element.WorldStateKey == MAX_uint8)
			continue;

		int32 Index = DefaultWorldState.IndexOfByKey(Element.WorldStateKey);

		if (Index == INDEX_NONE)
		{
			DefaultWorldState.Add(FWorldStateElement(Element));
		}
		else
		{
			DefaultWorldState[Index].Value = Element.Value;
		}
	}

	// Set the default worldstate values in the domains world state
	for (const auto& Element : DefaultWorldState)
	{
		HTNComponent->WorldState.SetValueUnsafe((FSHTNDefs::FWSKey)Element.WorldStateKey.KeyValue, Element.Value);
	}

	// Build the actual domain
	return BuildHTNDomain(HTNComponent->Domain);
}
