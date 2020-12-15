#include "CoreMinimal.h"
#include "SHTNDomain.h"
#include "SHTNPlanner.h"
#include "AITestsCommon.h"

enum class ETestSHTNWorldState : uint8
{
	EnemyHealth,
	EnemyActor,
	Ammo,
	AbilityRange,
	HasWeapon,
	MoveDestination,
	PickupLocation,
	CurrentLocation,
	CanSeeEnemy,

	MAX
};

enum class ETestSHTNTaskOperator : uint8
{
	DummyOperation,
	FindPatrolPoint,
	FindWeapon,
	NavigateTo,
	PickUp,
	UseWeapon,

	MAX
};

struct FSHTNTestBase : public FAITestBase
{
	FSHTNDomain Domain;
	FSHTNPlanner Planner;
	FSHTNWorldState WorldState;

	void PopulateWorldState()
	{
		WorldState.Init(int32(ETestSHTNWorldState::MAX));

		// Assign every key to their numerical index value - we assume this later on in the tests
		for (int32 Index = 0; Index < int32(ETestSHTNWorldState::MAX); ++Index)
		{
			WorldState.SetValueUnsafe(Index, Index);
		}
	}

	void PopulateDomain()
	{
		Domain.SetRootName(TEXT("Root"));
		{
			FSHTNCompositeTask& CompositeTask = Domain.AddCompositeTask(TEXT("Root"));
			{
				FSHTNMethod& Methods = CompositeTask.AddMethod(
					TArray<FSHTNCondition>({
					FSHTNCondition(ETestSHTNWorldState::EnemyHealth, ESHTNWorldStateCheck::Greater).SetRHSAsValue(0)
					, FSHTNCondition(ETestSHTNWorldState::EnemyActor, ESHTNWorldStateCheck::IsTrue)
						}));
				Methods.AddTask(TEXT("AttackEnemy"));
			}
			{
				FSHTNMethod& Methods = CompositeTask.AddMethod();
				Methods.AddTask(TEXT("FindPatrolPoint"));
				Methods.AddTask(TEXT("NavigateToMoveDestination"));
			}
		}
		{
			FSHTNCompositeTask& CompositeTask = Domain.AddCompositeTask(TEXT("AttackEnemy"));
			{
				FSHTNMethod& Methods = CompositeTask.AddMethod(FSHTNCondition(ETestSHTNWorldState::HasWeapon, ESHTNWorldStateCheck::IsTrue));
				Methods.AddTask(TEXT("NavigateToEnemy"));
				Methods.AddTask(TEXT("UseWeapon"));
				Methods.AddTask(TEXT("Root"));
			}
			{
				FSHTNMethod& Methods = CompositeTask.AddMethod();
				Methods.AddTask(TEXT("FindWeapon"));
				Methods.AddTask(TEXT("NavigateToWeapon"));
				Methods.AddTask(TEXT("PickUp"));
				Methods.AddTask(TEXT("AttackEnemy"));
			}
		}
		{
			FSHTNPrimitiveTask& PrimitiveTask = Domain.AddPrimitiveTask(TEXT("FindPatrolPoint"));
			PrimitiveTask.SetOperator(ETestSHTNTaskOperator::FindPatrolPoint, ETestSHTNWorldState::MoveDestination);
		}
		{
			FSHTNPrimitiveTask& PrimitiveTask = Domain.AddPrimitiveTask(TEXT("FindWeapon"));
			PrimitiveTask.SetOperator(ETestSHTNTaskOperator::FindWeapon, ETestSHTNWorldState::PickupLocation);
		}
		{
			FSHTNPrimitiveTask& PrimitiveTask = Domain.AddPrimitiveTask(TEXT("NavigateToMoveDestination"));
			PrimitiveTask.SetOperator(ETestSHTNTaskOperator::NavigateTo, ETestSHTNWorldState::MoveDestination);	// Local Variables?
			PrimitiveTask.AddEffect(FSHTNEffect(ETestSHTNWorldState::CurrentLocation, ESHTNWorldStateOperation::Set).SetRHSAsWSKey(ETestSHTNWorldState::MoveDestination));
		}
		{
			FSHTNPrimitiveTask& PrimitiveTask = Domain.AddPrimitiveTask(TEXT("NavigateToEnemy"));
			PrimitiveTask.SetOperator(ETestSHTNTaskOperator::NavigateTo, ETestSHTNWorldState::EnemyActor);
			PrimitiveTask.AddEffect(FSHTNEffect(ETestSHTNWorldState::CurrentLocation, ESHTNWorldStateOperation::Set).SetRHSAsWSKey(ETestSHTNWorldState::EnemyActor));
			PrimitiveTask.AddEffect(FSHTNEffect(ETestSHTNWorldState::CanSeeEnemy, ESHTNWorldStateOperation::Set).SetRHSAsValue(1));
		}
		{
			FSHTNPrimitiveTask& PrimitiveTask = Domain.AddPrimitiveTask(TEXT("NavigateToWeapon"));
			PrimitiveTask.SetOperator(ETestSHTNTaskOperator::NavigateTo, ETestSHTNWorldState::PickupLocation);
			PrimitiveTask.AddEffect(FSHTNEffect(ETestSHTNWorldState::CurrentLocation, ESHTNWorldStateOperation::Set).SetRHSAsWSKey(ETestSHTNWorldState::PickupLocation));
		}
		{
			FSHTNPrimitiveTask& PrimitiveTask = Domain.AddPrimitiveTask(TEXT("PickUp"));
			PrimitiveTask.SetOperator(ETestSHTNTaskOperator::PickUp, ETestSHTNWorldState::PickupLocation);
			PrimitiveTask.AddEffect(FSHTNEffect(ETestSHTNWorldState::HasWeapon, ESHTNWorldStateOperation::Set).SetRHSAsValue(1));
		}
		{
			FSHTNPrimitiveTask& PrimitiveTask = Domain.AddPrimitiveTask(TEXT("UseWeapon"));
			PrimitiveTask.SetOperator(ETestSHTNTaskOperator::UseWeapon, ETestSHTNWorldState::EnemyActor);
			PrimitiveTask.AddEffect(FSHTNEffect(ETestSHTNWorldState::Ammo, ESHTNWorldStateOperation::Decrease).SetRHSAsValue(1));
			PrimitiveTask.AddEffect(FSHTNEffect(ETestSHTNWorldState::EnemyHealth, ESHTNWorldStateOperation::Decrease).SetRHSAsValue(1));
		}
	}
};

