#include "ObjectWrapper/ObjectWrapperEditor.h"
#include "PropertyHandle.h"
#include "DetailWidgetRow.h"
#include "EditorHelpers.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailPropertyRow.h"
#include "ObjectWrapper/ObjectWrapper.h"
#include "UObject/UObjectGlobals.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Text/STextBlock.h"

const FName FObjectWrapperEditor::FullRefreshMetaTag(TEXT("FullRefresh"));

TSharedRef<IPropertyTypeCustomization> FObjectWrapperEditor::MakeInstance()
{
    return MakeShareable(new FObjectWrapperEditor);
}

void FObjectWrapperEditor::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
    CachedPropertyHandle = PropertyHandle.ToWeakPtr();
    
    bool bLocked = false;
    FString ClassName = "NULL";
    FObjectWrapper* Struct = nullptr;
    if (EditorHelpers::GetRawData(PropertyHandle, Struct))
    {
        bLocked = Struct->LockClass;
        ClassName = Struct->GetSelectedName();
    }

    HeaderRow.NameContent()
    [
        EditorHelpers::MakeDefaultEditorText(ClassName)
    ];

    auto ClassSelection = PropertyHandle->GetChildHandle("SelectedClass", false);
    
    if (!ClassSelection.IsValid() || !ClassSelection->IsValidHandle())
    {
        UE_LOG(ObjectWrapperEditorLog, Warning, TEXT("ClassSelection handle is invalid."));
        return;
    }

    if (!bLocked)
    {
        HeaderRow.ValueContent()
        [
            ClassSelection->CreatePropertyValueWidget(!bLocked)
        ];
        ClassSelection->SetOnPropertyValueChanged(FSimpleDelegate::CreateSPLambda(this,[this](){ ReevaluateClassSelection(); }));
    }
    else
    {
        // uint32 NumChildren = 0;
        // PropertyHandle->GetNumChildren(NumChildren);
        // FString ElementNumString = FString::Printf(TEXT("%i Elements"), NumChildren);
        // HeaderRow.ValueContent()
        // [
        //     EditorHelpers::MakeDefaultEditorText(ElementNumString)
        // ];
    }
}

void FObjectWrapperEditor::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
    if (!PropertyHandle->IsValidHandle())
    {
        EditorHelpers::DisplayChildError(ChildBuilder, "PropertyHandle is not valid.");
        return;
    }

    FObjectWrapper* Struct = nullptr;
    if (!EditorHelpers::GetRawData(PropertyHandle, Struct))
    {
        EditorHelpers::DisplayChildError(ChildBuilder, "Raw Data could not be accessed.");
        return;
    }

    UClass* SelectedClass = Struct->GetSelectedClass();
    if (!SelectedClass)
    {
        EditorHelpers::DisplayChildError(ChildBuilder, "Class selection is invalid.");
        return;
    }

    if (HasPropertyTag(PropertyHandle->GetProperty()))
    {
        if (CheckForInfiniteLoop(PropertyHandle, SelectedClass))
        {
            EditorHelpers::DisplayChildError(ChildBuilder, "Cannot select editing object if Instance Editable!");
            return;
        }
    }
	
	TArray<UObject*> OuterArray;
    PropertyHandle->GetOuterObjects(OuterArray);
    UObject* Outer = OuterArray.Num() <= 0 ? GetTransientPackage() : OuterArray[0];
    
    UE_LOG(ObjectWrapperEditorLog, Log, TEXT("Selected Class: %s --- Saved Class: %s"), *SelectedClass->GetName(), *Struct->ObjectData.GetClassName());
    UE_LOG(ObjectWrapperEditorLog, Log, TEXT("Choosing Outer: %s --- Num Outers Found: %i"), *Outer->GetName(), OuterArray.Num());
    
    bool RequiresSave = !Struct->IsDataSet();
    UObject* Instance = Struct->GetObject(Outer);
    CreatedUObject = Instance;
    
    if (RequiresSave)
    {
        UE_LOG(ObjectWrapperEditorLog, Log, TEXT("New Data has been created. Commencing capture."))
        CaptureData();
    }
    else
    {
        UE_LOG(ObjectWrapperEditorLog, Log, TEXT("Data has been loaded. No need for capture."))
    }
	
    int NonTaggedChildrenCount = 0;
    int TaggedChildrenCount = 0;
    for (TFieldIterator<FProperty> FieldIterator(SelectedClass); FieldIterator; ++FieldIterator)
    {
        if (HasPropertyTag(*FieldIterator))
        {
            FString PropertyName = FieldIterator->GetName();
            TaggedChildrenCount++;
            
            IDetailPropertyRow* Row = ChildBuilder.AddExternalObjectProperty(TArray{Instance}, FName(*PropertyName));
            PropertyRows.Add(Row);

            bool FullRefresh = (*FieldIterator)->HasMetaData(FullRefreshMetaTag);
            FSimpleDelegate Delegate = FSimpleDelegate::CreateSPLambda(this,
                [this, FullRefresh]()
                {
                    CaptureData();
                    if (FullRefresh)
                        if (CachedPropertyHandle.IsValid())
                            CachedPropertyHandle.Pin()->NotifyPostChange(EPropertyChangeType::Unspecified);
                });
            Row->GetPropertyHandle()->SetOnPropertyValueChanged(Delegate);
            Row->GetPropertyHandle()->SetOnChildPropertyValueChanged(Delegate);
        }
        else
        {
            NonTaggedChildrenCount++;
        }
    }

    if (TaggedChildrenCount <= 0)
        EditorHelpers::DisplayEmptyChildren(ChildBuilder);
    
    UE_LOG(ObjectWrapperEditorLog, Log, TEXT("Properties with Flag: %i"), TaggedChildrenCount);
    UE_LOG(ObjectWrapperEditorLog, Log, TEXT("Properties without Flag: %i"), NonTaggedChildrenCount);
}

