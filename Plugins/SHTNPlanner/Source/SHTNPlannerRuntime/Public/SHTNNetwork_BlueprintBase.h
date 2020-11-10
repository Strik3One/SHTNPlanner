// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SHTNDomain.h"
#include "SHTNNetwork_BlueprintBase.generated.h"

// This class makes it easy for us to make a detail customisation so we can make a dropdown to select the Key from
USTRUCT(BlueprintType)
struct FUserDefinedWorldState
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, Category = "SHTN")
		uint8 KeyValue;

	bool operator==(const FUserDefinedWorldState& Rhs) const
	{
		return KeyValue == Rhs.KeyValue;
	}

	bool operator==(const uint8& Rhs) const
	{
		return KeyValue == Rhs;
	}
};

USTRUCT(BlueprintType)
struct FWorldStateArrayElement
{
	GENERATED_BODY()

		UPROPERTY(BlueprintReadWrite, Category = "SHTN")
		uint8 WorldStateKey;

	UPROPERTY(BlueprintReadWrite, Category = "SHTN")
		int32 Value;

	FWorldStateArrayElement()
		: WorldStateKey(MAX_uint8), Value(0)
	{}
};

USTRUCT(BlueprintType)
struct FWorldStateElement
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "SHTN")
	FUserDefinedWorldState WorldStateKey;

	UPROPERTY(EditAnywhere, Category = "SHTN")
	int32 Value;

	FWorldStateElement(uint8 InKey = MAX_uint8, int32 InValue = 0)
		: Value(InValue)
	{
		WorldStateKey.KeyValue = InKey;
	}

	FWorldStateElement(const FWorldStateArrayElement& InElement)
		: FWorldStateElement(InElement.WorldStateKey, InElement.Value)
	{
	}

	bool operator==(const uint8& Rhs) const
	{
		return WorldStateKey == Rhs;
	}
};

// This class makes it easy for us to make a detail customisation so we can make a dropdown to select the Key from
USTRUCT(BlueprintType)
struct FUserDefinedOperator
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, Category = "SHTN")
		uint8 KeyValue;

	bool operator==(const FUserDefinedOperator& Rhs) const
	{
		return KeyValue == Rhs.KeyValue;
	}

	bool operator==(const uint8& Rhs) const
	{
		return KeyValue == Rhs;
	}
};

USTRUCT(BlueprintType)
struct FSHTNOperatorClassData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "SHTN")
	FUserDefinedOperator OperatorKey;

	UPROPERTY(EditAnywhere, Category = "SHTN")
	TSubclassOf<class USHTNOperator_BlueprintBase> Class;
};

UCLASS(Blueprintable)
class SHTNPLANNERRUNTIME_API USHTNNetwork_BlueprintBase : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "SHTN")
	TArray<FWorldStateElement> DefaultWorldState;

	UPROPERTY(EditDefaultsOnly, Category = "SHTN")
		UEnum* WorldStateEnumAsset;
	
	UPROPERTY(EditDefaultsOnly, Category = "SHTN")
		UEnum* OperatorEnumAsset;

	UPROPERTY(EditDefaultsOnly, Category = "SHTN")
		TArray<FSHTNOperatorClassData> OperatorClasses;

	bool BuildNetwork(class USHTNComponent* HTNComponent);

protected:

	UFUNCTION(BlueprintNativeEvent)
	bool ProceduralDefaultWorldState(class AAIController* AIOwner, class APawn* Pawn, TArray<FWorldStateArrayElement>& ProceduralWorldState);

	UFUNCTION(BlueprintNativeEvent)
	bool BuildHTNDomain(FSHTNDomain& Domain);

};