//////////////////////////////////////////////////////////

struct FAITest_SHTNDomainBuilding : public FAITestBase
{
	FAITest_SHTNDomainBuilding()
	{
		FSHTNDomain Domain;

		//Test if the Domain is empty on initialisation
		Test(TEXT("Initially the Domain should be empty"), Domain.CompositeTasks.Num() == 0 && Domain.PrimitiveTasks.Num() == 0);

		//Test if the Rootname is None on initialisation
		Test(TEXT("Initially the RootTaskName should be NAME_None"), Domain.RootTaskName == NAME_None);

		Domain.SetRootName("Root");

		// Make sure the rootname got set
		Test(TEXT("RootTaskName should be RootName as just set"), Domain.RootTaskName == FName("Root"));

		// Build an imaginary domain
		{
			FSHTNCompositeTask& CompositeTask = Domain.AddCompositeTask(TEXT("Root"));
			{
				FSHTNMethod& Method = CompositeTask.AddMethod(
					TArray<FSHTNCondition>({
						FSHTNCondition(FSHTNDefs::FWSKey(0), ESHTNWorldStateCheck(0)).SetRHSAsValue(0),
						FSHTNCondition(FSHTNDefs::FWSKey(1), ESHTNWorldStateCheck(2))
						}));
				Method.AddTask(TEXT("Task1"));
			}
			{
				FSHTNMethod& Method = CompositeTask.AddMethod(FSHTNCondition(FSHTNDefs::FWSKey(3), ESHTNWorldStateCheck(3)).SetRHSAsWSKey(1));
				Method.AddTask(TEXT("Task2"));
				Method.AddTask(TEXT("Task3"));
			}
		}
		{
			FSHTNCompositeTask& CompositeTask = Domain.AddCompositeTask(TEXT("Task1"));
			{
				FSHTNMethod& Method = CompositeTask.AddMethod(FSHTNCondition(FSHTNDefs::FWSKey(4), ESHTNWorldStateCheck(3)).SetRHSAsWSKey(1));
				Method.AddTask(TEXT("Task4"));
				Method.AddTask(TEXT("Task5"));
				Method.AddTask(TEXT("Root"));
			}
			{
				FSHTNMethod& Method = CompositeTask.AddMethod();
				Method.AddTask(TEXT("Task6"));
				Method.AddTask(TEXT("Task7"));
				Method.AddTask(TEXT("Task8"));
				Method.AddTask(TEXT("Task1"));
			}
		}
		{
			FSHTNPrimitiveTask& PrimitiveTask = Domain.AddPrimitiveTask(TEXT("Task2"));
			PrimitiveTask.SetOperator(0, 0);
		}
		{
			FSHTNPrimitiveTask& PrimitiveTask = Domain.AddPrimitiveTask(TEXT("Task3"));
			PrimitiveTask.SetOperator(1, 1);
			PrimitiveTask.AddEffect(FSHTNEffect(uint8(0), ESHTNWorldStateOperation(0)).SetRHSAsValue(0));
		}
		{
			FSHTNPrimitiveTask& PrimitiveTask = Domain.AddPrimitiveTask(TEXT("Task4"));
			PrimitiveTask.SetOperator(2, 2);
		}
		{
			FSHTNPrimitiveTask& PrimitiveTask = Domain.AddPrimitiveTask(TEXT("Task5"));
			PrimitiveTask.SetOperator(3, 3);
			PrimitiveTask.AddEffect(FSHTNEffect(uint8(0), ESHTNWorldStateOperation(0)).SetRHSAsValue(0));
			PrimitiveTask.AddEffect(FSHTNEffect(uint8(0), ESHTNWorldStateOperation(0)).SetRHSAsValue(0));
		}
		{
			FSHTNPrimitiveTask& PrimitiveTask = Domain.AddPrimitiveTask(TEXT("Task6"));
			PrimitiveTask.SetOperator(4, 4);
		}
		{
			FSHTNPrimitiveTask& PrimitiveTask = Domain.AddPrimitiveTask(TEXT("Task7"));
			PrimitiveTask.SetOperator(5, 5);
		}
		{
			FSHTNPrimitiveTask& PrimitiveTask = Domain.AddPrimitiveTask(TEXT("Task8"));
			PrimitiveTask.SetOperator(3, 4);
		}

		Test(TEXT("Root task should be a composite"), Domain.IsCompositeTask(Domain.RootTaskName) && !Domain.IsPrimitiveTask(Domain.RootTaskName));
		Test(TEXT("Domain should have 2 composite tasks"), Domain.CompositeTasks.Num() == 2);
		Test(TEXT("Domain should have 7 primitive tasks"), Domain.PrimitiveTasks.Num() == 7);

		// Test the first composite for everything we just set up. no need to do this on all tasks as they're the same code.
		{
			const FSHTNCompositeTask& CompositeTask = Domain.GetCompositeTask(Domain.RootTaskName);

			Test(TEXT("Root composite should have 2 methods"), CompositeTask.Methods.Num() == 2);

			Test(TEXT("Root task first method should have 2 conditions"), CompositeTask.Methods[0].Conditions.Num() == 2);
			Test(TEXT("Root task first method should have 1 task"), CompositeTask.Methods[0].Tasks.Num() == 1);

			Test(TEXT("Root task second method should have 1 condition"), CompositeTask.Methods[1].Conditions.Num() == 1);
			Test(TEXT("Root task second method should have 2 tasks"), CompositeTask.Methods[1].Tasks.Num() == 2);
		}

		Test(TEXT("Task5 should be a primitive"), Domain.IsPrimitiveTask(TEXT("Task5")) && !Domain.IsCompositeTask(TEXT("Task5")));

		// Test if Task5 does have 2 effects that have been added
		{
			const FSHTNPrimitiveTask& PrimitiveTask = Domain.GetPrimitiveTask(TEXT("Task5"));

			Test(TEXT("Task5 should have 2 effects"), PrimitiveTask.Effects.Num() == 2);
		}
	}
};
IMPLEMENT_AI_LATENT_TEST(FAITest_SHTNDomainBuilding, "SHTN.DomainBuilding")

