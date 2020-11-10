#include "UserDefinedOperatorDetails.h"
#include "SHTNNetwork_BlueprintBase.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/SWidget.h"
#include "PropertyEditing.h"

#define LOCTEXT_NAMESPACE "UserDefinedWorldStateDetails"

TSharedRef<IPropertyTypeCustomization> FUserDefinedOperatorDetails::MakeInstance()
{
	return MakeShareable(new FUserDefinedOperatorDetails);
}

void FUserDefinedOperatorDetails::CustomizeHeader(TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	WorldStateKeyValueProperty = StructPropertyHandle->GetChildHandle(TEXT("KeyValue"));

	OperatorKeyValues.Reset();

	TArray<UObject*> MyObjects;
	StructPropertyHandle->GetOuterObjects(MyObjects);
	for (int32 i = 0; i < MyObjects.Num(); i++)
	{
		USHTNNetwork_BlueprintBase* Network = Cast<USHTNNetwork_BlueprintBase>(MyObjects[i]);
		if (Network)
		{
			CachedWorldStateEnumAsset = Network->OperatorEnumAsset;
			break;
		}
	}

	if (CachedWorldStateEnumAsset)
	{

		for (int32 i = 0; i < CachedWorldStateEnumAsset->NumEnums() - 1; i++)
		{
			FString DisplayedName = CachedWorldStateEnumAsset->GetDisplayNameTextByIndex(i).ToString();
			OperatorKeyValues.Add(DisplayedName);
		}
	}
}

void FUserDefinedOperatorDetails::CustomizeChildren(TSharedRef<class IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	StructBuilder.AddCustomRow(LOCTEXT("TypeRow", "TypeRow"))
		.NameContent()
		[
			WorldStateKeyValueProperty->CreatePropertyNameWidget()
		]
	.ValueContent()
		[
			SNew(SComboButton)
			.OnGetMenuContent(this, &FUserDefinedOperatorDetails::OnGetKeyContent)
		.IsEnabled(this, &FUserDefinedOperatorDetails::IsEditingEnabled)
		.ContentPadding(FMargin(2.0f, 2.0f))
		.ButtonContent()
		[
			SNew(STextBlock)
			.Text(this, &FUserDefinedOperatorDetails::GetCurrentKeyDesc)
		.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		];
}

TSharedRef<SWidget> FUserDefinedOperatorDetails::OnGetKeyContent() const
{
	FMenuBuilder MenuBuilder(true, NULL);

	for (int32 Idx = 0; Idx < OperatorKeyValues.Num(); Idx++)
	{
		FUIAction ItemAction(FExecuteAction::CreateSP(const_cast<FUserDefinedOperatorDetails*>(this), &FUserDefinedOperatorDetails::OnKeyComboChange, Idx));
		MenuBuilder.AddMenuEntry(FText::FromString(OperatorKeyValues[Idx]), TAttribute<FText>(), FSlateIcon(), ItemAction);
	}

	return MenuBuilder.MakeWidget();
}

FText FUserDefinedOperatorDetails::GetCurrentKeyDesc() const
{
	FPropertyAccess::Result Result = FPropertyAccess::Fail;
	uint8 CurrentIntValue = MAX_uint8;

	if (CachedWorldStateEnumAsset)
	{
		Result = WorldStateKeyValueProperty->GetValue(CurrentIntValue);
	}

	if (Result == FPropertyAccess::Success && OperatorKeyValues.IsValidIndex(CurrentIntValue))
		return FText::FromString(OperatorKeyValues[CurrentIntValue]);
	else
		return FText::GetEmpty();
}

void FUserDefinedOperatorDetails::OnKeyComboChange(int32 Index)
{
	WorldStateKeyValueProperty->SetValue(uint8(Index));
}

#undef LOCTEXT_NAMESPACE