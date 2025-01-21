#pragma once

#include "CoreMinimal.h"

class OBJECTEXTENSIONSEDITOR_API FValueRangeEditor : public IPropertyTypeCustomization
{
	
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();
	
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;

protected:
	
	const FName NoEditInlineMetaTag = "NoEditInline";
};
