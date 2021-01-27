#include "WorldStateDebugDetails.h"

#include "PropertyEditing.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "InputCoreTypes.h"
#include "SGraphActionMenu.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/SWidget.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Engine/Selection.h"

#include "AIController.h"
#include "SHTNComponent.h"
#include "SHTNPlannerEditor.h"

#include "BehaviorTreeEditor/Private/SBehaviorTreeBlackboardView.cpp"
#include "BehaviorTreeEditor\Private\SBehaviorTreeBlackboardEditor.h"

#include "WorldStateDebugTool.h"

DEFINE_LOG_CATEGORY(LogBlackboardEditor);

#define LOCTEXT_NAMESPACE "WorldStateDebugDetails"

TSharedRef<IDetailCustomization> FWorldStateDebugDetails::MakeInstance()
{
	return MakeShareable(new FWorldStateDebugDetails);
}

void FWorldStateDebugDetails::RefreshDetails()
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditorModule.NotifyCustomizationModuleChanged();
}

void FWorldStateDebugDetails::OnNewPlanMade()
{
	FillPlanTaskNames();

	if (CurrentPlanWidget)
	{
		CurrentPlanWidget->RequestListRefresh();
	}
	else
	{
		DetailBuilderPtr->ForceRefreshDetails();
	}
}

void FWorldStateDebugDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DetailBuilderPtr = &DetailBuilder;

	TArray<TWeakObjectPtr<UObject>> CustomizedObjects;
	DetailBuilder.GetObjectsBeingCustomized(CustomizedObjects);
	ToolInstance = CastChecked<UWorldStateDebugTool>(CustomizedObjects[0].Get());

	ToolInstance->OnCreation(this);
	
	IDetailCategoryBuilder& Selection = DetailBuilder.EditCategory("Selection");

	Selection.AddCustomRow(LOCTEXT("SelectedAgent", "Selection Agent Selected"))
		.NameContent()
		[
		SNew(STextBlock)
			.Text(LOCTEXT("Selected HTN Agent", "Selected HTN Agent"))
		]
		.ValueContent()
		[
		SNew(SComboButton)
			.OnGetMenuContent(this, &FWorldStateDebugDetails::OnGetMenuContent)
			.IsEnabled(this, &FWorldStateDebugDetails::IsEditingEnabled)
			.ButtonContent()
		[
		SNew(STextBlock)
			.Text(this, &FWorldStateDebugDetails::GetCurrentAgentDesc)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		];

		if (ToolInstance->SelectedComponent)
		{
			Selection.AddCustomRow(LOCTEXT("Select Agent in PIE", "Select Agent in PIE"))
				.NameContent()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("Select Agent in PIE", "Select Agent in PIE"))
				]
			.ValueContent()
				[
					SNew(SCheckBox)
					.IsChecked(ToolInstance->bSelectInPIE)
					.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState)
			{	
				if (NewState == ECheckBoxState::Checked)
				{
					OnObjectSelectedHandle = USelection::SelectObjectEvent.AddSP(this, &FWorldStateDebugDetails::OnObjectSelected);
					ToolInstance->bSelectInPIE = true;
				}
				else
				{
					USelection::SelectObjectEvent.Remove(OnObjectSelectedHandle);
					ToolInstance->bSelectInPIE = false;
				}
			})
				];

			if (ToolInstance->bSelectInPIE)
			{
				OnObjectSelectedHandle = USelection::SelectObjectEvent.AddSP(this, &FWorldStateDebugDetails::OnObjectSelected);
			}


			IDetailCategoryBuilder& WorldState = DetailBuilder.EditCategory("WorldState");
			WorldState.AddCustomRow(LOCTEXT("WorldState", "WorldState"))
				.WholeRowContent()
				[
					SNew(SGraphActionMenu, true)
					.OnCreateWidgetForAction(this, &FWorldStateDebugDetails::HandleCreateWidgetForAction)
				.OnCollectAllActions(this, &FWorldStateDebugDetails::HandleCollectAllActions)
				.OnGetSectionTitle(this, &FWorldStateDebugDetails::HandleGetSectionTitle)
				//.OnActionSelected(this, &SBehaviorTreeBlackboardView::HandleActionSelected)
				//.OnContextMenuOpening(this, &SBehaviorTreeBlackboardView::HandleContextMenuOpening, InCommandList)
				//.OnActionMatchesName(this, &SBehaviorTreeBlackboardView::HandleActionMatchesName)
				.AlphaSortItems(GetDefault<UEditorPerProjectUserSettings>()->bDisplayBlackboardKeysInAlphabeticalOrder)
				.AutoExpandActionMenu(true)
				];

			FillPlanTaskNames();

			IDetailCategoryBuilder& Plan = DetailBuilder.EditCategory("Plan");
			Plan.AddCustomRow(LOCTEXT("Current Task", "Current Task"))
				.NameContent()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("Current Task:", "Current Task:"))
				]
				.ValueContent()
				[
					SNew(STextBlock)
					.Text_Raw(this, &FWorldStateDebugDetails::GetCurrentTaskName)
				];

			Plan.AddCustomRow(LOCTEXT("CurrentPlan", "CurrentPlan"))
				.WholeRowContent()
				[
					SNew(SExpandableArea)
					.AreaTitle(LOCTEXT("Current Plan", "Current Plan"))
					.InitiallyCollapsed(false)
					.BodyContent()
				[
					SAssignNew(CurrentPlanWidget, SListView<TSharedPtr<FString>>)
					.ListItemsSource(&PlanTaskNames)
					.OnGenerateRow(this, &FWorldStateDebugDetails::OnGenerateRowForList)
				]
				];
		}


		DetailBuilder.HideCategory("Tags");
		DetailBuilder.HideCategory("ComponentReplication");
		DetailBuilder.HideCategory("Activation");
		DetailBuilder.HideCategory("Variable");
		DetailBuilder.HideCategory("Cooking");
		DetailBuilder.HideCategory("AssetUserData");
}

