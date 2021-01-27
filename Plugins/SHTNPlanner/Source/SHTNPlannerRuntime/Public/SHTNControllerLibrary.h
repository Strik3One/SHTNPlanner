// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SHTNDomain.h"
//#include "SHTNNetwork_BlueprintBase.h"
#include "SHTNControllerLibrary.generated.h"


UCLASS()
class SHTNPLANNERRUNTIME_API USHTNControllerLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "SHTN")
	static bool RunHTNPlanner(class AAIController* AIController,  TSubclassOf<class USHTNNetwork_BlueprintBase> HTNNetwork);

protected:

	static bool SetupSHTNComponent(class USHTNComponent* SHTNComp, class USHTNNetwork_BlueprintBase* HTNNetwork);

	static USHTNComponent* GetSHTNComponent(AActor* Actor);

};
