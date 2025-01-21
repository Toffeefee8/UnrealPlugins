// Copyright Phoenix Dawn Development LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ObjectSerializationInterface.generated.h"

UINTERFACE()
class UObjectSerializationInterface : public UInterface
{
	GENERATED_BODY()
};

class OBJECTEXTENSIONS_API IObjectSerializationInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	void OnSerialize();
	UFUNCTION(BlueprintNativeEvent)
	void OnDeserialize();
};