void FWorldStateDebugDetails::OnAgentComboChange(int32 Index)
{
	if (Index != ToolInstance->CurrentIndex)
	{
		if (ToolInstance->OnNewPlanMadeHandle.IsValid())
		{
			ToolInstance->SelectedComponent->OnNewPlanMade.Remove(ToolInstance->OnNewPlanMadeHandle);
			ToolInstance->OnNewPlanMadeHandle.Reset();
		}
		
		ToolInstance->CurrentIndex = Index;
		ToolInstance->SelectedComponent = ToolInstance->HTNComponents[Index];

		FillPlanTaskNames();

		DetailBuilderPtr->ForceRefreshDetails();
	}
}

TSharedRef<SWidget> FWorldStateDebugDetails::OnGetMenuContent() const
{
	FMenuBuilder MenuBuilder(true, NULL);

	for (int32 Idx = 0; Idx < ToolInstance->HTNComponents.Num(); Idx++)
	{
		FUIAction ItemAction(FExecuteAction::CreateSP(this, &FWorldStateDebugDetails::OnAgentComboChange, Idx));
		MenuBuilder.AddMenuEntry(FText::FromName(ToolInstance->HTNComponents[Idx]->OwningPawnName), TAttribute<FText>(), FSlateIcon(), ItemAction);
	}

	return MenuBuilder.MakeWidget();
}

FText FWorldStateDebugDetails::GetCurrentAgentDesc() const
{
	if (ToolInstance->bIsPlayingWorld)
	{
		if (IsValid(ToolInstance->SelectedComponent))
		{
			return FText::FromName(ToolInstance->SelectedComponent->OwningPawnName);
		}
		else
		{
			return FText::FromString("No HTN Agent found");
		}
	}
	else
	{
		return FText::FromString("No active world");
	}
}

bool FWorldStateDebugDetails::IsEditingEnabled() const
{
	return ToolInstance->bIsPlayingWorld && ToolInstance->HTNComponents.Num();
}

void FWorldStateDebugDetails::FillPlanTaskNames()
{
	PlanTaskNames.Empty();

	for (auto& Name : ToolInstance->SelectedComponent->DebugTaskNames)
	{
		PlanTaskNames.Add(MakeShareable(new FString(Name.ToString())));
	}
}

FText FWorldStateDebugDetails::GetCurrentTaskName() const
{
	return FText::FromName(ToolInstance->SelectedComponent->CurrentTaskName);
}

TSharedRef<ITableRow> FWorldStateDebugDetails::OnGenerateRowForList(TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
		[
			SNew(STextBlock).Text(FText::FromString(*Item.Get()))
		];
}

TSharedRef<SWidget> FWorldStateDebugDetails::HandleCreateWidgetForAction(FCreateWidgetForActionData* const InCreateData)
{
	return SNew(SBehaviorTreeBlackboardItem, InCreateData)
		.OnIsDebuggerReady(this, &FWorldStateDebugDetails::IsDebuggerReady)
		.OnGetDebugKeyValue(this, &FWorldStateDebugDetails::HandleGetDebugKeyValue)
		.OnGetDisplayCurrentState(this, &FWorldStateDebugDetails::DisplayCurrentState);
}

void FWorldStateDebugDetails::HandleCollectAllActions(FGraphActionListBuilderBase& GraphActionListBuilder)
{
	if (ToolInstance->SelectedComponent != nullptr)
	{
		UBlackboardData* BlackboardData = ToolInstance->SelectedComponent->BlackboardState->GetBlackboardAsset();
		for (auto& ParentKey : BlackboardData->ParentKeys)
		{
			GraphActionListBuilder.AddAction(MakeShareable(new FEdGraphSchemaAction_BlackboardEntry(BlackboardData, ParentKey, true)));
		}

		for (auto& Key : BlackboardData->Keys)
		{
			GraphActionListBuilder.AddAction(MakeShareable(new FEdGraphSchemaAction_BlackboardEntry(BlackboardData, Key, false)));
		}
	}
}

FText FWorldStateDebugDetails::HandleGetDebugKeyValue(const FName& InKeyName, bool bUseCurrentState) const
{
	if (ToolInstance && ToolInstance->SelectedComponent)
	{
		return FText::FromString(ToolInstance->SelectedComponent->BlackboardState->DescribeKeyValue(InKeyName, EBlackboardDescription::OnlyValue));
	}

	return FText::FromString("Error");
}

bool FWorldStateDebugDetails::IsDebuggerReady() const
{
	return IsValid(ToolInstance->SelectedComponent);
}

FText FWorldStateDebugDetails::HandleGetSectionTitle(int32 SectionID) const
{
	switch (SectionID)
	{
	case EBlackboardSectionTitles::InheritedKeys:
		return LOCTEXT("InheritedKeysSectionLabel", "Inherited Keys");
	case EBlackboardSectionTitles::Keys:
		return LOCTEXT("KeysSectionLabel", "Keys");
	}

	return FText();
}
void FWorldStateDebugDetails::OnObjectSelected(UObject * Object)
{
	int32 Idx = ToolInstance->HTNComponents.IndexOfByPredicate([Object](USHTNComponent* Component) { return Component->OwningPawnName == Object->GetFName(); });

	if(Idx >= 0)
	{
		OnAgentComboChange(Idx);
	}
}
#undef LOCTEXT_NAMESPACE