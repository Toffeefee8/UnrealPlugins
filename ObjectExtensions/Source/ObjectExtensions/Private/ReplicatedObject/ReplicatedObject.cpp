#include "ReplicatedObject/ReplicatedObject.h"
#include "Engine/BlueprintGeneratedClass.h"

UWorld* UReplicatedObject::GetWorld() const
{
	
	if (GetOuter() == nullptr)
	{
		return nullptr;
	}
		
	if (Cast<UPackage>(GetOuter()) != nullptr)
	{
		return Cast<UWorld>(GetOuter()->GetOuter());
	}

	if (auto OwningActor = GetOwningActor())
	{
		return OwningActor->GetWorld();
	}

	return  nullptr;
	
}

void UReplicatedObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	if (UBlueprintGeneratedClass* BPClass = Cast<UBlueprintGeneratedClass>(GetClass()))
	{
		BPClass->GetLifetimeBlueprintReplicationList(OutLifetimeProps);
	}
}

bool UReplicatedObject::IsSupportedForNetworking() const
{
	return ReplicationState == EReplicationState::Replicates;
}

int32 UReplicatedObject::GetFunctionCallspace(UFunction* Function, FFrame* Stack)
{
	if (HasAnyFlags(RF_ClassDefaultObject) || !IsSupportedForNetworking())
	{
		return GEngine->GetGlobalFunctionCallspace(Function, this, Stack);
	}
	
	return GetOuter()->GetFunctionCallspace(Function, Stack);
}

bool UReplicatedObject::CallRemoteFunction(UFunction* Function, void* Parms, FOutParmRec* OutParms, FFrame* Stack)
{
	AActor* Owner = GetOwningActor();
	if (UNetDriver* NetDriver = Owner->GetNetDriver())
	{
		NetDriver->ProcessRemoteFunction(Owner, Function, Parms, OutParms, Stack, this);
		return true;
	}
	return false;
}

bool UReplicatedObject::HasAuthority() const
{
	if (IsValid(this))
		if (AActor* Owner = GetOwningActor())
			return Owner->HasAuthority();
	return false;
}

void UReplicatedObject::BeginDestroy()
{
	if (TickDelegateHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickDelegateHandle);
		TickDelegateHandle.Reset();
	}
	UObject::BeginDestroy();
}

void UReplicatedObject::PostInitProperties()
{
	Super::PostInitProperties();

	//PreConstruct();

	//Return if this is a template object
	if (HasAnyFlags(RF_ArchetypeObject))
		return;
	
	if (GetWorld())
	{
		if (CanEverTick)
			TickDelegateHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &UReplicatedObject::InternalTick));
		OnConstruct();
	}
}

void UReplicatedObject::PostLoad()
{
	Super::PostLoad();
}



AActor* UReplicatedObject::GetOwningActor() const
{
	return GetTypedOuter<AActor>();
}

APlayerController* UReplicatedObject::GetOwningController() const
{
	for (AActor* Actor = GetOwningActor(); Actor != nullptr; Actor = Actor->GetOwner())
	{
		if (Actor->IsA<APlayerController>())
			return Cast<APlayerController>(Actor);
	}
	return nullptr;
}

void UReplicatedObject::SetOuter(AActor* NewOwner)
{
	if (IsValid(NewOwner) && IsValid(this))
		Rename(nullptr, NewOwner);
}

void UReplicatedObject::OnConstruct_Implementation()
{
}

void UReplicatedObject::PreConstruct_Implementation()
{
}

void UReplicatedObject::OnTick_Implementation(float DeltaTime)
{
}

bool UReplicatedObject::InternalTick(float DeltaTime)
{
	OnTick(DeltaTime);
	return true;
}

void UReplicatedObject::OnExplicitDestroyed_Implementation()
{
}

void UReplicatedObject::DestroyObject()
{
	if (IsValid(this))
	{
		OnExplicitDestroyed();
		MarkAsGarbage();
	}
}
