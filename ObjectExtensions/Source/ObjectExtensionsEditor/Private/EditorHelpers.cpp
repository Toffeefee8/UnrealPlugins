#include "EditorHelpers.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"

FName EditorHelpers::NoAutoSizeTag = "NoAutoSize";

void EditorHelpers::DisplayChildError(IDetailChildrenBuilder& ChildBuilder, const FString& Text)
{
	ChildBuilder.AddCustomRow(FText::FromString("Error"))
				.NameContent()
	[
		SNew(STextBlock)
		.Text(FText::FromString("Error:"))
		.Font(IDetailLayoutBuilder::GetDetailFont())
		.ColorAndOpacity(FColor::Red)
	]
	.ValueContent()
	[
		SNew(STextBlock)
		.Text(FText::FromString(Text))
		.Font(IDetailLayoutBuilder::GetDetailFont())
		.ColorAndOpacity(FColor::Red)
	];
}

void EditorHelpers::DisplayEmptyChildren(IDetailChildrenBuilder& ChildBuilder)
{
	ChildBuilder.AddCustomRow(FText::FromString("Error"))
	.ValueContent()
	[
		SNew(STextBlock)
		.Text(FText::FromString("No Children Found."))
		.Font(IDetailLayoutBuilder::GetDetailFont())
	];
}

void EditorHelpers::DisplayCustomTextChild(IDetailChildrenBuilder& ChildBuilder, const FString& NameContent,
	const FString& ValueContent)
{
	ChildBuilder.AddCustomRow(FText::FromString("Error"))
	.NameContent()
	[
		SNew(STextBlock)
		.Text(FText::FromString(NameContent))
		.Font(IDetailLayoutBuilder::GetDetailFont())
	]
	.ValueContent()
	[
		SNew(STextBlock)
		.Text(FText::FromString(ValueContent))
		.Font(IDetailLayoutBuilder::GetDetailFont())
	];
}


void EditorHelpers::DisplayDefaultHeader(const TSharedRef<IPropertyHandle>& PropertyHandle, FDetailWidgetRow& HeaderRow)
{
	HeaderRow.NameContent()
	[
		PropertyHandle->CreatePropertyNameWidget()
	];

	HeaderRow.ValueContent()
	[
		PropertyHandle->CreatePropertyValueWidget()
	];
}

void EditorHelpers::DisplayDefaultChildren(const TSharedRef<IPropertyHandle>& PropertyHandle,
										   IDetailChildrenBuilder& ChildBuilder, const TArray<TSharedPtr<IPropertyHandle>>& ExcludedHandles)
{
	uint32 NumChildren;
	PropertyHandle->GetNumChildren(NumChildren);
	
	for (uint32 ChildIndex = 0; ChildIndex < NumChildren; ++ChildIndex)
	{
		TSharedPtr<IPropertyHandle> ChildProperty = PropertyHandle->GetChildHandle(ChildIndex);
		
		bool bIsExcluded = ExcludedHandles.ContainsByPredicate([&](const TSharedPtr<IPropertyHandle>& ExcludedHandle)
		{
			return ExcludedHandle.IsValid() && ExcludedHandle->GetProperty() == ChildProperty->GetProperty();
		});

		if (bIsExcluded)
			continue;
		
		ChildBuilder.AddProperty(ChildProperty.ToSharedRef());
	}
}

void EditorHelpers::DisplayCompactHeader(const TSharedRef<IPropertyHandle>& PropertyHandle, FDetailWidgetRow& HeaderRow)
{
	HeaderRow.NameContent()
	[
		PropertyHandle->CreatePropertyNameWidget()
	];

	HeaderRow.ValueContent()
	[
		GetChildPropertiesHorizontal(PropertyHandle)
	];
}

TSharedRef<SHorizontalBox> EditorHelpers::GetChildPropertiesHorizontal(TSharedRef<IPropertyHandle> ParentPropertyHandle)
{
	TSharedRef<SHorizontalBox> HorizontalBox = SNew(SHorizontalBox);

	uint32 NumChildren;
	ParentPropertyHandle->GetNumChildren(NumChildren);

	for (uint32 Index = 0; Index < NumChildren; ++Index)
	{
		int FrontPadding = Index == 0 ? 0 : 2;
		int BackPadding = Index == NumChildren - 1 ? 0 : 2;
		TSharedPtr<IPropertyHandle> ChildHandle = ParentPropertyHandle->GetChildHandle(Index);
		if (ChildHandle.IsValid())
		{
			if (ChildHandle->HasMetaData(NoAutoSizeTag))
			{
				HorizontalBox->AddSlot()
				[
					ChildHandle->CreatePropertyValueWidget()
				]
				.Padding(FrontPadding, 0, BackPadding, 0);
			}
			else
			{
				HorizontalBox->AddSlot()
				.AutoWidth()
				[
					ChildHandle->CreatePropertyValueWidget()
				]
				.Padding(FrontPadding, 0, BackPadding, 0);
			}
		}
	}

	return HorizontalBox;
}

TSharedRef<SHorizontalBox> EditorHelpers::GetPropertiesHorizontal(TArray<TSharedRef<IPropertyHandle>> ParentPropertyHandleArray)
{
	TSharedRef<SHorizontalBox> HorizontalBox = SNew(SHorizontalBox);

	uint32 NumChildren = ParentPropertyHandleArray.Num();
	for (uint32 Index = 0; Index < NumChildren; ++Index)
	{
		TSharedPtr<IPropertyHandle> ChildHandle = ParentPropertyHandleArray[Index];
		if (ChildHandle.IsValid())
		{
			if (ChildHandle->HasMetaData(NoAutoSizeTag))
			{
				HorizontalBox->AddSlot()
				[
					ChildHandle->CreatePropertyValueWidget()
				];
			}
			else
			{
				HorizontalBox->AddSlot()
				.AutoWidth()
				[
					ChildHandle->CreatePropertyValueWidget()
				];
			}
		}
	}

	return HorizontalBox;
}

TSharedRef<SHorizontalBox> EditorHelpers::GetWidgetsHorizontal(const TArray<TSharedRef<SWidget>>& WidgetsArray, bool AutoSize)
{
	TSharedRef<SHorizontalBox> HorizontalBox = SNew(SHorizontalBox);

	for (const TSharedRef<SWidget>& Widget : WidgetsArray)
	{
		if (AutoSize)
		{
			HorizontalBox->AddSlot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				Widget
			];
		}
		else
		{
			HorizontalBox->AddSlot()
			.VAlign(VAlign_Center)
			[
				Widget
			];
		}
	}

	return HorizontalBox;
}

TSharedRef<STextBlock> EditorHelpers::MakeDefaultEditorText(const FString& Text)
{
	return SNew(STextBlock)
	.Text(FText::FromString(Text))
	.Font(IDetailLayoutBuilder::GetDetailFont())
	.Justification(ETextJustify::Center);
}
