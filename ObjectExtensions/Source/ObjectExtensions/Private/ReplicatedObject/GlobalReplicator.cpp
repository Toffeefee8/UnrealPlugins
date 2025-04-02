#include "ReplicatedObject/GlobalReplicator.h"

#include "GameFramework/GameStateBase.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "ReplicatedObject/GlobalReplicatorProxy.h"
#include "Save/PropertyPackingLibrary.h"

DEFINE_LOG_CATEGORY(LogGlobalReplicator)

UGlobalReplicator::UGlobalReplicator()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UGlobalReplicator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
#if WITH_EDITOR
	double StartTime = FPlatformTime::Seconds();
#endif
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	bool bRegisteredChange = false;
	for (auto& Pair : ReplicatedDataMap)
	{
		FName Key = Pair.Key;
		FLocalData& Data = Pair.Value;

		if (Data.ValuePtr)
		{
			TArray<uint8> CurrentBytes;
			PackCurrentValue(Data.ValuePtr, Data.DataType, CurrentBytes);

			//Continue if no changes
			if (CurrentBytes == Data.LastSentBytes)
				continue;
			
			if (!HasAuthorityToChange(Data))
			{
				UE_LOG(LogGlobalReplicator, Warning, TEXT("No authority to change data for key: %s. Reverting value."), *Key.ToString());

				//Revert Value
				ApplyLastSentData(Data);
				continue;
			}
			Data.bPendingLocalUpdate = false;
			Data.LastChangeTimestamp = GetCurrentTimeStamp();

			//Debug: Value Changes
			FString OldValueString = GetValueString(Data.DataType, Data.LastSentBytes);
			FString NewValueString = GetValueString(Data.DataType, Data.ValuePtr);
			UE_LOG(LogGlobalReplicator, Log, TEXT("Data changed for key: %s. OldValue: %s, NewValue: %s. Executing callbacks."), *Key.ToString(), *OldValueString, *NewValueString);

			//Set Data and Execute Callbacks
			bRegisteredChange = true;
			Data.LastSentBytes = CurrentBytes;
			for (auto& CallBackPair : Data.Callbacks)
			{
				if (CallBackPair.bLocalCallOnChange)
					CallBackPair.Callback(CurrentBytes);
			}

			FReplicatedKey CurrentKey = FReplicatedKey(Key, GetCurrentTimeStamp());
			//RPCs
			if (GetOwner()->HasAuthority())
			{
				Multicast_UpdateData(CurrentKey, CurrentBytes, false);
			}
			else
			{
				UGlobalReplicatorProxy* Proxy = GetClientReplicatorProxy();
				if (!Proxy)
				{
					UE_LOG(LogGlobalReplicator, Error, TEXT("No Client Replicator Proxy Found for key: %s!!!"), *Key.ToString());
					continue;
				}

				Proxy->Server_ForwardChangeValue(CurrentKey, CurrentBytes);
			}
		}
	}
#if WITH_EDITOR
	if (bRegisteredChange || false)
	{
		double EndTime = FPlatformTime::Seconds();
		double ElapsedTimeMs = (EndTime - StartTime) * 1000.0;
		UE_LOG(LogGlobalReplicator, Log, TEXT("TickComponent executed in %.3f ms"), ElapsedTimeMs);
	}
#endif
}

FName UGlobalReplicator::GetUniqueIDFromObject(UObject* Object, FString Suffix)
{
	if (!Object)
	{
		return NAME_None;
	}

	FString PathName = Object->GetPathName();

#if WITH_EDITOR
	//Remove PIE prefix: "UEDPIE_X_" where X is the PIE instance number
	const FRegexPattern PIEPattern(TEXT("UEDPIE_\\d+_"));
	FRegexMatcher Matcher(PIEPattern, PathName);

	if (Matcher.FindNext())
	{
		PathName.RemoveAt(Matcher.GetMatchBeginning(), Matcher.GetMatchEnding() - Matcher.GetMatchBeginning(), EAllowShrinking::No);
	}
#endif

	if (!Suffix.IsEmpty())
	{
		PathName += TEXT("_") + Suffix;
	}

	return FName(*PathName);
}

