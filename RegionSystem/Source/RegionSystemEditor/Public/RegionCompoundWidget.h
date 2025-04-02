#pragma once
#include "GameplayTagContainer.h"

class URegionSubsystem;

class SRegionCompoundWidget : public SCompoundWidget
{
	
public:
	SLATE_BEGIN_ARGS(SRegionCompoundWidget) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	static const FName RegionTabName;
	static TSharedRef<SDockTab> MakeInstance(const FSpawnTabArgs& SpawnTabArgs);

private:
	void PopulateRegions();
	void OnRegionSelected(TSharedPtr<FGameplayTag> SelectedTag, ESelectInfo::Type SelectInfo);
	TSharedRef<SWidget> MakeRegionComboWidget(TSharedPtr<FGameplayTag> InTag) const;
	URegionSubsystem* GetRegionSubsystem() const;

	bool IsPIEActive() const;
	FWorldContext GetPIEWorldContext() const;
    
	FReply HideRegions();
	FReply ShowRegions();
	FReply HighlightRegion();

	FReply SelectVolumes();
	
	FReply RefreshRegions();
	FReply SelectParentRegion();
	
	FReply BakeRegions();

	TArray<TSharedPtr<FGameplayTag>> RegionTags;
	TSharedPtr<FGameplayTag> SelectedRegionTag;
	TSharedPtr<SComboBox<TSharedPtr<FGameplayTag>>> RegionComboBox;
	TSharedPtr<IDetailsView> DetailsView;
};
