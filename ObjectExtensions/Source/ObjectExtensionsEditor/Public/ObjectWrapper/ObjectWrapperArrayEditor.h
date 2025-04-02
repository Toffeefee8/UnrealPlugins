#pragma once

#include "CoreMinimal.h"

class UItemShard;

class FObjectWrapperArrayEditor : public IPropertyTypeCustomization
{
	
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();
	
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;

	virtual ~FObjectWrapperArrayEditor() override;
	
private:

	TArray<TSubclassOf<UItemShard>> RequiredClassCache;
	TWeakPtr<IPropertyHandle> CachedPropertyHandle;
};
