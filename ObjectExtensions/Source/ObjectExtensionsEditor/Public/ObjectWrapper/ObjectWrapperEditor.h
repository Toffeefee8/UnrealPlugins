#pragma once

#include "CoreMinimal.h"

class IDetailPropertyRow;

DECLARE_LOG_CATEGORY_CLASS(ObjectWrapperEditorLog, Log, All);

class FObjectWrapperEditor : public IPropertyTypeCustomization
{
	
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();
	
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;

	virtual ~FObjectWrapperEditor() override;

private:
	
	static bool CheckForInfiniteLoop(const TSharedPtr<IPropertyHandle>& PropertyHandle, const UClass* SelectedClass);
	static bool HasPropertyTag(const FProperty* Property);
	void ReevaluateClassSelection();
	void CaptureData();

	static constexpr uint64 RequiredPropertyFlags = EPropertyFlags::CPF_Edit;
	static constexpr uint64 BlockingPropertyFlags = EPropertyFlags::CPF_DisableEditOnInstance;
	static const FName FullRefreshMetaTag;
	TWeakObjectPtr<> CreatedUObject;
	TWeakPtr<IPropertyHandle> CachedPropertyHandle;
	TArray<IDetailPropertyRow*> PropertyRows;
};