FObjectWrapperEditor::~FObjectWrapperEditor()
{
    PropertyRows.Empty();

    if (CreatedUObject.IsValid())
        CreatedUObject.Get()->MarkAsGarbage();
}

bool FObjectWrapperEditor::CheckForInfiniteLoop(const TSharedPtr<IPropertyHandle>& PropertyHandle,
    const UClass* SelectedClass)
{
    TArray<UObject*> OuterObjects;
    PropertyHandle->GetOuterObjects(OuterObjects);
    if (const UClass* BaseClass = PropertyHandle->GetOuterBaseClass())
        OuterObjects.Add(BaseClass->GetDefaultObject());

    for (UObject* Object : OuterObjects)
    {
        if (!Object)
            continue;

        if (Object->GetOutermostObject()->GetClass() == SelectedClass)
            return true;
        
        UClass* OuterClass = Object->GetClass();
        if (OuterClass && OuterClass == SelectedClass)
        {
            for (TFieldIterator<FProperty> PropertyIt(OuterClass); PropertyIt; ++PropertyIt)
            {
                FProperty* Property = *PropertyIt;
                if (HasPropertyTag(Property))
                {
                    if (FStructProperty* StructProp = CastField<FStructProperty>(Property))
                    {
                        if (StructProp->Struct->IsChildOf(FObjectWrapper::StaticStruct()))
                        {
                            void* StructValue = StructProp->ContainerPtrToValuePtr<void>(OuterClass->GetDefaultObject());
                            UClass* DefaultClass = nullptr;
                            
                            if (StructValue && StructProp->Struct->IsChildOf(FObjectWrapper::StaticStruct()))
                            {
                                DefaultClass = *static_cast<UClass**>(StructValue);
                            }
                            
                            if (DefaultClass == SelectedClass)
                            {
                                return true;  
                            }
                        }
                    }
                }
            }
        }
    }

    return false;
}

bool FObjectWrapperEditor::HasPropertyTag(const FProperty* Property)
{
    if (!Property)
        return false;
    
    if (!Property->HasAllPropertyFlags(RequiredPropertyFlags))
        return false;
    
    if (Property->HasAnyPropertyFlags(BlockingPropertyFlags))
        return false;

    return true;
}

void FObjectWrapperEditor::ReevaluateClassSelection()
{
    if (!CachedPropertyHandle.IsValid() || !CachedPropertyHandle.Pin().IsValid())
        return;

    UE_LOG(ObjectWrapperEditorLog, Log, TEXT("On change selected class."))

    TSharedPtr<IPropertyHandle> PropertyHandle = CachedPropertyHandle.Pin();
    FObjectWrapper* Struct;
    if (!EditorHelpers::GetRawData(PropertyHandle, Struct))
        return;
    
    Struct->ObjectData.Clear();
    PropertyHandle->RequestRebuildChildren();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void FObjectWrapperEditor::CaptureData()
{
    TSharedPtr<IPropertyHandle> CachedHandle = CachedPropertyHandle.Pin();
    if (!CachedHandle.IsValid() || !CachedHandle->IsValidHandle())
    {
        UE_LOG(ObjectWrapperEditorLog, Warning, TEXT("CachedPropertyHandle is not valid on data capture."));
        return;
    }

    UE_LOG(ObjectWrapperEditorLog, Log, TEXT("Capturing Data: %s"), *CachedHandle->GetProperty()->GetName())
    
    FObjectWrapper* Struct = nullptr;
    if (EditorHelpers::GetRawData(CachedHandle, Struct))
    {
        if (UObjectSerializationLibrary::CaptureObject(Struct->ObjectData, CreatedUObject.Get()))
            UE_LOG(ObjectWrapperEditorLog, Log, TEXT("Data Capture Success"))
        else
            UE_LOG(ObjectWrapperEditorLog, Warning, TEXT("Data Capture Failed On Serialization."))
    }
    else
    {
        UE_LOG(ObjectWrapperEditorLog, Warning, TEXT("Data Capture Failed."))
    }
}