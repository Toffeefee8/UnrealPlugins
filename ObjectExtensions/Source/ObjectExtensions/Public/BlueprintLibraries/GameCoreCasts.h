// Copyright Phoenix Dawn Development LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Enums/CoreType.h"
#include "GameFramework/HUD.h"
#include "GameCoreCasts.generated.h"

UCLASS()
class OBJECTEXTENSIONS_API UGameCoreCasts : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (WorldContext  = "WorldContext", DeterminesOutputType = "Cast"), Category = "GameCore|Gets")
	static APawn* GetLocalPawn(const UObject* WorldContext, TSubclassOf<APawn> Cast = nullptr);

	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (WorldContext  = "WorldContext", DeterminesOutputType = "Cast"), Category = "GameCore|Gets")
	static APlayerController* GetLocalController(const UObject* WorldContext, TSubclassOf<APlayerController> Cast = nullptr);

	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (WorldContext  = "WorldContext", DeterminesOutputType = "Cast"), Category = "GameCore|Gets")
	static AHUD* GetLocalHUD(const UObject* WorldContext, TSubclassOf<AHUD> Cast = nullptr);

	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (WorldContext  = "WorldContext", DeterminesOutputType = "Cast"), Category = "GameCore|Gets")
	static UGameInstance* GetLocalGameInstance(const UObject* WorldContext, TSubclassOf<UGameInstance> Cast = nullptr);

	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (WorldContext  = "WorldContext", DeterminesOutputType = "Cast"), Category = "GameCore|Gets")
	static AGameStateBase* GetLocalGameState(const UObject* WorldContext, TSubclassOf<AGameStateBase> Cast = nullptr);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (WorldContext  = "WorldContext", DeterminesOutputType = "Cast"), Category = "GameCore|Gets")
	static APlayerState* GetLocalPlayerState(const UObject* WorldContext, TSubclassOf<APlayerState> Cast = nullptr);

	//Components
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (WorldContext  = "WorldContext", DeterminesOutputType = "Component"), Category = "GameCore|Gets")
	static UActorComponent* GetLocalCoreComponent(const UObject* WorldContext, ECoreType CoreType = ECoreType::GameState, TSubclassOf<UActorComponent> Component = nullptr);

};
