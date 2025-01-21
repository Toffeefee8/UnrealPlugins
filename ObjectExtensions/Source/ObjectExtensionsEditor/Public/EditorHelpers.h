#pragma once

struct OBJECTEXTENSIONSEDITOR_API EditorHelpers
{
public:
	template <typename T>
	static bool GetRawData(const TSharedPtr<IPropertyHandle>& PropertyHandle, T*& OutStruct);

	static void DisplayChildError(IDetailChildrenBuilder& ChildBuilder, const FString& Text = "An Error Occured.");
	static void DisplayEmptyChildren(IDetailChildrenBuilder& ChildBuilder);
	static void DisplayCustomTextChild(IDetailChildrenBuilder& ChildBuilder, const FString& NameContent = "An Error Occured.", const FString& ValueContent = "An Error Occured.");

	static void DisplayDefaultHeader(const TSharedRef<IPropertyHandle>& PropertyHandle, FDetailWidgetRow& HeaderRow);
	static void DisplayDefaultChildren(const TSharedRef<IPropertyHandle>& PropertyHandle, IDetailChildrenBuilder& ChildBuilder, const TArray<TSharedPtr<IPropertyHandle>>& ExcludedHandles = {});
	static void DisplayCompactHeader(const TSharedRef<IPropertyHandle>& PropertyHandle, FDetailWidgetRow& HeaderRow);
	
	static TSharedRef<SHorizontalBox> GetChildPropertiesHorizontal(TSharedRef<IPropertyHandle> ParentPropertyHandle);
	static TSharedRef<SHorizontalBox> GetPropertiesHorizontal(TArray<TSharedRef<IPropertyHandle>> ParentPropertyHandleArray);
	static TSharedRef<SHorizontalBox> GetWidgetsHorizontal(const TArray<TSharedRef<SWidget>>& WidgetsArray, bool AutoSize = false);

	static TSharedRef<STextBlock> MakeDefaultEditorText(const FString& Text);

	static FName NoAutoSizeTag;
};

template <typename T>
bool EditorHelpers::GetRawData(const TSharedPtr<IPropertyHandle>& PropertyHandle, T*& OutStruct)
{
	TArray<void*> RawData;
	PropertyHandle->AccessRawData(RawData);

	if (RawData.Num() > 0)
	{
		if (T* EditedStruct = static_cast<T*>(RawData[0]))
		{
			OutStruct = EditedStruct;
			return true;
		}
	}
	return false;
}