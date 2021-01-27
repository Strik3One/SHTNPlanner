#pragma once

#include "CoreMinimal.h"
#include "UnrealClient.h"
#include "IDetailCustomization.h"
#include "SGraphActionMenu.h"
#include "IPropertyTypeCustomization.h"

class IPropertyHandle;
class USHTNComponent;
class UWorldStateDebugTool;

class FWorldStateDebugDetails : public IDetailCustomization
{
public:

	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();

	/** IDetailCustomization interface */
	void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	//void CustomizeDetails(const TSharedPtr<IDetailLayoutBuilder>& DetailBuilder) override;

	void RefreshDetails();

	void OnNewPlanMade();

	void FillPlanTaskNames();

private:

	void OnAgentComboChange(int32 Index);
	TSharedRef<SWidget> OnGetMenuContent() const;
	FText GetCurrentAgentDesc() const;
	bool IsEditingEnabled() const;

	UWorldStateDebugTool* ToolInstance;
	IDetailLayoutBuilder* DetailBuilderPtr;

	TArray<TSharedPtr<FString>> PlanTaskNames;

	FText GetCurrentTaskName() const;

	/* The list widget for the CurrentPlan */
	TSharedPtr<SListView<TSharedPtr<FString>>> CurrentPlanWidget;

	/* Adds a new textbox with the string to the list */
	TSharedRef<ITableRow> OnGenerateRowForList(TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& OwnerTable);

	/** Delegate handler used to generate a widget for an 'action' (key) in the list */
	TSharedRef<class SWidget> HandleCreateWidgetForAction(FCreateWidgetForActionData* const InCreateData);

	/** Delegate handler used to collect all 'actions' (keys) for display */
	void HandleCollectAllActions(FGraphActionListBuilderBase& GraphActionListBuilder);

	/** Delegate handler for displaying debugger values */
	FText HandleGetDebugKeyValue(const FName& InKeyName, bool bUseCurrentState) const;

	/** Get the title of the specified section ID */
	FText HandleGetSectionTitle(int32 SectionID) const;

	void OnObjectSelected(UObject* Object);

	FDelegateHandle OnObjectSelectedHandle;

	bool IsDebuggerReady() const;
	bool DisplayCurrentState() const { return true; }

};