//////////////////////////////////////////////////////////

struct FAITest_SHTNDomainValidation : public FAITestBase
{
	FAITest_SHTNDomainValidation()
	{
		FSHTNDomain Domain;

		Test(TEXT("Domain should be Invalid upon initialisation"), !Domain.IsValid());

		// First we will construct an invalid imaginary Domain (Tasks missing)
		Domain.SetRootName("Root");
	
		{
			FSHTNCompositeTask& CompositeTask = Domain.AddCompositeTask("Root");
			{
				FSHTNMethod& Method = CompositeTask.AddMethod();
				Method.AddTask("Task1");
				Method.AddTask("Task2");
			}
			{
				FSHTNMethod& Method = CompositeTask.AddMethod();
				Method.AddTask("Task3");
				Method.AddTask("Task4");
			}
		}
		{
			FSHTNCompositeTask& CompositeTask = Domain.AddCompositeTask("Task1");
			{
				{
					FSHTNMethod& Method = CompositeTask.AddMethod();
					Method.AddTask("Task2");
					Method.AddTask("Task3");
				}
				{
					FSHTNMethod& Method = CompositeTask.AddMethod();
					Method.AddTask("Task4");
				}
			}
		}
		
		Domain.AddPrimitiveTask("Task2");

		// Test incomplete domain
		Test(TEXT("Domain should be invalid"), !Domain.Validate(false));
		Test(TEXT("Domain IsValid should return Invalid"), !Domain.IsValid());

		// Complete the domain
		Domain.AddPrimitiveTask("Task3");
		Domain.AddPrimitiveTask("Task4");

		//Test the complete domain
		Test(TEXT("Domain should be valid"), Domain.Validate());
		Test(TEXT("Domain IsValid should return valid"), Domain.IsValid());
	}
};
IMPLEMENT_AI_LATENT_TEST(FAITest_SHTNDomainValidation, "SHTN.DomainValidation")