UGlobalReplicator* UGlobalReplicator::Get(UObject* WorldContext)
{
	if (!WorldContext)
	{
		return nullptr;
	}

	UWorld* World = WorldContext->GetWorld();
	if (!World)
	{
		return nullptr;
	}

	AGameStateBase* GameState = World->GetGameState();
	if (!GameState)
	{
		return nullptr;
	}

	UGlobalReplicator* Replicator = GameState->GetComponentByClass<UGlobalReplicator>();
	if (!Replicator)
	{
		UE_LOG(LogGlobalReplicator, Error, TEXT("No valid global replicator on the Game State!"))
	}

	return Replicator;
}

void UGlobalReplicator::ReplicateFloat(FName ReplicationKey, float& InValue, const FOnReplicatedValueChanged& OnChanged, bool bLocalCallOnChange, EReplicationAccessType AccessType, bool bGetValueFromServer)
{
	auto CallbackWrapper = [OnChanged](const TArray<uint8>& Data)
	{
		OnChanged.ExecuteIfBound();
	};

	InternalReplicate(ReplicationKey, static_cast<void*>(&InValue), EReplicatedValueType::Float, CallbackWrapper, bLocalCallOnChange, AccessType, bGetValueFromServer);
}

void UGlobalReplicator::ReplicateBool(FName ReplicationKey, bool& InValue, const FOnReplicatedValueChanged& OnChanged, bool bLocalCallOnChange, EReplicationAccessType AccessType, bool bGetValueFromServer)
{
	auto CallbackWrapper = [OnChanged](const TArray<uint8>& Data)
	{
		OnChanged.ExecuteIfBound();
	};

	InternalReplicate(ReplicationKey, static_cast<void*>(&InValue), EReplicatedValueType::Bool, CallbackWrapper, bLocalCallOnChange, AccessType, bGetValueFromServer);
}

void UGlobalReplicator::ReplicateInt(FName ReplicationKey, int& InValue, const FOnReplicatedValueChanged& OnChanged, bool bLocalCallOnChange, EReplicationAccessType AccessType, bool bGetValueFromServer)
{
	auto CallbackWrapper = [OnChanged](const TArray<uint8>& Data)
	{
		OnChanged.ExecuteIfBound();
	};

	InternalReplicate(ReplicationKey, static_cast<void*>(&InValue), EReplicatedValueType::Int, CallbackWrapper, bLocalCallOnChange, AccessType, bGetValueFromServer);
}

void UGlobalReplicator::ReplicateByteArray(FName ReplicationKey, TArray<uint8>& InValue, const FOnReplicatedValueChanged& OnChanged, bool bLocalCallOnChange, EReplicationAccessType AccessType, bool bGetValueFromServer)
{
	auto CallbackWrapper = [OnChanged](const TArray<uint8>& Data)
	{
		OnChanged.ExecuteIfBound();
	};

	InternalReplicate(ReplicationKey, static_cast<void*>(&InValue), EReplicatedValueType::ByteArray, CallbackWrapper, bLocalCallOnChange, AccessType, bGetValueFromServer);
}

void UGlobalReplicator::ReplicateString(FName ReplicationKey, FString& InValue, const FOnReplicatedValueChanged& OnChanged, bool bLocalCallOnChange, EReplicationAccessType AccessType, bool bGetValueFromServer)
{
	auto CallbackWrapper = [OnChanged](const TArray<uint8>& Data)
	{
		OnChanged.ExecuteIfBound();
	};

	InternalReplicate(ReplicationKey, static_cast<void*>(&InValue), EReplicatedValueType::String, CallbackWrapper, bLocalCallOnChange, AccessType, bGetValueFromServer);
}

void UGlobalReplicator::ReplicateVector(FName ReplicationKey, FVector& InValue, const FOnReplicatedValueChanged& OnChanged, bool bLocalCallOnChange, EReplicationAccessType AccessType, bool bGetValueFromServer)
{
	auto CallbackWrapper = [OnChanged](const TArray<uint8>& Data)
	{
		OnChanged.ExecuteIfBound();
	};

	InternalReplicate(ReplicationKey, static_cast<void*>(&InValue), EReplicatedValueType::Vector, CallbackWrapper, bLocalCallOnChange, AccessType, bGetValueFromServer);
}

