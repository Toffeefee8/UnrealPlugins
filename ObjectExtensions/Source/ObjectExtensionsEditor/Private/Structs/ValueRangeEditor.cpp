#include "Structs/ValueRangeEditor.h"

#include "DetailWidgetRow.h"
#include "EditorHelpers.h"
#include "IDetailChildrenBuilder.h"
#include "IPropertyUtilities.h"

TSharedRef<IPropertyTypeCustomization> FValueRangeEditor::MakeInstance()
{
	return MakeShareable(new FValueRangeEditor);
}

void FValueRangeEditor::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow,
	IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	TSharedPtr<IPropertyHandle> BoolHandle = PropertyHandle->GetChildHandle("bUseRange", false);
	bool bUseRange = false;
	BoolHandle->GetValue(bUseRange);

	HeaderRow.NameContent()
	[
		PropertyHandle->CreatePropertyNameWidget()
	];

	if (bUseRange)
	{
		TSharedPtr<IPropertyHandle> MinHandle = PropertyHandle->GetChildHandle("MinValue", false);
		TSharedPtr<IPropertyHandle> MaxHandle = PropertyHandle->GetChildHandle("MaxValue", false);
		HeaderRow.ValueContent()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			[
				MinHandle->CreatePropertyValueWidget()
			]
			+ SHorizontalBox::Slot()
			[
				EditorHelpers::MakeDefaultEditorText("<->")
			]
			.VAlign(VAlign_Center)
			.AutoWidth()
			.Padding(4, 0)
			+ SHorizontalBox::Slot()
			[
				MaxHandle->CreatePropertyValueWidget()
			]
		];
	}
	else
	{
		TSharedPtr<IPropertyHandle> FixedHandle = PropertyHandle->GetChildHandle("FixedValue", false);
		HeaderRow.ValueContent()
		[
			FixedHandle->CreatePropertyValueWidget()
		];
	}
}

void FValueRangeEditor::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder,
	IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	TSharedPtr<IPropertyHandle> BoolHandle = PropertyHandle->GetChildHandle("bUseRange", false);

	IDetailPropertyRow& BoolPropertyRow = ChildBuilder.AddProperty(BoolHandle.ToSharedRef());

	auto PropertyUtils = CustomizationUtils.GetPropertyUtilities();
	BoolHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSPLambda(this,[PropertyUtils]()
	{
		   if (PropertyUtils.IsValid())
			   PropertyUtils->ForceRefresh();
	}));
}