//////////////////////////////////////////////////////////

struct FAITest_SHTNCondition : public FSHTNTestBase
{
	FAITest_SHTNCondition()
	{
		PopulateWorldState();

		const uint32 ReferenceValue = 3;

		for (int32 WSIndex = 0; WSIndex < int32(ETestSHTNWorldState::MAX); ++WSIndex)
		{
			for (int32 Value = 0; Value < int32(ETestSHTNWorldState::MAX); ++Value)
			{
				const FSHTNDefs::FWSValue AsValue = FSHTNDefs::FWSValue(Value);
				const FSHTNDefs::FWSKey AsKey = FSHTNDefs::FWSKey(Value);

				// Testing against values by SetRHSAsValue()
				Test(FString::Printf(TEXT("Condition WS[%d] < %d"), WSIndex, AsValue)
					, WorldState.CheckCondition(FSHTNCondition(WSIndex, ESHTNWorldStateCheck::Less).SetRHSAsValue(AsValue)) == (WSIndex < AsValue));
				Test(FString::Printf(TEXT("Condition WS[%d] < %d"), WSIndex, AsValue)
					, WorldState.CheckCondition(FSHTNCondition(WSIndex, ESHTNWorldStateCheck::LessOrEqual).SetRHSAsValue(AsValue)) == (WSIndex <= AsValue));
				Test(FString::Printf(TEXT("Condition WS[%d] < %d"), WSIndex, AsValue)
					, WorldState.CheckCondition(FSHTNCondition(WSIndex, ESHTNWorldStateCheck::Greater).SetRHSAsValue(AsValue)) == (WSIndex > AsValue));
				Test(FString::Printf(TEXT("Condition WS[%d] < %d"), WSIndex, AsValue)
					, WorldState.CheckCondition(FSHTNCondition(WSIndex, ESHTNWorldStateCheck::GreaterOrEqual).SetRHSAsValue(AsValue)) == (WSIndex >= AsValue));
				Test(FString::Printf(TEXT("Condition WS[%d] < %d"), WSIndex, AsValue)
					, WorldState.CheckCondition(FSHTNCondition(WSIndex, ESHTNWorldStateCheck::Equal).SetRHSAsValue(AsValue)) == (WSIndex == AsValue));
				Test(FString::Printf(TEXT("Condition WS[%d] < %d"), WSIndex, AsValue)
					, WorldState.CheckCondition(FSHTNCondition(WSIndex, ESHTNWorldStateCheck::NotEqual).SetRHSAsValue(AsValue)) == (WSIndex != AsValue));

				//Testing against WS values by SetRHSAsKey()
				// Because we assigned the WS to their numerical index, we can use the Key as the value for testing
				Test(FString::Printf(TEXT("Condition WS[%d] < %d"), WSIndex, AsValue)
					, WorldState.CheckCondition(FSHTNCondition(WSIndex, ESHTNWorldStateCheck::Less).SetRHSAsWSKey(AsKey)) == (WSIndex < AsKey));
				Test(FString::Printf(TEXT("Condition WS[%d] < %d"), WSIndex, AsValue)
					, WorldState.CheckCondition(FSHTNCondition(WSIndex, ESHTNWorldStateCheck::LessOrEqual).SetRHSAsWSKey(AsKey)) == (WSIndex <= AsKey));
				Test(FString::Printf(TEXT("Condition WS[%d] < %d"), WSIndex, AsValue)
					, WorldState.CheckCondition(FSHTNCondition(WSIndex, ESHTNWorldStateCheck::Greater).SetRHSAsWSKey(AsKey)) == (WSIndex > AsKey));
				Test(FString::Printf(TEXT("Condition WS[%d] < %d"), WSIndex, AsValue)
					, WorldState.CheckCondition(FSHTNCondition(WSIndex, ESHTNWorldStateCheck::GreaterOrEqual).SetRHSAsWSKey(AsKey)) == (WSIndex >= AsKey));
				Test(FString::Printf(TEXT("Condition WS[%d] < %d"), WSIndex, AsValue)
					, WorldState.CheckCondition(FSHTNCondition(WSIndex, ESHTNWorldStateCheck::Equal).SetRHSAsWSKey(AsKey)) == (WSIndex == AsKey));
				Test(FString::Printf(TEXT("Condition WS[%d] < %d"), WSIndex, AsValue)
					, WorldState.CheckCondition(FSHTNCondition(WSIndex, ESHTNWorldStateCheck::NotEqual).SetRHSAsWSKey(AsKey)) == (WSIndex != AsKey));
			}
		}
	}
};
IMPLEMENT_AI_LATENT_TEST(FAITest_SHTNCondition, "SHTN.Conditions")

