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

	UFUNCTION(BlueprintCallable, Category = "SHTN")
	static void SetWorldStateAsValue(AActor* Target, uint8 WorldStateKey, int32 Value);

	UFUNCTION(BlueprintCallable, Category = "SHTN")
	static void SetWorldStateAsKeyValue(AActor* Target, uint8 WorldStateKey, uint8 Key);

	UFUNCTION(BlueprintCallable, Category = "SHTN")
	static void ChangeWorldStateByValue(AActor* Target, ESHTNWorldStateOperation Operation, uint8 WorldStateKey, int32 Value);

	UFUNCTION(BlueprintCallable, Category = "SHTN")
	static void ChangeWorldStateByKeyValue(AActor* Target, ESHTNWorldStateOperation Operation, uint8 WorldStateKey, uint8 Key);

	UFUNCTION(BlueprintCallable, Category = "SHTN")
	static void GetWorldStateValue(AActor* Target, uint8 WorldStateKey, int32& Value, bool& bSuccess);

protected:

	static bool SetupSHTNComponent(class USHTNComponent* SHTNComp, class USHTNNetwork_BlueprintBase* HTNNetwork);

	static USHTNComponent* GetSHTNComponent(AActor* Actor);

};
