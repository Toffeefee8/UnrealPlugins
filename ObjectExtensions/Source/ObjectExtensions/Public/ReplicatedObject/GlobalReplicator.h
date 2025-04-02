#pragma once

#include "CoreMinimal.h"
#include "ObjectReplicator.h"
#include "GlobalReplicator.generated.h"

class UGlobalReplicatorProxy;
DECLARE_LOG_CATEGORY_EXTERN(LogGlobalReplicator, Log, All);

#pragma region Enums and Structs
UENUM(BlueprintType)
enum class EReplicatedValueType : uint8
{
	Float,
	Bool,
	Int,
	ByteArray,
	String,
	Vector,
};

UENUM(BlueprintType)
enum class EReplicationAccessType : uint8
{
	OnlyServer,
	OnlyClient,
	Both,
};

USTRUCT()
struct FReplicatedKey
{
	GENERATED_BODY()

public:
	FReplicatedKey() {  }
	FReplicatedKey(FName InReplicationKey, uint32 InTimeStamp) : ReplicationKey(InReplicationKey), TimeStamp(InTimeStamp) {}

	UPROPERTY()
	FName ReplicationKey = NAME_None;
	UPROPERTY()
	uint32 TimeStamp = 0;
};
#pragma endregion

DECLARE_DYNAMIC_DELEGATE(FOnReplicatedValueChanged);

UCLASS()
class OBJECTEXTENSIONS_API UGlobalReplicator : public UObjectReplicator
{
	GENERATED_BODY()

public:
	UGlobalReplicator();
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, BlueprintPure, DisplayName = "Get Unique ID From Object", Category = "Global Replicator|Helpers", meta = (AdvancedDisplay = 1))
	static FName GetUniqueIDFromObject(UObject* Object, FString Suffix = "");
	
	UFUNCTION(BlueprintCallable, DisplayName = "Get Global Replicator", Category = "Global Replicator", meta = (WorldContext = "WorldContext"))
	static UGlobalReplicator* Get(UObject* WorldContext);
	
	UFUNCTION(BlueprintCallable, Category = "Global Replicator", meta = (AdvancedDisplay = 3))
	void ReplicateFloat(
		FName ReplicationKey,
		UPARAM(ref) float& InValue,
		const FOnReplicatedValueChanged& OnChanged,
		bool bLocalCallOnChange = true,
		EReplicationAccessType AccessType = EReplicationAccessType::Both,
		bool bGetValueFromServer = true);
	
	UFUNCTION(BlueprintCallable, Category = "Global Replicator", meta = (AdvancedDisplay = 3))
	void ReplicateBool(
		FName ReplicationKey,
		UPARAM(ref) bool& InValue,
		const FOnReplicatedValueChanged& OnChanged,
		bool bLocalCallOnChange = true,
		EReplicationAccessType AccessType = EReplicationAccessType::Both,
		bool bGetValueFromServer = true);

	UFUNCTION(BlueprintCallable, Category = "Global Replicator", meta = (AdvancedDisplay = 3))
	void ReplicateInt(
		FName ReplicationKey,
		UPARAM(ref) int& InValue,
		const FOnReplicatedValueChanged& OnChanged,
		bool bLocalCallOnChange = true,
		EReplicationAccessType AccessType = EReplicationAccessType::Both,
		bool bGetValueFromServer = true);
	
	UFUNCTION(BlueprintCallable, Category = "Global Replicator", meta = (AdvancedDisplay = 3))
	void ReplicateByteArray(
		FName ReplicationKey,
		UPARAM(ref) TArray<uint8>& InValue,
		const FOnReplicatedValueChanged& OnChanged,
		bool bLocalCallOnChange = true,
		EReplicationAccessType AccessType = EReplicationAccessType::Both,
		bool bGetValueFromServer = true);
	
	UFUNCTION(BlueprintCallable, Category = "Global Replicator", meta = (AdvancedDisplay = 3))
	void ReplicateString(
		FName ReplicationKey,
		UPARAM(ref) FString& InValue,
		const FOnReplicatedValueChanged& OnChanged,
		bool bLocalCallOnChange = true,
		EReplicationAccessType AccessType = EReplicationAccessType::Both,
		bool bGetValueFromServer = true);

	UFUNCTION(BlueprintCallable, Category = "Global Replicator", meta = (AdvancedDisplay = 3))
	void ReplicateVector(
		FName ReplicationKey,
		UPARAM(ref) FVector& InValue,
		const FOnReplicatedValueChanged& OnChanged,
		bool bLocalCallOnChange = true,
		EReplicationAccessType AccessType = EReplicationAccessType::Both,
		bool bGetValueFromServer = true);

	UFUNCTION(BlueprintCallable, Category = "Global Replicator")
	bool DereplicateData(
		FName ReplicationKey,
		bool bPropagateToRemote = true);

protected:

	friend UGlobalReplicatorProxy;
	//RPCs
	void Server_UpdateData(FReplicatedKey ReplicationKey, const TArray<uint8>& NewData, bool OnlyUpdateRequested);
	void Server_RequestData(FName ReplicationKey);
	void Server_DereplicateData(FName ReplicationKey);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_UpdateData(FReplicatedKey ReplicationKey, const TArray<uint8>& NewData, bool OnlyUpdateRequested = false);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_DereplicateData(FName ReplicationKey);
	
private:
	
	struct FLocalData
	{
		struct FCallBackPair
		{
			FCallBackPair() {  }
			FCallBackPair(TFunction<void(const TArray<uint8>&)> Callback, bool bLocalCallOnChange) : Callback(Callback), bLocalCallOnChange(bLocalCallOnChange) {  }
			
			TFunction<void(const TArray<uint8>&)> Callback {};
			bool bLocalCallOnChange = true;
		};
		
		//Original Variable
		void* ValuePtr;
		//Last Serialized Value
		TArray<uint8> LastSentBytes;
		//TimeStamp
		uint32 LastChangeTimestamp = 0;
		//AccessType
		EReplicationAccessType AccessType;
		//Data Type
		EReplicatedValueType DataType;
		//CallBacks
		TArray<FCallBackPair> Callbacks;
		//NOT FULLY USED - for replication validation in the future
		bool bPendingLocalUpdate = false;
	};

	//Data
	TMap<FName, FLocalData> ReplicatedDataMap;
	UPROPERTY()
	mutable TObjectPtr<UGlobalReplicatorProxy> ClientReplicatorProxy = nullptr;
	
	UGlobalReplicatorProxy* GetClientReplicatorProxy() const;
	uint32 GetCurrentTimeStamp() const;

	//Helpers
	void InternalReplicate(
		FName ReplicationKey,
		void* ValuePtr,
		EReplicatedValueType DataType,
		TFunction<void(const TArray<uint8>&)> Callback,
		bool bLocalCallOnChange,
		EReplicationAccessType AccessType,
		bool bGetValueFromServer);
	
	void PackCurrentValue(void* ValuePtr, EReplicatedValueType DataType, TArray<uint8>& OutBytes) const;
	void ApplyLastSentData(FLocalData& Data);
	bool DeleteData(FName ReplicationKey);
	bool HasAuthorityToChange(const FLocalData& Data) const;

	//Debug
	static FString GetValueString(EReplicatedValueType DataType, TArray<uint8> Bytes);
	static FString GetValueString(EReplicatedValueType DataType, void* ValuePtr);
};