#include "Modules/RegionModule.h"
#include "Region.h"

void URegionModule::StartModule_Implementation(URegion* OwningRegion)
{
}

void URegionModule::EndModule_Implementation()
{
}

void URegionModule::NewParent_Implementation(URegion* OldParentRegion, URegion* NewParentRegion)
{
}

void URegionModule::NewChild_Implementation(URegion* NewChild)
{
}

void URegionModule::RefreshModule_Implementation()
{
}

URegion* URegionModule::GetOwningRegion() const
{
	return GetTypedOuter<URegion>();
}

FGameplayTag URegionModule::GetOwningRegionTag() const
{
	if (URegion* Region = GetOwningRegion())
		return Region->GetRegionTag();

	return {};
}

URegion* URegionModule::GetParentRegion() const
{
	if (auto OwningRegion = GetOwningRegion())
	{
		return OwningRegion->GetParentRegion();
	}
	return nullptr;
}

TSet<URegion*> URegionModule::GetChildRegions() const
{
	if (auto OwningRegion = GetOwningRegion())
	{
		return OwningRegion->GetChildRegions();
	}
	return {};
}

bool URegionModule::HasParentRegion() const
{
	return GetParentRegion() != nullptr;
}

bool URegionModule::HasRegionModule(TSubclassOf<URegionModule> ModuleClass, bool AllowParentSearch) const
{
	return GetRegionModule(ModuleClass, AllowParentSearch) != nullptr;
}

bool URegionModule::HasParentRegionModule(TSubclassOf<URegionModule> ModuleClass, bool AllowParentSearch) const
{
	return GetParentRegionModule(ModuleClass, AllowParentSearch) != nullptr;
}

URegionModule* URegionModule::GetRegionModule(TSubclassOf<URegionModule> ModuleClass, bool AllowParentSearch) const
{
	if (auto OwningRegion = GetOwningRegion())
	{
		return OwningRegion->GetRegionModule(ModuleClass, AllowParentSearch);
	}
	return nullptr;
}

URegionModule* URegionModule::GetParentRegionModule(TSubclassOf<URegionModule> ModuleClass, bool AllowParentSearch) const
{
	if (auto OwningRegion = GetParentRegion())
	{
		return OwningRegion->GetRegionModule(ModuleClass, AllowParentSearch);
	}
	return nullptr;
}
