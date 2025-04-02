#include "Modules/Implementations/Electricity/Consumer/PowerConsumerComponent.h"

#include "Region.h"
#include "Modules/Implementations/Electricity/ElectricityModule.h"
#include "ReplicatedObject/GlobalReplicator.h"

void UPowerConsumerComponent::GetPowerConsumptionData_Implementation(TArray<FPowerConsumerData>& OutConsumptionData) const
{
	FPowerConsumerData PowerConsumptionData {};
	PowerConsumptionData.bEnabled = bPowerConsumptionDesired;
	PowerConsumptionData.ConsumerType = ConsumerType;
	PowerConsumptionData.PowerConsumption = PowerConsumption;
	PowerConsumptionData.TotalPowerConsumption = PowerConsumption;

	OutConsumptionData.Add(PowerConsumptionData);
}

void UPowerConsumerComponent::OnGainPower_Implementation(EElectricityConsumerType InConsumerType)
{
	//Since it only subscribes one consumer type there is no need to check the consumer type here
	bHasPower = true;
	OnGainPower.Broadcast(this);
}

void UPowerConsumerComponent::OnLosePower_Implementation(EElectricityConsumerType InConsumerType)
{
	//Since it only subscribes one consumer type there is no need to check the consumer type here
	bHasPower = false;
	OnLosePower.Broadcast(this);
}

void UPowerConsumerComponent::ForceSetRegion_Implementation(FGameplayTag NewRegion)
{
	UE_LOG(LogTemp, Log, TEXT("SetRegionTag: %s"), *NewRegion.ToString());
	RegionTag = NewRegion;
}

void UPowerConsumerComponent::GetCheckData_Implementation(FVector& CheckLocation, ERegionTypes& DesiredType, bool& bOutDisableRuntimeChecks) const
{
	CheckLocation = GetOwner()->GetActorLocation();
	DesiredType = ERegionTypes::Room;
	bOutDisableRuntimeChecks = bDisableRuntimeChecks;
}

FGameplayTag UPowerConsumerComponent::GetRegionTag_Implementation() const
{
	return RegionTag;
}

bool UPowerConsumerComponent::WantsPower() const
{
	return bPowerConsumptionDesired;
}

bool UPowerConsumerComponent::HasPower() const
{
	return bHasPower;
}

void UPowerConsumerComponent::BeginPlay()
{
	Super::BeginPlay();

	bPowerConsumptionDesired = bReplicatedConsumptionState = bStartEnabled;
	ReplicatedConsumption = PowerConsumption;
	
	WaitForGlobalReplicator();
}

void UPowerConsumerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	UGlobalReplicator* Replicator = UGlobalReplicator::Get(this);
	if (!Replicator)
		return;
	
	if (bReplicateState)
		Replicator->DereplicateData(UGlobalReplicator::GetUniqueIDFromObject(this, "State"));

	if (bReplicateConsumption)
		Replicator->DereplicateData(UGlobalReplicator::GetUniqueIDFromObject(this, "Consumption"));
}

void UPowerConsumerComponent::WaitForGlobalReplicator()
{
	if (UGlobalReplicator* Replicator = UGlobalReplicator::Get(this))
	{
		InitializeReplication(Replicator);
	}
	else
	{
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UPowerConsumerComponent::WaitForGlobalReplicator, 0.1f, false);
	}
}

void UPowerConsumerComponent::InitializeReplication_Implementation(UGlobalReplicator* Replicator)
{
	if (bReplicateState)
	{
		FOnReplicatedValueChanged StateDelegate;
		StateDelegate.BindDynamic(this, &UPowerConsumerComponent::SyncReplicatedState);
		Replicator->ReplicateBool(UGlobalReplicator::GetUniqueIDFromObject(this, "State"), bReplicatedConsumptionState, StateDelegate);
	}

	if (bReplicateConsumption)
	{
		FOnReplicatedValueChanged ConsumptionDelegate;
		ConsumptionDelegate.BindDynamic(this, &UPowerConsumerComponent::SyncReplicatedConsumption);
		Replicator->ReplicateFloat(UGlobalReplicator::GetUniqueIDFromObject(this, "Consumption"), ReplicatedConsumption, ConsumptionDelegate);
	}
}

void UPowerConsumerComponent::SyncReplicatedState()
{
	if (bReplicatedConsumptionState)
	{
		TurnOn();
	}
	else
	{
		TurnOff();
	}
}

void UPowerConsumerComponent::SyncReplicatedConsumption()
{
	ChangePowerConsumption(ReplicatedConsumption);
}

void UPowerConsumerComponent::TurnOn()
{
	if (bPowerConsumptionDesired)
		return;

	bPowerConsumptionDesired = bReplicatedConsumptionState = true;

	UElectricityModule::TryReevaluatePowerConsumers(this, RegionTag, ConsumerType);
}

void UPowerConsumerComponent::TurnOff()
{
	if (!bPowerConsumptionDesired)
		return;

	bPowerConsumptionDesired = bReplicatedConsumptionState = false;

	UElectricityModule::TryReevaluatePowerConsumers(this, RegionTag, ConsumerType);
}

void UPowerConsumerComponent::ChangePowerConsumption(float NewPowerConsumption)
{
	if (PowerConsumption == NewPowerConsumption)
		return;
	
	float OldConsumption = PowerConsumption;
	PowerConsumption = ReplicatedConsumption = NewPowerConsumption;

	OnConsumptionChange.Broadcast(this, OldConsumption, NewPowerConsumption);

	UElectricityModule::TryReevaluatePowerConsumers(this, RegionTag, ConsumerType);
}

void UPowerConsumerComponent::ChangePowerType(EElectricityConsumerType NewPowerType)
{
	EElectricityConsumerType OldConsumptionType = ConsumerType;
	ConsumerType = NewPowerType;

	OnConsumptionTypeChange.Broadcast(this, OldConsumptionType, NewPowerType);

	UElectricityModule::TryRefreshConsumerData(this, RegionTag, this);
}