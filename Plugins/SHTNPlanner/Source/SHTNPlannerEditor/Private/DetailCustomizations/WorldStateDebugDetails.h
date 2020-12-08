#pragma once

#include "CoreMinimal.h"
#include "UnrealClient.h"
#include "IDetailCustomization.h"
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

private:

	void OnAgentComboChange(int32 Index);
	TSharedRef<SWidget> OnGetMenuContent() const;
	FText GetCurrentAgentDesc() const;
	bool IsEditingEnabled() const;

	UWorldStateDebugTool* ToolInstance;
	IDetailLayoutBuilder* DetailBuilderPtr;
};