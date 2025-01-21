#include "Common/CompactRowEditor.h"

#include "EditorHelpers.h"

TSharedRef<IPropertyTypeCustomization> FCompactRowEditor::MakeInstance()
{
	return MakeShareable(new FCompactRowEditor);
}

void FCompactRowEditor::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow,
	IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	if (PropertyHandle->HasMetaData(NoEditInlineMetaTag))
		EditorHelpers::DisplayDefaultHeader(PropertyHandle, HeaderRow);
	else
		EditorHelpers::DisplayCompactHeader(PropertyHandle, HeaderRow);
}

void FCompactRowEditor::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder,
	IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	if (PropertyHandle->HasMetaData(NoEditInlineMetaTag))
		EditorHelpers::DisplayDefaultChildren(PropertyHandle, ChildBuilder);
}
