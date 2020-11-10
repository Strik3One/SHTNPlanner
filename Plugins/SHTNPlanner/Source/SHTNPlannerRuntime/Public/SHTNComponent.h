// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BrainComponent.h"
#include "SHTNDomain.h"
#include "SHTNPlanner.h"
#include "SHTNOperator_BlueprintBase.h"
#include "SHTNComponent.generated.h"

/**
 * 
 */
UCLASS()
class SHTNPLANNERRUNTIME_API USHTNComponent : public UBrainComponent
{
	GENERATED_BODY()
	
public:

	USHTNComponent();

	FSHTNWorldState WorldState;
	FSHTNDomain Domain;

protected:
	
	FSHTNPlan Plan;
	
	FSHTNPlanner Planner;

	// Make this a UPROPERTY so the elements aren't garbage collected
	UPROPERTY()
	TMap<uint8, USHTNOperator_BlueprintBase*> SHTNOperators;

	USHTNOperator_BlueprintBase* CurrentOperator;
	FSHTNPrimitiveTask CurrentTask;
	bool bReplan;
	
	void GeneratePlan();

public:

	friend class USHTNControllerLibrary;

	//Called every frame
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

};
