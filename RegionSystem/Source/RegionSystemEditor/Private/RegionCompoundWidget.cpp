#include "RegionCompoundWidget.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "GameplayTagContainer.h"
#include "Region.h"
#include "RegionSubsystem.h"
#include "RegionTags.h"
#include "Extensions/GameplayTagExtensions.h"
#include "Widgets/Layout/SWrapBox.h"

const FName SRegionCompoundWidget::RegionTabName = "Region Visualiser";

TSharedRef<SDockTab> SRegionCompoundWidget::MakeInstance(const FSpawnTabArgs& SpawnTabArgs)
{
    return SNew(SDockTab)
        .TabRole(ETabRole::NomadTab)
        [
            SNew(SRegionCompoundWidget)
        ];
}

void SRegionCompoundWidget::Construct(const FArguments& InArgs)
{
    FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	
    FDetailsViewArgs DetailsViewArgs;
    DetailsViewArgs.bAllowSearch = false;
    DetailsViewArgs.bForceHiddenPropertyVisibility = false;
    DetailsViewArgs.bHideSelectionTip = true;
    DetailsViewArgs.bLockable = false;
    DetailsViewArgs.bUpdatesFromSelection = false;
    DetailsViewArgs.bShowPropertyMatrixButton = false;
    DetailsViewArgs.ViewIdentifier = "RegionEditor";
	
    DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
    
    PopulateRegions();
    ShowRegions();

    ChildSlot
    [
        SNew(SVerticalBox)

        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(10)
        [
            SNew(SHorizontalBox)

            + SHorizontalBox::Slot()
            .AutoWidth()
            .HAlign(HAlign_Center)
            [
                SNew(SButton)
                .Text(FText::FromString("Refresh"))
                .OnClicked(this, &SRegionCompoundWidget::RefreshRegions)
            ]
            
            // ComboBox
            + SHorizontalBox::Slot()
            .AutoWidth()
            .FillWidth(1)
            .Padding(10, 0)
            [
                SAssignNew(RegionComboBox, SComboBox<TSharedPtr<FGameplayTag>>)
                .OptionsSource(&RegionTags)
                .OnGenerateWidget(this, &SRegionCompoundWidget::MakeRegionComboWidget)
                .OnSelectionChanged(this, &SRegionCompoundWidget::OnRegionSelected)
                [
                    SNew(STextBlock)
                    .Text_Lambda([this]() -> FText {
                        return SelectedRegionTag.IsValid() ?
                            FText::FromName(SelectedRegionTag->GetTagName()) : 
                            FText::FromString("Select Region");
                    })
                ]
            ]
            
            + SHorizontalBox::Slot()
            .AutoWidth()
            .HAlign(HAlign_Center)
            [
                SNew(SButton)
                .Text(FText::FromString("Parent"))
                .OnClicked(this, &SRegionCompoundWidget::SelectParentRegion)
            ]
        ]

        // Buttons (Highlight, Hide, Show, Bake)
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(10, 5)
        [
            SNew(SWrapBox)
            .UseAllottedSize(true)
            .InnerSlotPadding(FVector2D(10, 5))
            + SWrapBox::Slot()
            [
                SNew(SButton)
                .Text(FText::FromString("Select Region"))
                .OnClicked(this, &SRegionCompoundWidget::SelectVolumes)
            ]
            + SWrapBox::Slot()
            [
                SNew(SButton)
                .Text(FText::FromString("Highlight Region"))
                .OnClicked(this, &SRegionCompoundWidget::HighlightRegion)
            ]
            + SWrapBox::Slot()
            [
                SNew(SButton)
                .Text(FText::FromString("Hide Regions"))
                .OnClicked(this, &SRegionCompoundWidget::HideRegions)
            ]
            + SWrapBox::Slot()
            [
                SNew(SButton)
                .Text(FText::FromString("Show Regions"))
                .OnClicked(this, &SRegionCompoundWidget::ShowRegions)
            ]
            + SWrapBox::Slot()
            [
                SNew(SButton)
                .Text(FText::FromString("Bake Regions"))
                .OnClicked(this, &SRegionCompoundWidget::BakeRegions)
            ]
        ]
        
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(10, 5)
        [
            SNew(STextBlock)
            .Text_Lambda([this]() -> FText {
                return SelectedRegionTag.IsValid() ? 
                    FText::Format(FText::FromString("Editing Region: {0}"), FText::FromName(UGameplayTagExtensions::RemoveParentTagFromName(*SelectedRegionTag, RegionTags::Areas::Name))) :
                    FText::FromString("Select a region to edit.");
            })
        ]
        
        + SVerticalBox::Slot()
        .FillHeight(1.0f)
        [
            DetailsView.ToSharedRef()
        ]
    ];
}

