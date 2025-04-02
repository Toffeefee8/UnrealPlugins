#include "Modules/Implementations/Electricity/FuzeBox/FuzeBoxComponent.h"

void UFuzeBoxComponent::GetFuzeBoxData_Implementation(FFuzeBoxData& OutData)
{
	OutData.ActivationDelay = ActivationDelay;
	OutData.bIndependent = bIndependent;
}

void UFuzeBoxComponent::OnReceiveModule_Implementation(UElectricityModule* Module)
{
	if (PowerModule.IsValid())
	{
		//UNSUBSCRIBE
	}
	PowerModule = Module;

	if (bStartEnabled)
		Repair();
	else
		Break();
}

void UFuzeBoxComponent::ForceSetRegion_Implementation(FGameplayTag NewRegion)
{
	RegionTag = NewRegion;
}

void UFuzeBoxComponent::GetCheckData_Implementation(FVector& OutCheckLocation, ERegionTypes& OutDesiredType, bool& bOutDisableRuntimeChecks) const
{
	OutCheckLocation = GetOwner()->GetActorLocation();
	OutDesiredType = DesiredRegion;
	bOutDisableRuntimeChecks = bDisableRuntimeChecks;
}

FGameplayTag UFuzeBoxComponent::GetRegionTag_Implementation() const
{
	return RegionTag;
}

bool UFuzeBoxComponent::HasPowerModule() const
{
	return PowerModule.IsValid();
}

UElectricityModule* UFuzeBoxComponent::GetPowerModule() const
{
	return PowerModule.Get(); 
}

void UFuzeBoxComponent::Repair()
{
	if (PowerModule.IsValid())
	{
		PowerModule->Repair();
	}
}

void UFuzeBoxComponent::Break()
{
	if (PowerModule.IsValid())
	{
		PowerModule->Break();
	}
}
