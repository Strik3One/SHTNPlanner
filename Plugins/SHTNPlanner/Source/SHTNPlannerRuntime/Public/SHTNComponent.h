// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BrainComponent.h"
#include "SHTNDomain.h"
#include "SHTNPlanner.h"
#include "SHTNOperator_BlueprintBase.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "SHTNComponent.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnNewPlanMade)

class UBlackboardComponent;
/**
 * 
 */
UCLASS()
class SHTNPLANNERRUNTIME_API USHTNComponent : public UBrainComponent
{
	GENERATED_BODY()
	
public:

	USHTNComponent();

	//FSHTNWorldState WorldState;

	UWorldStateComponent* BlackboardState;
	UWorldStateComponent* WorkingWorldState;

	FSHTNDomain Domain;

	// Debug Variables
	FName OwningPawnName;
	//UEnum* WorldStateEnumAsset;

	UFUNCTION(BlueprintCallable, Category = "SHTN")
		void ForceReplan() { bReplan = true; }

	/** Starts brain logic. If brain is already running, will not do anything. */
	virtual void StartLogic() override;

	/** Restarts currently running or previously ran brain logic. */
	virtual void RestartLogic() override;

	/** Stops currently running brain logic. */
	virtual void StopLogic(const FString& Reason) override;

	/** AI logic won't be needed anymore, stop all activity and run cleanup */
	virtual void Cleanup() override;

	/** Pause logic and blackboard updates. */
	virtual void PauseLogic(const FString& Reason) override;

	/** Resumes paused brain logic. */
	virtual EAILogicResuming::Type ResumeLogic(const FString& Reason) override;

	virtual bool IsRunning() const override;
	virtual bool IsPaused() const override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	EBlackboardNotificationResult OnBlackboardKeyValueChange(const UBlackboardComponent& Blackboard, FBlackboard::FKey ChangedKeyID);

	FOnNewPlanMade OnNewPlanMade;

protected:
	
	FSHTNPlan Plan;
	TArray<FName> DebugTaskNames;
	
	FSHTNPlanner Planner;

	UPROPERTY()
	TArray<USHTNOperator_BlueprintBase*> SHTNOperators; 

	USHTNOperator_BlueprintBase* CurrentOperator;
	FSHTNPrimitiveTask CurrentTask;
	FName CurrentTaskName;

	bool bReplan;

	bool bIsPaused;
	bool bIsRunning;
	bool bCurrentExecutionSuccessful;
	
	void GeneratePlan();

public:

	friend class USHTNControllerLibrary;
	friend class FWorldStateDebugDetails;

	//Called every frame
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

};