bool UGlobalReplicator::DereplicateData(FName ReplicationKey, bool bPropagateToRemote)
{
	bool bSuccess = false;
	bool bHasRights = true;
	if (ReplicatedDataMap.Contains(ReplicationKey))
	{
		FLocalData& Data = ReplicatedDataMap[ReplicationKey];
		if (HasAuthorityToChange(Data))
		{
			bSuccess = DeleteData(ReplicationKey);
		}
		else
		{
			bHasRights = false;
		}
	}
	
	if (bPropagateToRemote && bHasRights)
	{
		if (GetOwner()->HasAuthority())
		{
			Multicast_DereplicateData(ReplicationKey);
		}
		else
		{
			UGlobalReplicatorProxy* Proxy = GetClientReplicatorProxy();
			if (Proxy)
			{
				Proxy->Server_ForwardDereplicate(ReplicationKey);
			}
			else
			{
				UE_LOG(LogGlobalReplicator, Error, TEXT("DereplicateData: No Client Replicator Proxy found."));
			}
		}
	}

	return bSuccess;
}

void UGlobalReplicator::Server_UpdateData(FReplicatedKey ReplicationKey, const TArray<uint8>& NewData, bool OnlyUpdateRequested)
{
	Multicast_UpdateData(ReplicationKey, NewData, OnlyUpdateRequested);
}

void UGlobalReplicator::Server_RequestData(FName ReplicationKey)
{
	if (ReplicatedDataMap.Contains(ReplicationKey))
	{
		FLocalData& Data = ReplicatedDataMap[ReplicationKey];
		FReplicatedKey ReplicatedKey = FReplicatedKey(ReplicationKey, Data.LastChangeTimestamp);
		Multicast_UpdateData(ReplicatedKey, Data.LastSentBytes, true);
	}
}

void UGlobalReplicator::Server_DereplicateData(FName ReplicationKey)
{
	Multicast_DereplicateData(ReplicationKey);
}

void UGlobalReplicator::Multicast_UpdateData_Implementation(FReplicatedKey ReplicationKey, const TArray<uint8>& NewData, bool OnlyUpdateRequested)
{
	if (ReplicatedDataMap.Contains(ReplicationKey.ReplicationKey))
	{
		FLocalData& Data = ReplicatedDataMap[ReplicationKey.ReplicationKey];

		//Return if only update pending data
		if (OnlyUpdateRequested && !Data.bPendingLocalUpdate)
		{
			UE_LOG(LogGlobalReplicator, Log, TEXT("OnReceiveData: Returning because only update pending data and no pending update for key: %s"), *ReplicationKey.ReplicationKey.ToString());
			return;
		}

		//Return if Replicated Data is out of date
		if (!OnlyUpdateRequested && Data.LastChangeTimestamp >= ReplicationKey.TimeStamp)
		{
			UE_LOG(LogGlobalReplicator, Log, TEXT("OnReceiveData: Returning because data is out of date for key: %s"), *ReplicationKey.ReplicationKey.ToString());
			return;
		}
		Data.LastChangeTimestamp = ReplicationKey.TimeStamp;
		
		//Return if no change
		if (!OnlyUpdateRequested && NewData == Data.LastSentBytes)
		{
			UE_LOG(LogGlobalReplicator, Log, TEXT("OnReceiveData: Returning because no change for key: %s"), *ReplicationKey.ReplicationKey.ToString());
			return;
		}

		FString NewValueString = GetValueString(Data.DataType, NewData);
		FString OldValueString = GetValueString(Data.DataType, Data.ValuePtr);
		UE_LOG(LogGlobalReplicator, Log, TEXT("OnReceiveData: New data for key: %s. OldValue: %s, NewValue: %s. Executing callbacks."), *ReplicationKey.ReplicationKey.ToString(), *OldValueString, *NewValueString);

		Data.bPendingLocalUpdate = false;
		Data.LastSentBytes = NewData;
		ApplyLastSentData(Data);

		for (auto& CallBackPair : Data.Callbacks)
		{
			CallBackPair.Callback(NewData);
		}
	}
	else
	{
		UE_LOG(LogGlobalReplicator, Warning, TEXT("OnReceiveData: No ReplicationData found for key: %s"), *ReplicationKey.ReplicationKey.ToString());
	}
}

void UGlobalReplicator::Multicast_DereplicateData_Implementation(FName ReplicationKey)
{
	DeleteData(ReplicationKey);
}

UGlobalReplicatorProxy* UGlobalReplicator::GetClientReplicatorProxy() const
{
	if (!IsValid(ClientReplicatorProxy))
	{
		if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
		{
			ClientReplicatorProxy = PlayerController->GetComponentByClass<UGlobalReplicatorProxy>();
		}
	}
	
	return ClientReplicatorProxy;
}

uint32 UGlobalReplicator::GetCurrentTimeStamp() const
{
	AGameStateBase* GameState = GetWorld()->GetGameState();
	if (!GameState)
		return 0;

	return static_cast<uint32>(GameState->GetServerWorldTimeSeconds() * 1000);
}