void SRegionCompoundWidget::PopulateRegions()
{
    if (!GetRegionSubsystem())
        return;

    RegionTags.Empty();
    FGameplayTagContainer AllRegions = GetRegionSubsystem()->GetAllRegionTags();

    for (const FGameplayTag& RegionTag : AllRegions)
    {
        RegionTags.Add(MakeShared<FGameplayTag>(RegionTag));
    }

    if (RegionComboBox.IsValid())
    {
        RegionComboBox->RefreshOptions();
    }
}

void SRegionCompoundWidget::OnRegionSelected(TSharedPtr<FGameplayTag> SelectedTag, ESelectInfo::Type SelectInfo)
{
    if (SelectedTag.IsValid())
    {
        GEditor->SelectNone(true, true);
        SelectedRegionTag = SelectedTag;
        
        if (GetRegionSubsystem())
        {
            if (URegion* Region = GetRegionSubsystem()->GetRegionByTag(*SelectedTag))
            {
                DetailsView->SetObject(Region);
                HighlightRegion();
            }
        }
    }
}

TSharedRef<SWidget> SRegionCompoundWidget::MakeRegionComboWidget(TSharedPtr<FGameplayTag> InTag) const
{
    return SNew(STextBlock).Text(FText::FromName(InTag->GetTagName()));
}

URegionSubsystem* SRegionCompoundWidget::GetRegionSubsystem() const
{
    UWorld* World = nullptr;

    if (GEditor)
    {
        World = GEditor->GetEditorWorldContext().World();
    }

    if (IsPIEActive())
    {
        World = GetPIEWorldContext().World();
    }

    return World ? World->GetSubsystem<URegionSubsystem>() : nullptr;
}

bool SRegionCompoundWidget::IsPIEActive() const
{
    return IsValid(GetPIEWorldContext().World());
}

FWorldContext SRegionCompoundWidget::GetPIEWorldContext() const
{
    for (const FWorldContext& Context : GEngine->GetWorldContexts())
    {
        if (Context.World() && (Context.WorldType == EWorldType::PIE || Context.WorldType == EWorldType::Game))
        {
            return Context;
        }
    }
    return FWorldContext();
}

FReply SRegionCompoundWidget::HideRegions()
{
    if (GetRegionSubsystem())
    {
        GetRegionSubsystem()->HideAllRegions();
    }
    return FReply::Handled();
}

FReply SRegionCompoundWidget::ShowRegions()
{
    if (GetRegionSubsystem())
    {
        GetRegionSubsystem()->ShowAllRegions();
    }
    return FReply::Handled();
}

FReply SRegionCompoundWidget::HighlightRegion()
{
    if (GetRegionSubsystem())
    {
        if (SelectedRegionTag)
        {
            GetRegionSubsystem()->HighlightRegion(*SelectedRegionTag, false);
        }
    }
    return FReply::Handled();
}

FReply SRegionCompoundWidget::SelectVolumes()
{
    if (GetRegionSubsystem() && SelectedRegionTag)
    {
        if (URegion* Region = GetRegionSubsystem()->GetRegionByTag(*SelectedRegionTag))
        {
            GEditor->SelectNone(false, true);
            
            for (auto Volume : Region->GetRegionVolumes())
            {
                GEditor->SelectActor(Volume, true, true, true);
            }
        }
    }

    return FReply::Handled();
}

FReply SRegionCompoundWidget::RefreshRegions()
{
    if (!IsPIEActive())
        GetRegionSubsystem()->ForceRefreshRegions();
        
    PopulateRegions();
    
    SelectedRegionTag = nullptr;
    
    if (RegionComboBox.IsValid())
    {
        RegionComboBox->ClearSelection();
    }
    
    if (DetailsView.IsValid())
    {
        DetailsView->SetObject(nullptr);
    }

    ShowRegions();

    UE_LOG(LogTemp, Log, TEXT("Refreshing Region List"));
    return FReply::Handled();
}

FReply SRegionCompoundWidget::SelectParentRegion()
{
    if (GetRegionSubsystem() && SelectedRegionTag)
    {
        if (URegion* Region = GetRegionSubsystem()->GetRegionByTag(*SelectedRegionTag))
        {
            FGameplayTag ParentRegionTag = Region->GetParentRegionTag();
            if (ParentRegionTag.IsValid())
            {
                for (TSharedPtr<FGameplayTag> TagPtr : RegionTags)
                {
                    if (TagPtr.IsValid() && *TagPtr == ParentRegionTag)
                    {
                        RegionComboBox->SetSelectedItem(TagPtr);
                        OnRegionSelected(TagPtr, ESelectInfo::Direct);
                        break;
                    }
                }
            }
        }
    }
    return FReply::Handled();
}

FReply SRegionCompoundWidget::BakeRegions()
{
    if (GetRegionSubsystem())
    {
        GetRegionSubsystem()->BakeRegions();
    }
    return FReply::Handled();
}
