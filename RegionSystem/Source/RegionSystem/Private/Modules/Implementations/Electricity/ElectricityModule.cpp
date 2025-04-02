#include "Modules/Implementations/Electricity/ElectricityModule.h"

#include "DebugFunctionLibrary.h"
#include "Modules/Implementations/Electricity/Provider/ElectricityProviderInterface.h"
#include "Modules/Implementations/Electricity/Consumer/ElectricityConsumerInterface.h"
#include "GameplayTagContainer.h"
#include "Region.h"
#include "RegionObjectInterface.h"
#include "RegionSystem.h"
#include "RegionTags.h"
#include "Modules/Implementations/Electricity/FuzeBox/FuzeBoxInterface.h"
#include "ReplicatedObject/GlobalReplicator.h"
#include "Settings/RegionSettings.h"
#include "Templates/SharedPointer.h"

#define DEBUG_ELECTRICITY_MODULE(LogType, fmt, ...) \
DEBUG_SIMPLE(LogRegions, LogType, FColor::White, FString::Printf(TEXT("[%s] "), *GetOwningRegionTag().GetTagName().ToString()) + FString::Printf(TEXT(fmt), ##__VA_ARGS__),  RegionTags::Modules::Electricity::Name);

void UElectricityModule::TryReevaluatePowerConsumers(UObject* WorldContextObject, FGameplayTag RegionTag, EElectricityConsumerType ConsumerType)
{
    if (URegionSubsystem* Subsystem = URegionSubsystem::Get(WorldContextObject))
    {
        if (URegion* Region = Subsystem->GetRegionByTag(RegionTag))
        {
            if (UElectricityModule* Module = Region->GetRegionModule<UElectricityModule>())
            {
                Module->ReevaluatePowerConsumers(ConsumerType);
            }
        }
    }
}

void UElectricityModule::TryRefreshConsumerData(UObject* WorldContextObject, FGameplayTag RegionTag, UObject* Consumer)
{
    if (URegionSubsystem* Subsystem = URegionSubsystem::Get(WorldContextObject))
    {
        if (URegion* Region = Subsystem->GetRegionByTag(RegionTag))
        {
            if (UElectricityModule* Module = Region->GetRegionModule<UElectricityModule>())
            {
                Module->RefreshConsumerData(Consumer);
            }
        }
    }
}

void UElectricityModule::TryRefreshPowerProviderData(UObject* WorldContextObject, FGameplayTag RegionTag)
{
    if (URegionSubsystem* Subsystem = URegionSubsystem::Get(WorldContextObject))
    {
        if (URegion* Region = Subsystem->GetRegionByTag(RegionTag))
        {
            if (UElectricityModule* Module = Region->GetRegionModule<UElectricityModule>())
            {
                Module->RefreshPowerProviderData();
            }
        }
    }
}

void UElectricityModule::GetPowerConsumptionData_Implementation(TArray<FPowerConsumerData>& OutConsumptionData) const
{
    if (IsIndependent())
    {
        OutConsumptionData.Empty();
        return;
    }
    
    for (auto Data : ConsumerDataByType)
    {
        FPowerConsumerData PCD {};
        PCD.bEnabled = true; // True to always receive calls
        PCD.PowerConsumption = GetPowerConsumptionOfType(Data.Key);
        PCD.TotalPowerConsumption = GetTotalPowerConsumptionOfType(Data.Key);
        PCD.ConsumerType = Data.Key;
        
        OutConsumptionData.Add(PCD);
    }
}

void UElectricityModule::OnGainPower_Implementation(EElectricityConsumerType ConsumerType)
{
    DEBUG_ELECTRICITY_MODULE(Log, "OnGainPower called for ConsumerType: %s", *UEnum::GetValueAsString(ConsumerType));
    
    if (IsIndependent())
    {
        DEBUG_ELECTRICITY_MODULE(Log, "OnGainPower called, but Module is Independent. Returning.");
        return;
    }

    if (HasActivationDelay() && IsRepaired() && GetWorld())
    {
        FTimerManager& TimerManager = GetWorld()->GetTimerManager();
        TimerManager.ClearTimer(FuzeBoxData.DeactivationTimerHandle);

        DEBUG_ELECTRICITY_MODULE(Log, "OnGainPower: delaying execution for %f seconds", GetActivationDelay());
        
        auto Callback = [this, ConsumerType]()
        {
            if (IsValid(this))
            {
                ActivateType(ConsumerType);
                FuzeBoxData.ActivationTimerHandle.Invalidate();
            }
        };
        TimerManager.SetTimer(FuzeBoxData.ActivationTimerHandle, Callback, GetActivationDelay(), false);
    }
    else
    {
        ActivateType(ConsumerType);
    }
}

void UElectricityModule::OnLosePower_Implementation(EElectricityConsumerType ConsumerType)
{
    DEBUG_ELECTRICITY_MODULE(Log, "OnLosePower called for ConsumerType: %s", *UEnum::GetValueAsString(ConsumerType));
    
    if (IsIndependent())
    {
        DEBUG_ELECTRICITY_MODULE(Log, "OnLosePower: Module is Independent. Returning.");
        return;
    }
    
    if (HasActivationDelay() && IsRepaired() && GetWorld())
    {
        FTimerManager& TimerManager = GetWorld()->GetTimerManager();
        TimerManager.ClearTimer(FuzeBoxData.ActivationTimerHandle);

        DEBUG_ELECTRICITY_MODULE(Log, "OnLosePower: delaying execution for %f seconds", GetActivationDelay());

        auto Callback = [this, ConsumerType]()
        {
            if (IsValid(this))
            {
                DeactivateType(ConsumerType);
                FuzeBoxData.DeactivationTimerHandle.Invalidate();
            }
        };
        TimerManager.SetTimer(FuzeBoxData.DeactivationTimerHandle, Callback, GetActivationDelay(), false);
    }
    else
    {
        DeactivateType(ConsumerType);
    }
}

void UElectricityModule::GetPowerProviderData_Implementation(FPowerProviderData& OutProvisionData)
{
    if (IsIndependent())
    {
        OutProvisionData = {};
        return;
    }
    
    OutProvisionData.PowerProvision = GetPowerProvisions();
    OutProvisionData.TotalPowerProvision = GetTotalPowerProvisions();
    OutProvisionData.bEnabled = IsProvidingPower();
}

void UElectricityModule::StartModule_Implementation(URegion* OwningRegion)
{
    DEBUG_ELECTRICITY_MODULE(Log, "StartModule");
    
    //Set initial state
    SetupBundledConsumerDatas();
    bFuze = URegionSettings::GetDefaultModuleFuzeState();
    RefreshFuzeData();
    
    if (UElectricityModule* Module = GetParentRegionModule<UElectricityModule>())
    {
        DEBUG_ELECTRICITY_MODULE(Log, "StartModule: Parent ElectricityModule found. Registering as Power Consumer and Provider.");
        Module->RegisterPowerConsumer(this);
        Module->RegisterPowerProvider(this);
        PreviousParentModule = Module;
    }
    else
    {
        DEBUG_ELECTRICITY_MODULE(Log, "StartModule: No parent ElectricityModule found.");
    }

    TryInitializeReplication();
}

void UElectricityModule::EndModule_Implementation()
{
    Super::EndModule_Implementation();

    DEBUG_ELECTRICITY_MODULE(Log, "EndModule");
    
    if (UGlobalReplicator* Replicator = UGlobalReplicator::Get(this))
    {
        Replicator->DereplicateData(GetReplicationKey());
    }

    if (const UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(FuzeBoxData.ActivationTimerHandle);
        World->GetTimerManager().ClearTimer(FuzeBoxData.DeactivationTimerHandle);
    }
}

void UElectricityModule::RefreshModule_Implementation()
{
    DEBUG_ELECTRICITY_MODULE(Log, "RefreshModule");
    
    if (URegionSubsystem* RegionSubsystem = URegionSubsystem::Get(this))
    {
        for (auto RegionVolume : GetOwningRegion()->GetRegionVolumes())
        {
            DEBUG_ELECTRICITY_MODULE(Log, "RefreshModule: Checking RegionVolume: %s", *RegionVolume->GetName());

            TArray<UObject*> InterfaceObjects = RegionVolume->GetOverlappingInterfaceObjects<UFuzeBoxInterface>();
            DEBUG_ELECTRICITY_MODULE(Log, "RefreshModule: Found %d overlapping FuzeBoxes.", InterfaceObjects.Num());

            for (auto Object : InterfaceObjects)
            {
                URegion* Region = GetOwningRegion();
                if (Object->Implements<URegionObject>())
                {
                    Region = RegionSubsystem->GetRegionByTag(IRegionObject::Execute_GetRegionTag(Object));
                    DEBUG_ELECTRICITY_MODULE(Log, "RefreshModule: Found RegionObject Interface. Getting Region by Tag: %s", *IRegionObject::Execute_GetRegionTag(Object).ToString());
                }

                if (!Region)
                {
                    DEBUG_ELECTRICITY_MODULE(Warning, "RefreshModule: Could not find Region for object.");
                    continue;
                }

                if (UElectricityModule* Module = Region->GetRegionModule<UElectricityModule>())
                {
                    DEBUG_ELECTRICITY_MODULE(Log, "RefreshModule: Found ElectricityModule in Region: %s", *Region->GetRegionTag().ToString());
                    Module->SetFuzeBox(Object);
                }
            }
            
            InterfaceObjects.Empty();
            InterfaceObjects = RegionVolume->GetOverlappingInterfaceObjects<UElectricityProviderInterface>();
            DEBUG_ELECTRICITY_MODULE(Log, "RefreshModule: Found %d overlapping Providers.", InterfaceObjects.Num());

            for (auto Object : InterfaceObjects)
            {
                URegion* Region = GetOwningRegion();
                if (Object->Implements<URegionObject>())
                {
                    Region = RegionSubsystem->GetRegionByTag(IRegionObject::Execute_GetRegionTag(Object));
                    DEBUG_ELECTRICITY_MODULE(Log, "RefreshModule: Found RegionObject Interface. Getting Region by Tag: %s", *IRegionObject::Execute_GetRegionTag(Object).ToString());
                }

                if (!Region)
                {
                    DEBUG_ELECTRICITY_MODULE(Warning, "RefreshModule: Could not find Region for object.");
                    continue;
                }

                if (UElectricityModule* Module = Region->GetRegionModule<UElectricityModule>())
                {
                    DEBUG_ELECTRICITY_MODULE(Log, "RefreshModule: Found ElectricityModule in Region: %s", *Region->GetRegionTag().ToString());
                    Module->RegisterPowerProvider(Object);
                }
            }

            InterfaceObjects.Empty();
            InterfaceObjects = RegionVolume->GetOverlappingInterfaceObjects<UElectricityConsumerInterface>();
            DEBUG_ELECTRICITY_MODULE(Log, "RefreshModule: Found %d overlapping Consumers.", InterfaceObjects.Num());

            for (auto Object : InterfaceObjects)
            {
                URegion* Region = GetOwningRegion();
                if (Object->Implements<URegionObject>())
                {
                    Region = RegionSubsystem->GetRegionByTag(IRegionObject::Execute_GetRegionTag(Object));
                    DEBUG_ELECTRICITY_MODULE(Log, "RefreshModule: Found RegionObject Interface. Getting Region by Tag: %s", *IRegionObject::Execute_GetRegionTag(Object).ToString());
                }

                if (!Region)
                {
                    DEBUG_ELECTRICITY_MODULE(Warning, "RefreshModule: Could not find Region for object.");
                    continue;
                }

                if (UElectricityModule* Module = Region->GetRegionModule<UElectricityModule>())
                {
                    DEBUG_ELECTRICITY_MODULE(Log, "RefreshModule: Found ElectricityModule in Region: %s", *Region->GetRegionTag().ToString());                    
                    Module->RegisterPowerConsumer(Object);
                }
            }
        }
    }
    else
    {
        DEBUG_ELECTRICITY_MODULE(Error, "RefreshModule: Could not find RegionSubsystem.");
    }
}

void UElectricityModule::NewParent_Implementation(URegion* OldParentRegion, URegion* NewParentRegion)
{
    DEBUG_ELECTRICITY_MODULE(Log, "NewParent: Old Parent: %s, New Parent: %s", OldParentRegion? *OldParentRegion->GetRegionTag().ToString(): TEXT("nullptr"), NewParentRegion? *NewParentRegion->GetRegionTag().ToString(): TEXT("nullptr"));
    
    UElectricityModule* Module = GetParentRegionModule<UElectricityModule>();
    if (PreviousParentModule != Module)
    {
        if (PreviousParentModule.IsValid())
        {
            DEBUG_ELECTRICITY_MODULE(Log, "NewParent: Deregistering from previous parent ElectricityModule.");
            PreviousParentModule->DeregisterPowerConsumer(this);
            PreviousParentModule->DeregisterPowerProvider(this);
        }

        if (Module)
        {
            DEBUG_ELECTRICITY_MODULE(Log, "NewParent: Registering with new parent ElectricityModule.");
            Module->RegisterPowerConsumer(this);
            Module->RegisterPowerProvider(this);
        }
    }
}

void UElectricityModule::Repair()
{
    if (bFuze)
    {
        DEBUG_ELECTRICITY_MODULE(Log, "Repair: Module is already repaired. Returning.");
        return;
    }

    bFuze = true;
    DEBUG_ELECTRICITY_MODULE(Log, "Repair: Module repaired. Calling ReevaluatePowerConsumers.");
    OnStateChange.Broadcast(this, true);
    
    ReevaluatePowerConsumers();
    ReplicatedState = PackReplicatedState();
}

void UElectricityModule::Break()
{
    if (!bFuze)
    {
        DEBUG_ELECTRICITY_MODULE(Log, "Break: Module is already broken. Returning.");
        return;
    }

    bFuze = false;
    DEBUG_ELECTRICITY_MODULE(Log, "Break: Module broken. Calling ReevaluatePowerConsumers.");
    OnStateChange.Broadcast(this, false);

    ReevaluatePowerConsumers();
    ReplicatedState = PackReplicatedState();
}

void UElectricityModule::ActivateType(EElectricityConsumerType ConsumerType, bool bRepairModule)
{
    DEBUG_ELECTRICITY_MODULE(Log, "ActivateType: Called with ConsumerType: %s", *UEnum::GetValueAsString(ConsumerType));
    
    if (!IsTypeEnabledIgnoreState(ConsumerType))
    {
        InternalActivate(ConsumerType);
    }
    else
    {
        DEBUG_ELECTRICITY_MODULE(Log, "ActivateType: ConsumerType is already enabled: %s", *UEnum::GetValueAsString(ConsumerType));
    }
    
    if (bRepairModule)
        Repair();
}

void UElectricityModule::DeactivateType(EElectricityConsumerType ConsumerType)
{
    DEBUG_ELECTRICITY_MODULE(Log, "DeactivateType: Called with ConsumerType: %s", *UEnum::GetValueAsString(ConsumerType));
    
    if (!IsTypeEnabledIgnoreState(ConsumerType))
    {
        return;
    }
    else
    {
        DEBUG_ELECTRICITY_MODULE(Log, "DeactivateType: ConsumerType is already disabled: %s", *UEnum::GetValueAsString(ConsumerType));
    }

    InternalDeactivate(ConsumerType);
}

void UElectricityModule::ActivateTypes(TArray<EElectricityConsumerType> ConsumerTypes, bool bRepairModule)
{
    for (const auto& ConsumerType : ConsumerTypes)
    {
        ActivateType(ConsumerType, false);
    }
    
    if (bRepairModule)
        Repair();
}

void UElectricityModule::DeactivateTypes(TArray<EElectricityConsumerType> ConsumerTypes)
{
    for (auto ConsumerType : ConsumerTypes)
    {
        DeactivateType(ConsumerType);
    }
}

void UElectricityModule::ActivateAllTypes()
{
    DEBUG_ELECTRICITY_MODULE(Log, "ActivateAllTypes");
    ActivateTypes(GetAllConsumerTypes(), false);
}

void UElectricityModule::DeactivateAllTypes()
{
    DEBUG_ELECTRICITY_MODULE(Log, "DeactivateAllTypes");
    DeactivateTypes(GetAllConsumerTypes());
}

void UElectricityModule::FullActivate()
{
    DEBUG_ELECTRICITY_MODULE(Log, "FullActivate");
    ActivateAllTypes();
    Repair();
}

void UElectricityModule::FullDeactivate()
{
    DEBUG_ELECTRICITY_MODULE(Log, "FullDeactivate");
    DeactivateAllTypes();
    Break();
}

float UElectricityModule::GetPowerConsumption() const
{
    float Total = 0.0f;
    for (auto Pair : ConsumerDataByType)
    {
        Total += Pair.Value.PowerConsumption;
    }
    return Total;
}

float UElectricityModule::GetTotalPowerConsumption() const
{
    float Total = 0.0f;
    for (auto Pair : ConsumerDataByType)
    {
        Total += Pair.Value.TotalPowerConsumption;
    }
    return Total;
}

float UElectricityModule::GetPowerConsumptionOfType(EElectricityConsumerType ConsumerType) const
{
    if (const FConsumerBundledData* BundledData = ConsumerDataByType.Find(ConsumerType))
        return BundledData->PowerConsumption;
    
    return 0;
}

bool UElectricityModule::IsRepaired() const
{
    return bFuze;
}

bool UElectricityModule::IsBroken() const
{
    return !bFuze;
}

float UElectricityModule::GetTotalPowerConsumptionOfType(EElectricityConsumerType ConsumerType) const
{
    if (const FConsumerBundledData* BundledData = ConsumerDataByType.Find(ConsumerType))
        return BundledData->TotalPowerConsumption;
    
    return 0;
}

float UElectricityModule::GetPowerProvisions() const
{
    return ProviderData.PowerProvision;
}

float UElectricityModule::GetTotalPowerProvisions() const
{
    return ProviderData.TotalPowerProvision;
}

bool UElectricityModule::IsProvidingPower() const
{
    return GetPowerProvisions() > 0;
}

void UElectricityModule::RefreshPowerProviderData()
{
    DEBUG_ELECTRICITY_MODULE(Log, "Refresh Providers.");

    float NewProvision = 0.f;
    float NewTotalProvision = 0.f;
    for (auto& Provider : ProviderData.RegisteredPowerProviders)
    {
        if (Provider.Key) // Check if the key is valid
        {
            if (Provider.Key->Implements<UElectricityProviderInterface>())
            {
                DEBUG_ELECTRICITY_MODULE(Log, "Refresh Providers: Checking Power Provider: %s", *Provider.Key->GetName());

                //Get Data
                FPowerProviderData OutProviderData {};
                IElectricityProviderInterface::Execute_GetPowerProviderData(Provider.Key, OutProviderData);
                DEBUG_ELECTRICITY_MODULE(Log, "Refresh Providers: Power Provider Data: %s", *OutProviderData.ToString());

                //Sync Provider State
                Provider.Value = OutProviderData.bEnabled;

                NewTotalProvision += OutProviderData.TotalPowerProvision;
                if (Provider.Value)
                    NewProvision += OutProviderData.PowerProvision;
            }
            else
            {
                DEBUG_ELECTRICITY_MODULE(Warning, "Refresh Providers: Provider Key: %s does not implement UElectricityProviderInterface.", *Provider.Key->GetName());
            }
        }
        else
        {
            DEBUG_ELECTRICITY_MODULE(Error, "Refresh Providers: Invalid Provider Key (nullptr) found in RegisteredPowerProviders!");
        }
    }
    bool bChange = ProviderData.PowerProvision != NewProvision || ProviderData.TotalPowerProvision != NewTotalProvision;
    ProviderData.PowerProvision = NewProvision;
    ProviderData.TotalPowerProvision = NewTotalProvision;

    DEBUG_ELECTRICITY_MODULE(Log, "Refresh Providers: New Power Provision: %f, New Total Power Provision: %f", NewProvision, NewTotalProvision);

    if (bChange)
    {
        DEBUG_ELECTRICITY_MODULE(Log, "Refresh Providers: Power Provision changed.");
        OnProvisionChange.Broadcast(this);

        DEBUG_ELECTRICITY_MODULE(Log, "Refresh Providers: Notifying parent about provider change.");
        NotifyParentAboutProviderChange();
    }
    else
    {
        DEBUG_ELECTRICITY_MODULE(Log, "Refresh Providers: Power Provision did not change.");
    }
}

void UElectricityModule::RegisterPowerProvider(UObject* Provider)
{
    if (!Provider)
    {
        DEBUG_ELECTRICITY_MODULE(Warning, "Register Provider: Invalid Provider!");
        return;
    }
    DEBUG_ELECTRICITY_MODULE(Log, "Register Provider: %s", *Provider->GetName());
    
    if (ProviderData.RegisteredPowerProviders.Contains(Provider))
    {
        DEBUG_ELECTRICITY_MODULE(Warning, "Register Provider: Provider %s is already registered!", *Provider->GetName());
        return;
    }
    if (!Provider->Implements<UElectricityProviderInterface>())
    {
        DEBUG_ELECTRICITY_MODULE(Warning, "Register Provider: Provider %s does not implement UElectricityProviderInterface!", *Provider->GetName());
        return;
    }
    
    ProviderData.RegisteredPowerProviders.Add(Provider);
    DEBUG_ELECTRICITY_MODULE(Log, "Register Provider: Provider registered.");
    OnChangePowerProviders.Broadcast(this);
    RefreshPowerProviderData();
}

void UElectricityModule::DeregisterPowerProvider(UObject* Provider)
{
    if (!Provider)
    {
        DEBUG_ELECTRICITY_MODULE(Warning, "Deregister Provider: Invalid Provider!");
        return;
    }

    DEBUG_ELECTRICITY_MODULE(Log, "Deregister Provider: %s", *Provider->GetName());
    
    if (ProviderData.RegisteredPowerProviders.Contains(Provider))
    {
        ProviderData.RegisteredPowerProviders.Remove(Provider);
        DEBUG_ELECTRICITY_MODULE(Log, "Deregister Provider: Provider deregistered.");

        OnChangePowerProviders.Broadcast(this);
        RefreshPowerProviderData();
    }
    else
    {
        DEBUG_ELECTRICITY_MODULE(Warning, "Deregister Provider: Provider %s is not registered!", *Provider->GetName());
    }
}

bool UElectricityModule::SetFuzeBox(UObject* FuzeBox)
{
    if (!FuzeBox)
    {
        DEBUG_ELECTRICITY_MODULE(Warning, "Set FuzeBox: Invalid FuzeBox!");
        return false;
    }

    DEBUG_ELECTRICITY_MODULE(Log, "Set FuzeBox: %s", *FuzeBox->GetName());

    if (HasFuzeBox())
    {
        if (FuzeBox == FuzeBoxData.FuzeBox)
        {
            DEBUG_ELECTRICITY_MODULE(Log, "Set FuzeBox: FuzeBox is already set!");
            return true;
        }
        
        DEBUG_ELECTRICITY_MODULE(Warning, "Set FuzeBox: FuzeBox is already set to something else!");
        return false;
    }

    if (!FuzeBox->Implements<UFuzeBoxInterface>())
    {
        DEBUG_ELECTRICITY_MODULE(Warning, "Set FuzeBox: FuzeBox does not implement IFuzeBoxInterface!");
        return false;
    }

    FuzeBoxData.FuzeBox = FuzeBox;
    RefreshFuzeData();
    IFuzeBoxInterface::Execute_OnReceiveModule(FuzeBox, this);

    DEBUG_ELECTRICITY_MODULE(Log, "Set FuzeBox: FuzeBox set to: %s", *FuzeBox->GetName());
    
    return true;
}

bool UElectricityModule::IsTypeEnabled(EElectricityConsumerType ConsumerType) const
{
    bool ReturnBool = bFuze;
    if (const FConsumerBundledData* Data = ConsumerDataByType.Find(ConsumerType))
    {
        ReturnBool = ReturnBool && Data->bEnabled;
    }

    return ReturnBool;
}

bool UElectricityModule::IsTypeEnabledIgnoreState(EElectricityConsumerType ConsumerType) const
{
    bool ReturnBool = bFuze;
    if (const FConsumerBundledData* Data = ConsumerDataByType.Find(ConsumerType))
    {
        ReturnBool = Data->bEnabled;
    }

    return ReturnBool;
}

bool UElectricityModule::AreTypesEnabled(TArray<EElectricityConsumerType> ConsumerTypes) const
{
    for (auto ConsumerType : ConsumerTypes)
    {
        if (!IsTypeEnabled(ConsumerType))
            return false;
    }
    
    return true;
}

TArray<EElectricityConsumerType> UElectricityModule::GetAllConsumerTypes() const
{
    TArray<EElectricityConsumerType> ReturnTypes {};
    for (const auto& Data : ConsumerDataByType)
        ReturnTypes.Add(Data.Key);

    return ReturnTypes;
}

void UElectricityModule::ReevaluatePowerConsumersMulti(TArray<EElectricityConsumerType> ConsumerTypes)
{
    DEBUG_ELECTRICITY_MODULE(Log, "Reevaluate Power Consumers.");

    for (auto Type: ConsumerTypes)
    {
        ReevaluatePowerConsumers(Type);
    }
}

void UElectricityModule::ReevaluatePowerConsumers(EElectricityConsumerType ConsumerType)
{
    DEBUG_ELECTRICITY_MODULE(Log, "Reevaluate Power Consumers: %s", *UEnum::GetValueAsString(ConsumerType));
    
    bool bTypeEnabled = IsTypeEnabledIgnoreState(ConsumerType);
    FConsumerBundledData* BundledData = ConsumerDataByType.Find(ConsumerType);

    if (!BundledData)
    {
        DEBUG_ELECTRICITY_MODULE(Warning, "Reevaluate Power Consumers: No BundledData found for ConsumerType: %s", *UEnum::GetValueAsString(ConsumerType));
        return;
    }

    float NewConsumption = 0.f;
    float NewTotalConsumption = 0.f;

    for (auto& Consumer: BundledData->RegisteredPowerConsumers)
    {
        if (!Consumer.Key)
        {
            DEBUG_ELECTRICITY_MODULE(Error, "Reevaluate Power Consumers: Invalid Consumer Key (nullptr) found in RegisteredPowerConsumers!");
            continue;
        }
        if (!Consumer.Key->Implements<UElectricityConsumerInterface>())
        {
            DEBUG_ELECTRICITY_MODULE(Error, "Reevaluate Power Consumers: Consumer Key: %s does not implement UElectricityConsumerInterface!", 
            *Consumer.Key->GetName());
            continue;
        }

        DEBUG_ELECTRICITY_MODULE(Log, "Reevaluate Power Consumers: Checking Consumer: %s", *Consumer.Key->GetName());

        // Get Data
        TArray<FPowerConsumerData> Data {};
        IElectricityConsumerInterface::Execute_GetPowerConsumptionData(Consumer.Key, Data);
        FPowerConsumerData LocalData{};

        DEBUG_ELECTRICITY_MODULE(Log, "Reevaluate Power Consumers: Consumer Data: %s", *LocalData.ToString());

        for (auto PowerConsumptionData: Data)
        {
            if (ConsumerType == PowerConsumptionData.ConsumerType)
            {
                LocalData = PowerConsumptionData;
                break;
            }
        }

        // Sync Consumers State
        bool PreviousState = Consumer.Value;
        Consumer.Value = bTypeEnabled && LocalData.bEnabled && IsRepaired();

        if (PreviousState != Consumer.Value)
        {
            DEBUG_ELECTRICITY_MODULE(Log, "Reevaluate Power Consumers: Consumer State Changed: %s. Previous: %s, New: %s", *Consumer.Key->GetName(), PreviousState? TEXT("true"): TEXT("false"), Consumer.Value? TEXT("true"): TEXT("false"));

            if (Consumer.Value)
            {
                DEBUG_ELECTRICITY_MODULE(Log, "Reevaluate Power Consumers: Enabling Consumer: %s. Type: %s", *Consumer.Key->GetName(), *UEnum::GetValueAsString(ConsumerType));
                IElectricityConsumerInterface::Execute_OnGainPower(Consumer.Key, ConsumerType);
            }
            else
            {
                DEBUG_ELECTRICITY_MODULE(Log, "Reevaluate Power Consumers: Disabling Consumer: %s. Type: %s", *Consumer.Key->GetName(), *UEnum::GetValueAsString(ConsumerType));
                IElectricityConsumerInterface::Execute_OnLosePower(Consumer.Key, ConsumerType);
            }
        }
        NewTotalConsumption += LocalData.TotalPowerConsumption;
        if (Consumer.Value)
            NewConsumption += LocalData.PowerConsumption;
    }

    bool bChange = BundledData->PowerConsumption!= NewConsumption || BundledData->TotalPowerConsumption!= NewTotalConsumption;
    BundledData->PowerConsumption = NewConsumption;
    BundledData->TotalPowerConsumption = NewTotalConsumption;

    if (bChange)
    {
        DEBUG_ELECTRICITY_MODULE(Log, "Reevaluate Power Consumers: Consumption changed for Type: %s. Broadcasting OnConsumptionChange delegate.", *UEnum::GetValueAsString(ConsumerType));
        OnConsumptionChange.Broadcast(this, ConsumerType);
        DEBUG_ELECTRICITY_MODULE(Log, "Reevaluate Power Consumers: Notifying parent about consumer change for Type: %s.", *UEnum::GetValueAsString(ConsumerType));
        NotifyParentAboutConsumerChange();
    }
    else
    {
        DEBUG_ELECTRICITY_MODULE(Log, "Reevaluate Power Consumers: Consumption did not change for Type: %s.", *UEnum::GetValueAsString(ConsumerType));
    }
}

void UElectricityModule::RefreshConsumerData(UObject* Consumer)
{
    DEBUG_ELECTRICITY_MODULE(Log, "RefreshConsumerData: Called for Consumer: %s", Consumer? *Consumer->GetName(): TEXT("All Consumers"));

    TArray<TObjectPtr<UObject>> ObjectsToRefresh{};
    if (Consumer)
    {
        ObjectsToRefresh.Add(Consumer);
    }
    else
    {
        RegisteredPowerConsumers.GetKeys(ObjectsToRefresh);
    }

    for (auto ConsumerToRefresh: ObjectsToRefresh)
    {
        if (!ConsumerToRefresh)
        {
            DEBUG_ELECTRICITY_MODULE(Error, "RefreshConsumerData: Invalid Consumer Key (nullptr) found in RegisteredPowerConsumers!");
            continue;
        }

        if (!RegisteredPowerConsumers.Contains(ConsumerToRefresh))
        {
            DEBUG_ELECTRICITY_MODULE(Warning, "RefreshConsumerData: Consumer Key: %s not found in RegisteredPowerConsumers!", *ConsumerToRefresh->GetName());
            continue;
        }

        FPowerConsumerDataArray& PreviousData = RegisteredPowerConsumers.FindChecked(ConsumerToRefresh);

        //Get Data
        TArray<FPowerConsumerData> Data{};
        IElectricityConsumerInterface::Execute_GetPowerConsumptionData(ConsumerToRefresh, Data);

        TArray<FPowerConsumerData> AddedConsumptionTypes;
        TArray<FPowerConsumerData> RemovedConsumptionTypes;
        TArray<FPowerConsumerData> UnchangedConsumptionTypes;
        PreviousData.GetDiff(AddedConsumptionTypes, RemovedConsumptionTypes, UnchangedConsumptionTypes, Data);

        for (auto RemovedType: RemovedConsumptionTypes)
        {
            FConsumerBundledData* BundledData = ConsumerDataByType.Find(RemovedType.ConsumerType);
            if (!BundledData)
            {
                DEBUG_ELECTRICITY_MODULE(Warning, "RefreshConsumerData: No BundledData found for ConsumerType: %s. Skipping removal.", *UEnum::GetValueAsString(RemovedType.ConsumerType));
                continue;
            }

            bool* bCurrentlyEnabled = BundledData->RegisteredPowerConsumers.Find(ConsumerToRefresh);
            if (bCurrentlyEnabled == nullptr)
            {
                DEBUG_ELECTRICITY_MODULE(Warning, "RefreshConsumerData: Consumer %s not found in RegisteredPowerConsumers for type %s. Skipping removal.", *ConsumerToRefresh->GetName(), *UEnum::GetValueAsString(RemovedType.ConsumerType));
                continue;
            }

            BundledData->RegisteredPowerConsumers.Remove(ConsumerToRefresh);
            DEBUG_ELECTRICITY_MODULE(Log, "RefreshConsumerData: Removing Consumer: %s for Type: %s", *ConsumerToRefresh->GetName(), *UEnum::GetValueAsString(RemovedType.ConsumerType));

            BundledData->TotalPowerConsumption -= RemovedType.PowerConsumption;
            if (*bCurrentlyEnabled)
            {
                BundledData->PowerConsumption -= RemovedType.PowerConsumption;

                //Lose Power Before Removing
                DEBUG_ELECTRICITY_MODULE(Log, "RefreshConsumerData: Calling OnLosePower for %s, Type: %s", *ConsumerToRefresh->GetName(), *UEnum::GetValueAsString(RemovedType.ConsumerType));
                IElectricityConsumerInterface::Execute_OnLosePower(ConsumerToRefresh, RemovedType.ConsumerType);
            }

            DEBUG_ELECTRICITY_MODULE(Log, "RefreshConsumerData: Removed Consumer: %s for Type: %s", *ConsumerToRefresh->GetName(), *UEnum::GetValueAsString(RemovedType.ConsumerType));

            OnConsumptionChange.Broadcast(this, RemovedType.ConsumerType);
            NotifyParentAboutConsumerChange();
        }
        for (auto AddedType: AddedConsumptionTypes)
        {
            bool bTypeEnabled = IsTypeEnabledIgnoreState(AddedType.ConsumerType);
            FConsumerBundledData* BundledData = ConsumerDataByType.Find(AddedType.ConsumerType);
            if (!BundledData)
            {
                DEBUG_ELECTRICITY_MODULE(Log, "RefreshConsumerData: No BundledData found for ConsumerType: %s. Creating new entry.", *UEnum::GetValueAsString(AddedType.ConsumerType));
                ConsumerDataByType.Add(AddedType.ConsumerType);
                BundledData = ConsumerDataByType.Find(AddedType.ConsumerType);
                BundledData->bEnabled = URegionSettings::GetDefaultModuleFuzeState();
            }
            bool EnableNewConsumer = bTypeEnabled && AddedType.bEnabled;
            BundledData->RegisteredPowerConsumers.Add(ConsumerToRefresh, EnableNewConsumer);

            DEBUG_ELECTRICITY_MODULE(Log, "RefreshConsumerData: Adding Consumer: %s for Type: %s", *ConsumerToRefresh->GetName(), *UEnum::GetValueAsString(AddedType.ConsumerType));

            //Initial Power Message
            if (EnableNewConsumer)
            {
                DEBUG_ELECTRICITY_MODULE(Log, "RefreshConsumerData: Calling OnGainPower for %s, Type: %s", *ConsumerToRefresh->GetName(), *UEnum::GetValueAsString(AddedType.ConsumerType));
                IElectricityConsumerInterface::Execute_OnGainPower(ConsumerToRefresh, AddedType.ConsumerType);
            }
            else
            {
                DEBUG_ELECTRICITY_MODULE(Log, "RefreshConsumerData: Calling OnLosePower for %s, Type: %s", *ConsumerToRefresh->GetName(), *UEnum::GetValueAsString(AddedType.ConsumerType));
                IElectricityConsumerInterface::Execute_OnLosePower(ConsumerToRefresh, AddedType.ConsumerType);
            }

            BundledData->TotalPowerConsumption += AddedType.PowerConsumption;
            if (EnableNewConsumer)
                BundledData->PowerConsumption += AddedType.PowerConsumption;

            DEBUG_ELECTRICITY_MODULE(Log, "RefreshConsumerData: Added Consumer: %s for Type: %s", *ConsumerToRefresh->GetName(), *UEnum::GetValueAsString(AddedType.ConsumerType));

            OnConsumptionChange.Broadcast(this, AddedType.ConsumerType);
            NotifyParentAboutConsumerChange();
        }

        // Reevaluate unchanged types (important for potential state changes)
        for (auto UnchangedType: UnchangedConsumptionTypes)
        {
            DEBUG_ELECTRICITY_MODULE(Log, "RefreshConsumerData: Reevaluating unchanged ConsumerType: %s for %s", *UEnum::GetValueAsString(UnchangedType.ConsumerType), *ConsumerToRefresh->GetName());
            ReevaluatePowerConsumers(UnchangedType.ConsumerType);
        }

        PreviousData = Data;
    }
}

void UElectricityModule::RegisterPowerConsumer(UObject* Consumer)
{
    if (!Consumer)
    {
        DEBUG_ELECTRICITY_MODULE(Log, "RegisterPowerConsumer: Invalid Consumer!");
        return;
    }
    DEBUG_ELECTRICITY_MODULE(Log, "Register PowerConsumer: %s", *Consumer->GetName());
    
    if (!Consumer->Implements<UElectricityConsumerInterface>())
    {
        DEBUG_ELECTRICITY_MODULE(Log, "RegisterPowerConsumer: Consumer %s does not implement UElectricityConsumerInterface!", *Consumer->GetName());
        return;
    }
    
    //Get Data
    TArray<FPowerConsumerData> Data {};
    IElectricityConsumerInterface::Execute_GetPowerConsumptionData(Consumer, Data);
    
    RegisteredPowerConsumers.Add(Consumer, Data);

    for (auto PowerConsumptionData: Data)
    {
        DEBUG_ELECTRICITY_MODULE(Log, "RegisterPowerConsumer: Adding Consumer: %s for Type: %s", *Consumer->GetName(), *UEnum::GetValueAsString(PowerConsumptionData.ConsumerType));
        
        FConsumerBundledData* BundledData = ConsumerDataByType.Find(PowerConsumptionData.ConsumerType);
        if (!BundledData)
        {
            DEBUG_ELECTRICITY_MODULE(Log, "RegisterPowerConsumer: No BundledData found for ConsumerType: %s. Creating new entry.", *UEnum::GetValueAsString(PowerConsumptionData.ConsumerType));
            ConsumerDataByType.Add(PowerConsumptionData.ConsumerType, GetNewDefaultBundledData(PowerConsumptionData.ConsumerType));
            BundledData = ConsumerDataByType.Find(PowerConsumptionData.ConsumerType);
            
            ReplicatedState = PackReplicatedState();
        }
        
        bool bCanEnable = CanActivate(PowerConsumptionData.ConsumerType);
        bool EnableNewConsumer = bCanEnable && PowerConsumptionData.bEnabled;
        BundledData->RegisteredPowerConsumers.Add(Consumer, EnableNewConsumer);
        DEBUG_ELECTRICITY_MODULE(Log, "RegisterPowerConsumer: Adding Consumer: %s for Type: %s", *Consumer->GetName(), *UEnum::GetValueAsString(PowerConsumptionData.ConsumerType));

        //Initial Power Message
        if (EnableNewConsumer)
        {
            DEBUG_ELECTRICITY_MODULE(Log, "RegisterPowerConsumer: Calling OnGainPower for %s, Type: %s", *Consumer->GetName(), *UEnum::GetValueAsString(PowerConsumptionData.ConsumerType));
            IElectricityConsumerInterface::Execute_OnGainPower(Consumer, PowerConsumptionData.ConsumerType);
        }
        else
        {
            DEBUG_ELECTRICITY_MODULE(Log, "RegisterPowerConsumer: Calling OnLosePower for %s, Type: %s", *Consumer->GetName(), *UEnum::GetValueAsString(PowerConsumptionData.ConsumerType));
            IElectricityConsumerInterface::Execute_OnLosePower(Consumer, PowerConsumptionData.ConsumerType);
        }

        BundledData->TotalPowerConsumption += PowerConsumptionData.PowerConsumption;
        if (EnableNewConsumer)
            BundledData->PowerConsumption += PowerConsumptionData.PowerConsumption;

        DEBUG_ELECTRICITY_MODULE(Log, "RegisterPowerConsumer: Added Consumer: %s for Type: %s", *Consumer->GetName(), *UEnum::GetValueAsString(PowerConsumptionData.ConsumerType));

        OnConsumptionChange.Broadcast(this, PowerConsumptionData.ConsumerType);
        NotifyParentAboutConsumerChange();
    }

    OnChangePowerConsumers.Broadcast(this);
}

void UElectricityModule::DeregisterPowerConsumer(UObject* Consumer)
{
    DEBUG_ELECTRICITY_MODULE(Log, "DeregisterPowerConsumer: %s", *Consumer->GetName());

    if (Consumer)
    {
        if (Consumer->Implements<UElectricityConsumerInterface>())
        {
            //Get Data
            TArray<FPowerConsumerData> Data{};
            IElectricityConsumerInterface::Execute_GetPowerConsumptionData(Consumer, Data);

            for (auto PowerConsumptionData: Data)
            {
                DEBUG_ELECTRICITY_MODULE(Log, "DeregisterPowerConsumer: Deregistering Consumer: %s for Type: %s", *Consumer->GetName(), *UEnum::GetValueAsString(PowerConsumptionData.ConsumerType));

                FConsumerBundledData* BundledData = ConsumerDataByType.Find(PowerConsumptionData.ConsumerType);
                if (!BundledData)
                {
                    DEBUG_ELECTRICITY_MODULE(Warning, "DeregisterPowerConsumer: No BundledData found for ConsumerType: %s. Skipping.", *UEnum::GetValueAsString(PowerConsumptionData.ConsumerType));
                    continue;
                }

                bool* bCurrentlyEnabled = BundledData->RegisteredPowerConsumers.Find(Consumer);
                if (bCurrentlyEnabled == nullptr)
                {
                    DEBUG_ELECTRICITY_MODULE(Warning, "DeregisterPowerConsumer: Consumer %s not found in RegisteredPowerConsumers for type %s. Skipping.", *Consumer->GetName(), *UEnum::GetValueAsString(PowerConsumptionData.ConsumerType));
                    continue;
                }
                BundledData->RegisteredPowerConsumers.Remove(Consumer);

                BundledData->TotalPowerConsumption -= PowerConsumptionData.PowerConsumption;
                if (*bCurrentlyEnabled)
                {
                    BundledData->PowerConsumption -= PowerConsumptionData.PowerConsumption;

                    //Lose Power Before Removing
                    DEBUG_ELECTRICITY_MODULE(Log, "DeregisterPowerConsumer: Calling OnLosePower for %s, Type: %s", *Consumer->GetName(), *UEnum::GetValueAsString(PowerConsumptionData.ConsumerType));
                    IElectricityConsumerInterface::Execute_OnLosePower(Consumer, PowerConsumptionData.ConsumerType);
                }
                DEBUG_ELECTRICITY_MODULE(Log, "DeregisterPowerConsumer: Removed Consumer: %s for Type: %s", *Consumer->GetName(), *UEnum::GetValueAsString(PowerConsumptionData.ConsumerType));

                OnConsumptionChange.Broadcast(this, PowerConsumptionData.ConsumerType);
                NotifyParentAboutConsumerChange();
            }

            OnChangePowerConsumers.Broadcast(this);
        }
        else
        {
            DEBUG_ELECTRICITY_MODULE(Warning, "DeregisterPowerConsumer: Consumer %s does not implement UElectricityConsumerInterface.", *Consumer->GetName());
        }
    }
    else
    {
        DEBUG_ELECTRICITY_MODULE(Warning, "DeregisterPowerConsumer: Invalid Consumer.");
    }
}

void UElectricityModule::InternalDeactivate(EElectricityConsumerType ConsumerType)
{
    DEBUG_ELECTRICITY_MODULE(Log, "InternalDeactivate: %s", *UEnum::GetValueAsString(ConsumerType));
    
    FConsumerBundledData* BundledData = ConsumerDataByType.Find(ConsumerType);
    if (!BundledData || !BundledData->bEnabled)
    {
        DEBUG_SIMPLE(LogRegions, Log, FColor::White, FString::Printf(TEXT("InternalDeactivate: No BundledData found or type already disabled. Returning.")), RegionTags::Modules::Electricity::Name);
        return;
    }
    
    BundledData->bEnabled = false;
    DEBUG_ELECTRICITY_MODULE(Log, "InternalDeactivate: Deactivated ConsumerType: %s", *UEnum::GetValueAsString(ConsumerType));
    OnConsumerTypePowerChange.Broadcast(this, ConsumerType, false);
    
    ReevaluatePowerConsumers(ConsumerType);
    ReplicatedState = PackReplicatedState();
}

void UElectricityModule::InternalActivate(EElectricityConsumerType ConsumerType)
{
    DEBUG_ELECTRICITY_MODULE(Log, "InternalActivate: %s", *UEnum::GetValueAsString(ConsumerType));
    
    FConsumerBundledData* BundledData = ConsumerDataByType.Find(ConsumerType);
    if (!BundledData || BundledData->bEnabled)
    {
        DEBUG_ELECTRICITY_MODULE(Log, "InternalActivate: No BundledData found or type already enabled. Returning.");
        return;
    }
    BundledData->bEnabled = true;

    DEBUG_ELECTRICITY_MODULE(Log, "InternalActivate: Activated ConsumerType: %s", *UEnum::GetValueAsString(ConsumerType));
    OnConsumerTypePowerChange.Broadcast(this, ConsumerType, true);
    
    ReevaluatePowerConsumers(ConsumerType);
    ReplicatedState = PackReplicatedState();
}

void UElectricityModule::NotifyParentAboutConsumerChange(bool bAllowLocalReevaluation)
{
    DEBUG_ELECTRICITY_MODULE(Log, "NotifyParentAboutConsumerChange");

    UElectricityModule* Module = GetParentRegionModule<UElectricityModule>();
    if (!Module || IsIndependent())
    {
        DEBUG_ELECTRICITY_MODULE(Log, "NotifyParentAboutConsumerChange: No parent ElectricityModule found. Calling ReevaluateState locally.");

        if (bAllowLocalReevaluation)
            ReevaluateState();

        return;
    }

    DEBUG_ELECTRICITY_MODULE(Log, "NotifyParentAboutConsumerChange: Parent ElectricityModule found. Calling ReevaluatePowerConsumers on parent.");
    Module->RefreshConsumerData(this);
}

void UElectricityModule::NotifyParentAboutProviderChange(bool bAllowLocalReevaluation)
{
    DEBUG_ELECTRICITY_MODULE(Log, "NotifyParentAboutProviderChange");

    UElectricityModule* Module = GetParentRegionModule<UElectricityModule>();
    if (!Module || IsIndependent())
    {
        DEBUG_ELECTRICITY_MODULE(Log, "NotifyParentAboutProviderChange: No parent ElectricityModule found. Calling ReevaluateState locally.");
        
        if (bAllowLocalReevaluation)
            ReevaluateState();
        
        return;
    }

    DEBUG_ELECTRICITY_MODULE(Log, "NotifyParentAboutProviderChange: Parent ElectricityModule found. Calling ReevaluatePowerProviders on parent.");
    Module->RefreshPowerProviderData();
}

void UElectricityModule::ReevaluateState()
{
    DEBUG_ELECTRICITY_MODULE(Log, "ReevaluateState");

    float PowerConsumption = GetPowerConsumption();
    float PowerProvision = GetPowerProvisions();

    DEBUG_ELECTRICITY_MODULE(Log, "ReevaluateState: Power Consumption: %f, Power Provision: %f", PowerConsumption, PowerProvision);

    if (PowerConsumption > PowerProvision)
    {
        DEBUG_ELECTRICITY_MODULE(Log, "ReevaluateState: Power Consumption exceeds Power Provision. Deactivating.");
        const URegionSettings* Settings = URegionSettings::Get();
        if (Settings)
        {
            bool bDeactivated = false;
            if (Settings->bDeactivateAllTypesOnBreak)
            {
                DEBUG_SIMPLE(LogRegions, Warning, FColor::White, FString::Printf(TEXT("ReevaluateState: Deactivating all types.")), RegionTags::Modules::Electricity::Name);
                DeactivateAllTypes();
                bDeactivated = true;
            }
            if (Settings->bDeactivateModuleOnBreak)
            {
                DEBUG_SIMPLE(LogRegions, Warning, FColor::White, FString::Printf(TEXT("ReevaluateState: Breaking module.")), RegionTags::Modules::Electricity::Name);
                Break();
                bDeactivated = true;
            }
            
            if (!bDeactivated)
            {
                DEBUG_SIMPLE(LogRegions, Warning, FColor::White, FString::Printf(TEXT("ReevaluateState: No deactivation available. Break module manually.")), RegionTags::Modules::Electricity::Name);
            }
        }
    }
    else
    {
        DEBUG_ELECTRICITY_MODULE(Log, "ReevaluateState: Power Consumption is within Power Provision. No action taken.");
    }
}

FConsumerBundledData UElectricityModule::GetNewDefaultBundledData(EElectricityConsumerType Type) const
{
    FConsumerBundledData Data {};
    Data.bEnabled = URegionSettings::IsTypeEnabledByDefault(Type);

    //Get Parents State
    if (UElectricityModule* Module = GetParentRegionModule<UElectricityModule>())
    {
        Data.bEnabled = Module->IsTypeEnabledIgnoreState(Type);
    }

    return Data;
}

void UElectricityModule::SetupBundledConsumerDatas()
{
    if (UEnum* EnumPtr = StaticEnum<EElectricityConsumerType>())
    {
        for (int32 i = 0; i < EnumPtr->GetMaxEnumValue(); ++i)
        {
            if (EnumPtr->IsValidEnumValue(i))
            {
                EElectricityConsumerType EnumValue = static_cast<EElectricityConsumerType>(i);
                DEBUG_ELECTRICITY_MODULE(Log, "Setup Consumer Datas: Adding ConsumerData for EnumValue: %s", *UEnum::GetValueAsString(EnumValue));
                ConsumerDataByType.Add(EnumValue, GetNewDefaultBundledData(EnumValue));
            }
        }
    }
}

bool UElectricityModule::HasFuzeBox() const
{
    return IsValid(FuzeBoxData.FuzeBox);
}

float UElectricityModule::GetActivationDelay() const
{
    return FuzeBoxData.CachedData.ActivationDelay;
}

bool UElectricityModule::HasActivationDelay() const
{
    return GetActivationDelay() > 0.f;
}

bool UElectricityModule::IsIndependent() const
{
    return FuzeBoxData.CachedData.bIndependent || !HasParentRegionModule<UElectricityModule>();
}

void UElectricityModule::RefreshFuzeData()
{
    FFuzeBoxData PreviousData = FuzeBoxData.CachedData;
    
    FuzeBoxData.CachedData.ActivationDelay = 0.f;
    FuzeBoxData.CachedData.bIndependent = false;

    if (HasFuzeBox())
    {
        IFuzeBoxInterface::Execute_GetFuzeBoxData(FuzeBoxData.FuzeBox, FuzeBoxData.CachedData);
    }
    else if (const URegionSettings* Settings = URegionSettings::Get())
    {
        FuzeBoxData.CachedData.ActivationDelay = Settings->DefaultActivationDelay;
    }
    else
    {
        //Something went wrong
    }

    //Check if anything changed
    if (PreviousData.bIndependent != FuzeBoxData.CachedData.bIndependent)
    {
        NotifyParentAboutConsumerChange(false);
        NotifyParentAboutProviderChange(false);
        if (IsIndependent())
            ReevaluateState();
    }
}

bool UElectricityModule::CanActivate(EElectricityConsumerType ConsumerType) const
{
    const UElectricityModule* ParentModule = this;
    do
    {
        if (!ParentModule->IsTypeEnabled(ConsumerType))
            return false;

        if (!ParentModule->IsIndependent())
            ParentModule = ParentModule->GetParentRegionModule<UElectricityModule>();
        else
            ParentModule = nullptr;
    }
    while (ParentModule);
    
    return true;
}

void UElectricityModule::TryInitializeReplication()
{
    if (UGlobalReplicator* Replicator = UGlobalReplicator::Get(this))
    {
        OnInitializeReplication(Replicator);
    }
    else
    {
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UElectricityModule::TryInitializeReplication, 0.1f, false);
    }
}

void UElectricityModule::OnInitializeReplication(UGlobalReplicator* Replicator)
{
    FOnReplicatedValueChanged Delegate;
    Delegate.BindDynamic(this, &UElectricityModule::OnRep_ReplicatedState);
    Replicator->ReplicateInt(GetReplicationKey(), ReplicatedState, Delegate, false);
}

void UElectricityModule::OnRep_ReplicatedState()
{
    TMap<EElectricityConsumerType, bool> ConsumerTypeStates;
    bool bState = false;

    UnpackReplicatedState(ConsumerTypeStates, bState);

    DEBUG_ELECTRICITY_MODULE(Log, "Receive Replicated State: Unpacked Data:");
    DEBUG_ELECTRICITY_MODULE(Log, "Receive Replicated State: Replicated State: %s", bState ? TEXT("true") : TEXT("false"));
    for (const auto& ConsumerTypeStatePair : ConsumerTypeStates)
    {
        DEBUG_ELECTRICITY_MODULE(Log, "Receive Replicated State: ConsumerType: %s, State: %s", *UEnum::GetValueAsString(ConsumerTypeStatePair.Key), ConsumerTypeStatePair.Value ? TEXT("true") : TEXT("false"));
    }

    if (bState)
    {
        DEBUG_ELECTRICITY_MODULE(Log, "Receive Replicated State: Replicated State is true. Repairing.");
        Repair();
    }
    else
    {
        DEBUG_ELECTRICITY_MODULE(Log, "Receive Replicated State: Replicated State is false. Breaking.");
        Break();
    }
    
    for (const auto& ConsumerTypeStatePair : ConsumerTypeStates)
    {
        EElectricityConsumerType ConsumerType = ConsumerTypeStatePair.Key;
    
        if (ConsumerTypeStatePair.Value)
        {
            DEBUG_ELECTRICITY_MODULE(Log, "Receive Replicated State: Activating ConsumerType: %s", *UEnum::GetValueAsString(ConsumerType));
            ActivateType(ConsumerType);
        }
        else
        {
            DEBUG_ELECTRICITY_MODULE(Log, "Receive Replicated State: Deactivating ConsumerType: %s", *UEnum::GetValueAsString(ConsumerType));
            DeactivateType(ConsumerType);
        }
    }
}

void UElectricityModule::UnpackReplicatedState(TMap<EElectricityConsumerType, bool>& ConsumerTypeStates, bool& bState) const
{
    for (auto& Pair : ConsumerDataByType)
    {
        const int16 Index = GetTypePackedIndex(Pair.Key) + 1; // +1 offset to skip the bEnabled bit
        const bool bModuleEnabled = (ReplicatedState & (1 << Index)) != 0;
        ConsumerTypeStates.Add(Pair.Key, bModuleEnabled);
    }
    bState = (ReplicatedState & 1) != 0;
}

int UElectricityModule::PackReplicatedState() const
{
    int PackedState = 0;
    PackedState |= (bFuze ? 1 : 0);

    DEBUG_ELECTRICITY_MODULE(Log, "Pack Replicated State: Replicated State: %s", bFuze ? TEXT("true") : TEXT("false"));

    for (auto& Pair : ConsumerDataByType)
    {
        const int16 Index = GetTypePackedIndex(Pair.Key) + 1; // +1 offset to skip the bEnabled bit
        const bool bModuleEnabled = Pair.Value.bEnabled;
        DEBUG_ELECTRICITY_MODULE(Log, "Pack Replicated State: ConsumerType: %s, State: %s", *UEnum::GetValueAsString(Pair.Key), bModuleEnabled ? TEXT("true") : TEXT("false"));
        PackedState |= (bModuleEnabled ? 1 : 0) << Index;
    }
    return PackedState;
}

int16 UElectricityModule::GetTypePackedIndex(EElectricityConsumerType ConsumerType) const
{
    return static_cast<int16>(ConsumerType);
}

FName UElectricityModule::GetReplicationKey() const
{
    return *FString::Printf(TEXT("%s_ElectricityModule"), *GetOwningRegionTag().GetTagName().ToString());
}