////////////////////////////////////////////////////////

struct FAITest_SHTNEffects : public FSHTNTestBase
{
	FAITest_SHTNEffects()
	{
		//Make sure the value of Ammo is 0
		Test(TEXT("Worldstate should be initialized with 0 values"), !WorldState.SetValue(FSHTNDefs::FWSKey(ETestSHTNWorldState::Ammo), 0));

		WorldState.Init(int32(ETestSHTNWorldState::MAX));

		for (int32 Value = 0; Value <= 5; ++Value)
		{
			// Apply Set Effect
			WorldState.ApplyEffect(FSHTNEffect(ETestSHTNWorldState::Ammo, ESHTNWorldStateOperation::Set).SetRHSAsValue(Value));

			Test(FString::Printf(TEXT("Ammo worldstate value should be %d"), Value), WorldState.GetValueUnsafe(FSHTNDefs::FWSKey(ETestSHTNWorldState::Ammo)) == FSHTNDefs::FWSValue(Value));
		}

		int32 AmmoCount = 0;

		// Reset Ammo to AmmoCount
		WorldState.SetValue(FSHTNDefs::FWSKey(ETestSHTNWorldState::Ammo), AmmoCount);

		for (int32 Increment = 0; Increment <= 5; ++Increment)
		{
			AmmoCount += Increment;

			// Apply Increase Effect
			WorldState.ApplyEffect(FSHTNEffect(ETestSHTNWorldState::Ammo, ESHTNWorldStateOperation::Increase).SetRHSAsValue(Increment));

			Test(FString::Printf(TEXT("Ammo worldstate value should be %d"), AmmoCount), WorldState.GetValueUnsafe(FSHTNDefs::FWSKey(ETestSHTNWorldState::Ammo)) == FSHTNDefs::FWSValue(AmmoCount));
		}

		for (int32 Decrement = 0; Decrement <= 5; ++Decrement)
		{
			AmmoCount -= Decrement;

			// Apply Decrease Effect
			WorldState.ApplyEffect(FSHTNEffect(ETestSHTNWorldState::Ammo, ESHTNWorldStateOperation::Decrease).SetRHSAsValue(Decrement));

			Test(FString::Printf(TEXT("Ammo worldstate value should be %d"), AmmoCount), WorldState.GetValueUnsafe(FSHTNDefs::FWSKey(ETestSHTNWorldState::Ammo)) == FSHTNDefs::FWSValue(AmmoCount));
		}

		
		// No need to test as this is a lower enum than ammo
		WorldState.SetValueUnsafe(FSHTNDefs::FWSKey(ETestSHTNWorldState::EnemyHealth), 5);
		
		// Reset Ammo to 0
		WorldState.SetValueUnsafe(FSHTNDefs::FWSKey(ETestSHTNWorldState::Ammo), 0);

		// Increase the Ammo worldstate 3 times by using the value of Enemy Health
		for (int32 i = 0; i < 3; ++i)
		{
			WorldState.ApplyEffect(FSHTNEffect(ETestSHTNWorldState::Ammo, ESHTNWorldStateOperation::Increase).SetRHSAsWSKey(ETestSHTNWorldState::EnemyHealth));
		}

		Test(TEXT("Ammo should be 3 times EnemyHealth (15)"), WorldState.GetValueUnsafe(FSHTNDefs::FWSKey(ETestSHTNWorldState::Ammo)) == 3 * WorldState.GetValueUnsafe(FSHTNDefs::FWSKey(ETestSHTNWorldState::EnemyHealth)));

		// Decrease the Ammo worldstate 3 times by using the value of Enemy Health
		for (int32 i = 0; i < 3; ++i)
		{
			WorldState.ApplyEffect(FSHTNEffect(ETestSHTNWorldState::Ammo, ESHTNWorldStateOperation::Decrease).SetRHSAsWSKey(ETestSHTNWorldState::EnemyHealth));
		}

		Test(TEXT("Ammo should be 0 after decrementing it 3 times again"), WorldState.GetValueUnsafe(FSHTNDefs::FWSKey(ETestSHTNWorldState::Ammo)) == FSHTNDefs::FWSValue(0));

		// Set the Ammo Worldstate to the EnemyHealth worldstate
		WorldState.ApplyEffect(FSHTNEffect(ETestSHTNWorldState::Ammo, ESHTNWorldStateOperation::Set).SetRHSAsWSKey(ETestSHTNWorldState::EnemyHealth));

		Test(TEXT("Ammo should be equal to EnemyHealth"), WorldState.GetValueUnsafe(FSHTNDefs::FWSKey(ETestSHTNWorldState::Ammo)) == WorldState.GetValueUnsafe(FSHTNDefs::FWSKey(ETestSHTNWorldState::EnemyHealth)));
	}
};
IMPLEMENT_AI_LATENT_TEST(FAITest_SHTNEffects, "SHTN.Effects")

