#include "Modules/Implementations/Electricity/Provider/PowerProviderComponent.h"

#include "Modules/Implementations/Electricity/ElectricityModule.h"
#include "ReplicatedObject/GlobalReplicator.h"

void UPowerProviderComponent::GetPowerProviderData_Implementation(FPowerProviderData& OutProvisionData)
{
	OutProvisionData.PowerProvision = ProvidedPower;
	OutProvisionData.TotalPowerProvision = ProvidedPower;
	OutProvisionData.bEnabled = bPowerProvisionEnabled;
}

void UPowerProviderComponent::ForceSetRegion_Implementation(FGameplayTag NewRegion)
{
	RegionTag = NewRegion;
}

void UPowerProviderComponent::GetCheckData_Implementation(FVector& CheckLocation, ERegionTypes& DesiredType, bool& bOutDisableRuntimeChecks) const
{
	CheckLocation = GetOwner()->GetActorLocation();
	DesiredType = ERegionTypes::Room;
	bOutDisableRuntimeChecks = bDisableRuntimeChecks;
}

FGameplayTag UPowerProviderComponent::GetRegionTag_Implementation() const
{
	return RegionTag;
}

bool UPowerProviderComponent::IsProvidingPower() const
{
	return bPowerProvisionEnabled;
}

void UPowerProviderComponent::TurnOn()
{
	if (bPowerProvisionEnabled)
		return;

	bPowerProvisionEnabled = bReplicatedProvisionState = true;

	UElectricityModule::TryRefreshPowerProviderData(this, RegionTag);
}

void UPowerProviderComponent::TurnOff()
{
	if (!bPowerProvisionEnabled)
		return;

	bPowerProvisionEnabled = bReplicatedProvisionState = false;

	UElectricityModule::TryRefreshPowerProviderData(this, RegionTag);
}

void UPowerProviderComponent::ChangePowerProvision(float NewPowerProvision)
{
	if (ProvidedPower == NewPowerProvision)
		return;
	
	float OldProvision = ProvidedPower;
	ProvidedPower = ReplicatedProvidedPower = NewPowerProvision;

	OnConsumptionChange.Broadcast(this, OldProvision, NewPowerProvision);

	UElectricityModule::TryRefreshPowerProviderData(this, RegionTag);
}

void UPowerProviderComponent::BeginPlay()
{
	Super::BeginPlay();

	bPowerProvisionEnabled = bReplicatedProvisionState = bStartEnabled;
	ReplicatedProvidedPower = ProvidedPower;
	
	WaitForGlobalReplicator();
}

void UPowerProviderComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	UGlobalReplicator* Replicator = UGlobalReplicator::Get(this);
	if (!Replicator)
		return;
	
	if (bReplicateState)
		Replicator->DereplicateData(UGlobalReplicator::GetUniqueIDFromObject(this, "State"));

	if (bReplicateProvision)
		Replicator->DereplicateData(UGlobalReplicator::GetUniqueIDFromObject(this, "Provision"));
}

void UPowerProviderComponent::WaitForGlobalReplicator()
{
	if (UGlobalReplicator* Replicator = UGlobalReplicator::Get(this))
	{
		InitializeReplication(Replicator);
	}
	else
	{
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UPowerProviderComponent::WaitForGlobalReplicator, 0.1f, false);
	}
}

void UPowerProviderComponent::InitializeReplication_Implementation(UGlobalReplicator* Replicator)
{
	if (bReplicateState)
	{
		FOnReplicatedValueChanged StateDelegate;
		StateDelegate.BindDynamic(this, &UPowerProviderComponent::SyncReplicatedState);
		Replicator->ReplicateBool(UGlobalReplicator::GetUniqueIDFromObject(this, "State"), bReplicatedProvisionState, StateDelegate);
	}
	if (bReplicateProvision)
	{
		FOnReplicatedValueChanged ProvisionDelegate;
		ProvisionDelegate.BindDynamic(this, &UPowerProviderComponent::SyncReplicatedProvision);
		Replicator->ReplicateFloat(UGlobalReplicator::GetUniqueIDFromObject(this, "Provision"), ReplicatedProvidedPower, ProvisionDelegate);
	}
}

void UPowerProviderComponent::SyncReplicatedState()
{
	if (bReplicatedProvisionState)
	{
		TurnOn();
	}
	else
	{
		TurnOff();
	}
}

void UPowerProviderComponent::SyncReplicatedProvision()
{
	ChangePowerProvision(ReplicatedProvidedPower);
}