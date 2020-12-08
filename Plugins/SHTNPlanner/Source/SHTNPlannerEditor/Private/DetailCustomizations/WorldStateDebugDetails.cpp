#include "WorldStateDebugDetails.h"

#include "PropertyEditing.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "InputCoreTypes.h"

#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "SHTNComponent.h"
#include "SHTNPlannerEditor.h"

#include "WorldStateDebugTool.h"

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
			IDetailCategoryBuilder& WorldState = DetailBuilder.EditCategory("World State");

			int EnumAmount = ToolInstance->SelectedComponent->WorldStateEnumAsset->NumEnums();

			for (int32 i = 0; i < EnumAmount - 1; i++)
			{
				FText EnumName = ToolInstance->SelectedComponent->WorldStateEnumAsset->GetDisplayNameTextByIndex(i);

				WorldState.AddCustomRow(FText::Format(LOCTEXT("WorldState", "{0}"), EnumName))
					.NameContent()
					[
						SNew(STextBlock)
						.Text(EnumName)
						.MinDesiredWidth(500.f)
					]
					.ValueContent()
					[
						SNew(SNumericEntryBox<int32>)
						.IsEnabled_Lambda([]() {return false; })
						.Value_Lambda([this, i]() 
						{ 
						int32 Value;
						if (ToolInstance->SelectedComponent && ToolInstance->SelectedComponent->WorldState.GetValue(i, Value))
						{
							return Value;
						}

						return int32(0);
						})
						.BorderBackgroundColor(FSlateColor(FLinearColor(0, 255, 0)))
					];
			}
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
		ToolInstance->CurrentIndex = Index;
		ToolInstance->SelectedComponent = ToolInstance->HTNComponents[Index];

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

#undef LOCTEXT_NAMESPACE