////////////////////////////////////////////////////////////////////////

struct FAITest_SHTNPlanning : public FSHTNTestBase
{
	FAITest_SHTNPlanning()
	{
		FSHTNPlan Plan;

		Planner.CreatePlan(Domain, WorldState, Plan);
		Test(TEXT("Planning with an empty domain should result in an empty plan"), Plan.TaskNames.Num() == 0);

		PopulateDomain();
		WorldState.Init(int32(ETestSHTNWorldState::MAX));

		Planner.CreatePlan(Domain, WorldState, Plan);
		Test(TEXT("Plan should be of lenght 2"), Plan.TaskNames.Num() == 2);

		Test(TEXT("Plan element mismatch"), Plan.TaskNames[0] == TEXT("FindPatrolPoint"));
		Test(TEXT("Plan element mismatch"), Plan.TaskNames[1] == TEXT("NavigateToMoveDestination"));

		FSHTNDomain EmptyDomain;
		Planner.CreatePlan(EmptyDomain, WorldState, Plan);
		Test(TEXT("Reusing previous plan with an empty domain should result in an empty plan"), Plan.TaskNames.Num() == 0);

		// Changing the worldstate so that the plan will use the first method
		WorldState.SetValue(FSHTNDefs::FWSKey(ETestSHTNWorldState::EnemyHealth), 1);
		WorldState.SetValue(FSHTNDefs::FWSKey(ETestSHTNWorldState::EnemyActor), 1);

		Planner.CreatePlan(Domain, WorldState, Plan);

		Test(TEXT("After WorldState changes the plan should be 7 tasks long"), Plan.TaskNames.Num() == 7);
	}
};
IMPLEMENT_AI_LATENT_TEST(FAITest_SHTNPlanning, "SHTN.Planning")