void UGlobalReplicator::InternalReplicate(FName ReplicationKey, void* ValuePtr, EReplicatedValueType DataType, TFunction<void(const TArray<uint8>&)> Callback, bool bLocalCallOnChange, EReplicationAccessType AccessType, bool bGetValueFromServer)
{
	FLocalData::FCallBackPair CallbackPair(Callback, bLocalCallOnChange);
	if (ReplicatedDataMap.Contains(ReplicationKey))
	{
		ReplicatedDataMap[ReplicationKey].Callbacks.Add(CallbackPair);
		if (bLocalCallOnChange)
			Callback(ReplicatedDataMap[ReplicationKey].LastSentBytes);
	}
	else
	{
		FLocalData NewData;
		NewData.ValuePtr = ValuePtr;
		NewData.DataType = DataType;
		NewData.AccessType = AccessType;
		
		//Initialize LastSentBytes with the current value.
		PackCurrentValue(ValuePtr, DataType, NewData.LastSentBytes);
		
		NewData.Callbacks.Add(CallbackPair);
		ReplicatedDataMap.Add(ReplicationKey, NewData);

		if (bGetValueFromServer && !GetOwner()->HasAuthority())
		{
			UGlobalReplicatorProxy* Proxy = GetClientReplicatorProxy();
			if (!Proxy)
			{
				UE_LOG(LogGlobalReplicator, Error, TEXT("No Client Replicator Proxy Found!!!"));
				return;
			}
			
			ReplicatedDataMap[ReplicationKey].bPendingLocalUpdate = bGetValueFromServer;
			Proxy->Server_ForwardRequestValue(ReplicationKey);
		}
		else
		{
			if (bLocalCallOnChange)
				Callback(NewData.LastSentBytes);
		}
	}
}

void UGlobalReplicator::PackCurrentValue(void* ValuePtr, EReplicatedValueType DataType, TArray<uint8>& OutBytes) const
{
	switch (DataType)
	{
	case EReplicatedValueType::Float:
		{
			float Value = *static_cast<float*>(ValuePtr);
			UPropertyPackingLibrary::PackValue<float>(Value, OutBytes);
			break;
		}
	case EReplicatedValueType::Bool:
		{
			bool Value = *static_cast<bool*>(ValuePtr);
			UPropertyPackingLibrary::PackValue<bool>(Value, OutBytes);
			break;
		}
	case EReplicatedValueType::Int:
		{
			int Value = *static_cast<int*>(ValuePtr);
			UPropertyPackingLibrary::PackValue<int>(Value, OutBytes);
			break;
		}
	case EReplicatedValueType::String:
		{
			FString Value = *static_cast<FString*>(ValuePtr);
			UPropertyPackingLibrary::PackValue<FString>(Value, OutBytes);
			break;
		}
	case EReplicatedValueType::Vector:
		{
			FVector Value = *static_cast<FVector*>(ValuePtr);
			UPropertyPackingLibrary::PackValue<FVector>(Value, OutBytes);
			break;
		}
	case EReplicatedValueType::ByteArray:
		{
			TArray<uint8> Value = *static_cast<TArray<uint8>*>(ValuePtr);
			UPropertyPackingLibrary::PackValue<TArray<uint8>>(Value, OutBytes);
			break;
		}
	default:
		break;
	}
}

void UGlobalReplicator::ApplyLastSentData(FLocalData& Data)
{
	switch (Data.DataType)
	{
	case EReplicatedValueType::Float:
		{
			float NewValue = 0.f;
			UPropertyPackingLibrary::UnpackValue<float>(Data.LastSentBytes, NewValue);
			*static_cast<float*>(Data.ValuePtr) = NewValue;
			break;
		}
	case EReplicatedValueType::Bool:
		{
			bool NewValue = false;
			UPropertyPackingLibrary::UnpackValue<bool>(Data.LastSentBytes, NewValue);
			*static_cast<bool*>(Data.ValuePtr) = NewValue;
			break;
		}
	case EReplicatedValueType::Int:
		{
			int NewValue = 0;
			UPropertyPackingLibrary::UnpackValue<int>(Data.LastSentBytes, NewValue);
			*static_cast<int*>(Data.ValuePtr) = NewValue;
			break;
		}
	case EReplicatedValueType::String:
		{
			FString NewValue = FString();
			UPropertyPackingLibrary::UnpackValue<FString>(Data.LastSentBytes, NewValue);
			*static_cast<FString*>(Data.ValuePtr) = NewValue;
			break;
		}
	case EReplicatedValueType::Vector:
		{
			FVector NewValue = FVector();
			UPropertyPackingLibrary::UnpackValue<FVector>(Data.LastSentBytes, NewValue);
			*static_cast<FVector*>(Data.ValuePtr) = NewValue;
			break;
		}
	case EReplicatedValueType::ByteArray:
		{
			TArray<uint8> NewValue {};
			UPropertyPackingLibrary::UnpackValue<TArray<uint8>>(Data.LastSentBytes, NewValue);
			*static_cast<TArray<uint8>*>(Data.ValuePtr) = NewValue;
			break;
		}
	default:
		break;
	}
}

