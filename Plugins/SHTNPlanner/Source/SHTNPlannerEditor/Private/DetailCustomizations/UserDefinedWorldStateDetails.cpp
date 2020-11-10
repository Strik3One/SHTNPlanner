#include "UserDefinedWorldStateDetails.h"
#include "SHTNNetwork_BlueprintBase.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/SWidget.h"
#include "PropertyEditing.h"

#define LOCTEXT_NAMESPACE "UserDefinedWorldStateDetails"

TSharedRef<IPropertyTypeCustomization> FUserDefinedWorldStateDetails::MakeInstance()
{
	return MakeShareable(new FUserDefinedWorldStateDetails);
}

void FUserDefinedWorldStateDetails::CustomizeHeader(TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	WorldStateKeyValueProperty = StructPropertyHandle->GetChildHandle(TEXT("KeyValue"));

	WorldStateKeyValues.Reset();

	TArray<UObject*> MyObjects;
	StructPropertyHandle->GetOuterObjects(MyObjects);
	for (int32 i = 0; i < MyObjects.Num(); i++)
	{
		USHTNNetwork_BlueprintBase* Network = Cast<USHTNNetwork_BlueprintBase>(MyObjects[i]);
		if (Network)
		{
			CachedWorldStateEnumAsset = Network->WorldStateEnumAsset;
			break;
		}
	}

	if (CachedWorldStateEnumAsset)
	{

		for (int32 i = 0; i < CachedWorldStateEnumAsset->NumEnums() - 1; i++)
		{
			FString DisplayedName = CachedWorldStateEnumAsset->GetDisplayNameTextByIndex(i).ToString();
			WorldStateKeyValues.Add(DisplayedName);
		}
	}
}

void FUserDefinedWorldStateDetails::CustomizeChildren(TSharedRef<class IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	StructBuilder.AddCustomRow(LOCTEXT("TypeRow", "TypeRow"))
		.NameContent()
		[
			WorldStateKeyValueProperty->CreatePropertyNameWidget()
		]
	.ValueContent()
		[
			SNew(SComboButton)
			.OnGetMenuContent(this, &FUserDefinedWorldStateDetails::OnGetKeyContent)
		.IsEnabled(this, &FUserDefinedWorldStateDetails::IsEditingEnabled)
		.ContentPadding(FMargin(2.0f, 2.0f))
		.ButtonContent()
		[
			SNew(STextBlock)
			.Text(this, &FUserDefinedWorldStateDetails::GetCurrentKeyDesc)
		.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		];
}

TSharedRef<SWidget> FUserDefinedWorldStateDetails::OnGetKeyContent() const
{
	FMenuBuilder MenuBuilder(true, NULL);

	for (int32 Idx = 0; Idx < WorldStateKeyValues.Num(); Idx++)
	{
		FUIAction ItemAction(FExecuteAction::CreateSP(const_cast<FUserDefinedWorldStateDetails*>(this), &FUserDefinedWorldStateDetails::OnKeyComboChange, Idx));
		MenuBuilder.AddMenuEntry(FText::FromString(WorldStateKeyValues[Idx]), TAttribute<FText>(), FSlateIcon(), ItemAction);
	}

	return MenuBuilder.MakeWidget();
}

FText FUserDefinedWorldStateDetails::GetCurrentKeyDesc() const
{
	FPropertyAccess::Result Result = FPropertyAccess::Fail;
	uint8 CurrentIntValue = MAX_uint8;

	if (CachedWorldStateEnumAsset)
	{
		Result = WorldStateKeyValueProperty->GetValue(CurrentIntValue);
	}

	if (Result == FPropertyAccess::Success && WorldStateKeyValues.IsValidIndex(CurrentIntValue))
		return FText::FromString(WorldStateKeyValues[CurrentIntValue]);
	else
		return FText::GetEmpty();
}

void FUserDefinedWorldStateDetails::OnKeyComboChange(int32 Index)
{
	WorldStateKeyValueProperty->SetValue(uint8(Index));
}

#undef LOCTEXT_NAMESPACE