//////////////////////////////////////////////////////////////////////////

struct FAITest_SHTNPlanningRollback : public FSHTNTestBase
{
	FAITest_SHTNPlanningRollback()
	{
		FSHTNPlan Plan;

		// Build a domain that will force rolling back
		// First method should get accepted in the first step
		// The task in the method should fail its condition so it will roll back to the other method

		Domain.SetRootName(TEXT("Root"));

		{
			FSHTNCompositeTask& CompositeTask = Domain.AddCompositeTask(TEXT("Root"));
			{
				FSHTNMethod& Method = CompositeTask.AddMethod();
				Method.AddTask(TEXT("FailedComposite"));
			}
			{
				FSHTNMethod& Method = CompositeTask.AddMethod();
				Method.AddTask(TEXT("SuccessfulComposite"));
			}
		}
		{
			FSHTNCompositeTask& CompositeTask = Domain.AddCompositeTask(TEXT("FailedComposite"));
			{
				FSHTNMethod& Method = CompositeTask.AddMethod(FSHTNCondition(0, ESHTNWorldStateCheck::Greater).SetRHSAsValue(0));
				Method.AddTask(TEXT("DummyPrimitive1"));
			}
		}
		{
			FSHTNCompositeTask& CompositeTask = Domain.AddCompositeTask(TEXT("SuccessfulComposite"));
			{
				FSHTNMethod& Method = CompositeTask.AddMethod(FSHTNCondition(0, ESHTNWorldStateCheck::Equal).SetRHSAsValue(0));
				Method.AddTask(TEXT("DummyPrimitive2"));
			}

		}

		Domain.AddPrimitiveTask(TEXT("DummyPrimitive1"));
		Domain.AddPrimitiveTask(TEXT("DummyPrimitive2"));

		WorldState.Init(1);
		Planner.CreatePlan(Domain, WorldState, Plan);

		Test(TEXT("The plan should consist of one primitive task: DummyPrimitive2"), Plan.TaskNames.Num() == 1 && Plan.TaskNames[0] == TEXT("DummyPrimitive2"));
	}
};
IMPLEMENT_AI_LATENT_TEST(FAITest_SHTNPlanningRollback, "SHTN.PlanningRollback")