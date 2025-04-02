#include "RegionVolumeEditor.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"

TSharedRef<IDetailCustomization> FRegionVolumeEditor::MakeInstance()
{
	return MakeShareable(new FRegionVolumeEditor);
}

void FRegionVolumeEditor::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& RegionCategory = DetailBuilder.EditCategory("Regions");
	IDetailCategoryBuilder& RegionVolumeCategory = DetailBuilder.EditCategory("RegionVolume");
}

