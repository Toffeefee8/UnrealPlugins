#include "ObjectWrapper/ObjectWrapperArrayEditor.h"
#include "PropertyHandle.h"
#include "DetailWidgetRow.h"
#include "EditorHelpers.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailPropertyRow.h"
#include "ObjectWrapper/ObjectWrapperArray.h"
#include "UObject/UObjectGlobals.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Text/STextBlock.h"

TSharedRef<IPropertyTypeCustomization> FObjectWrapperArrayEditor::MakeInstance()
{
    return MakeShareable(new FObjectWrapperArrayEditor);
}

void FObjectWrapperArrayEditor::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
    CachedPropertyHandle = PropertyHandle;
    
    HeaderRow.NameContent()
    [
        PropertyHandle->CreatePropertyNameWidget()
    ];
    
    bool Locked = false;
    FObjectWrapperArray* Struct;
    if (EditorHelpers::GetRawData(PropertyHandle, Struct))
        Locked = Struct->LockedShards;

    TSharedPtr<IPropertyHandle> ArrayHandle = PropertyHandle->GetChildHandle("ObjectWrappers");
    HeaderRow.ValueContent()
    [
        ArrayHandle->CreatePropertyValueWidget(!Locked)
    ];
    // if (Locked)
    // {
    //     FString HeaderText = FString::Printf(TEXT("%i Elements"), Struct->ObjectWrappers.Num());
    //     HeaderRow.ValueContent()
    //     [
    //         EditorHelpers::MakeDefaultEditorText(HeaderText)
    //     ];
    // }
    // else
    // {

    // }
}

void FObjectWrapperArrayEditor::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
    if (!PropertyHandle->IsValidHandle())
    {
        EditorHelpers::DisplayChildError(ChildBuilder);
        return;
    }
    
    FObjectWrapperArray* Struct;
    if (!EditorHelpers::GetRawData(PropertyHandle, Struct))
    {
        EditorHelpers::DisplayChildError(ChildBuilder, "Raw data could not be accessed.");
        return;
    }

    //Bind to on change
    auto ReevaluateDelegate = FSimpleDelegate::CreateSPLambda(this,[this, PropertyHandle](){ PropertyHandle->RequestRebuildChildren(); });
    TSharedRef<IPropertyHandle> ArrayHandle = PropertyHandle->GetChildHandle("ObjectWrappers", false).ToSharedRef();
    ArrayHandle->SetOnPropertyValueChanged(ReevaluateDelegate);
    ArrayHandle->SetOnChildPropertyValueChanged(ReevaluateDelegate);
    
    //Display children
    const int NumChildren = Struct->ObjectWrappers.Num();
    for (int i = 0; i < NumChildren; ++i)
    {
        TSharedPtr<IPropertyHandle> ChildRef = ArrayHandle->GetChildHandle(i);
        auto& Row = ChildBuilder.AddProperty(ChildRef.ToSharedRef());
        Row.ShowPropertyButtons(!(Struct->LockedShards || Struct->ObjectWrappers[i].LockClass));
    }
}

FObjectWrapperArrayEditor::~FObjectWrapperArrayEditor()
{
    RequiredClassCache.Empty();
}