bool UGlobalReplicator::DeleteData(FName ReplicationKey)
{
	int32 NumRemoved = ReplicatedDataMap.Remove(ReplicationKey);
	UE_LOG(LogGlobalReplicator, Log, TEXT("DeleteData: Removed %d entries for key: %s"), NumRemoved, *ReplicationKey.ToString());
	return NumRemoved > 0;
}

bool UGlobalReplicator::HasAuthorityToChange(const FLocalData& Data) const
{
	bool bServer = GetOwner()->HasAuthority();
	return bServer ? Data.AccessType != EReplicationAccessType::OnlyClient : Data.AccessType != EReplicationAccessType::OnlyServer;
}

FString UGlobalReplicator::GetValueString(EReplicatedValueType DataType, TArray<uint8> Bytes)
{
	switch (DataType)
	{
	case EReplicatedValueType::Float:
		{
			float Value;
			UPropertyPackingLibrary::UnpackValue<float>(Bytes, Value);
			return FString::Printf(TEXT("%f"), Value);
		}
	case EReplicatedValueType::Bool:
		{
			bool Value;
			UPropertyPackingLibrary::UnpackValue<bool>(Bytes, Value);
			return Value ? FString("true") : FString("false");
		}
	case EReplicatedValueType::Int:
		{
			int Value;
			UPropertyPackingLibrary::UnpackValue<int>(Bytes, Value);
			return FString::Printf(TEXT("%d"), Value);
		}
	case EReplicatedValueType::String:
		{
			FString Value;
			UPropertyPackingLibrary::UnpackValue<FString>(Bytes, Value);
			return Value;
		}
	case EReplicatedValueType::Vector:
		{
			FVector Value;
			UPropertyPackingLibrary::UnpackValue<FVector>(Bytes, Value);
			return Value.ToString();
		}
	case EReplicatedValueType::ByteArray:
		{
			TArray<uint8> Value;
			UPropertyPackingLibrary::UnpackValue<TArray<uint8>>(Bytes, Value);
			
			TArray<FString> StringValues;
			for (uint8 Byte : Value)
			{
				StringValues.Add(FString::FromInt(Byte));
			}

			FString Result = FString::Join(StringValues, TEXT(", "));
			return Result;
		}
	default:
		break;
	}

	return "";
}

FString UGlobalReplicator::GetValueString(EReplicatedValueType DataType, void* ValuePtr)
{
	if (!ValuePtr)
		return "";

	switch (DataType)
	{
	case EReplicatedValueType::Float:
		{
			float Value = *static_cast<float*>(ValuePtr);
			return FString::Printf(TEXT("%f"), Value);
		}
	case EReplicatedValueType::Bool:
		{
			bool Value = *static_cast<bool*>(ValuePtr);
			return Value ? FString("true") : FString("false");
		}
	case EReplicatedValueType::Int:
		{
			int Value = *static_cast<int*>(ValuePtr);
			return FString::Printf(TEXT("%d"), Value);
		}
	case EReplicatedValueType::String:
		{
			FString Value = *static_cast<FString*>(ValuePtr);
			return Value;
		}
	case EReplicatedValueType::Vector:
		{
			FVector Value = *static_cast<FVector*>(ValuePtr);
			return Value.ToString();
		}
	case EReplicatedValueType::ByteArray:
		{
			TArray<uint8> Value = *static_cast<TArray<uint8>*>(ValuePtr);
			
			TArray<FString> StringValues;
			for (uint8 Byte : Value)
			{
				StringValues.Add(FString::FromInt(Byte));
			}

			FString Result = FString::Join(StringValues, TEXT(", "));
			return Result;
		}
	default:
		break;
	}

	return "";
}