#include "RegionPOIEditor.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"

TSharedRef<IDetailCustomization> FRegionPOIEditor::MakeInstance()
{
	return MakeShareable(new FRegionPOIEditor);
}

void FRegionPOIEditor::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& RegionCategory = DetailBuilder.EditCategory("Regions");
	IDetailCategoryBuilder& RegionVolumeCategory = DetailBuilder.EditCategory("RegionPOI");
}

