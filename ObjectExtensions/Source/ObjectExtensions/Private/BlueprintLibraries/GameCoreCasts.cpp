// Copyright Phoenix Dawn Development LLC. All Rights Reserved.

#include "BlueprintLibraries/GameCoreCasts.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"

APawn* UGameCoreCasts::GetLocalPawn(const UObject* WorldContext, TSubclassOf<APawn> InCast)
{
	if (!InCast)
		InCast = APawn::StaticClass();

	APawn* Pawn = UGameplayStatics::GetPlayerPawn(WorldContext,0);
	if (Pawn && Pawn->IsA(InCast))
		return Pawn;
	return nullptr;
}

APlayerController* UGameCoreCasts::GetLocalController(const UObject* WorldContext, TSubclassOf<APlayerController> InCast)
{
	if (!InCast)
		InCast = APlayerController::StaticClass();
	
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(WorldContext,0);
	if (PlayerController && PlayerController->IsA(InCast))
		return PlayerController;
	return nullptr;
}

AHUD* UGameCoreCasts::GetLocalHUD(const UObject* WorldContext, TSubclassOf<AHUD> InCast)
{
	if (!InCast)
		InCast = AHUD::StaticClass();

	if (APlayerController* PlayerController = GetLocalController(WorldContext, APlayerController::StaticClass()))
	{
		AHUD* HUD = PlayerController->GetHUD();
		if (HUD && HUD->IsA(InCast))
			return HUD;
	}
	return nullptr;
}

UGameInstance* UGameCoreCasts::GetLocalGameInstance(const UObject* WorldContext, TSubclassOf<UGameInstance> InCast)
{
	if (!InCast)
		InCast = UGameInstance::StaticClass();
	
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContext);
	if (GameInstance && GameInstance->IsA(InCast))
		return GameInstance;
	return nullptr;
}

AGameStateBase* UGameCoreCasts::GetLocalGameState(const UObject* WorldContext, TSubclassOf<AGameStateBase> InCast)
{
	if (!InCast)
		InCast = AGameStateBase::StaticClass();

	AGameStateBase* GameStateBase = UGameplayStatics::GetGameState(WorldContext);
	if (GameStateBase && GameStateBase->IsA(InCast))
		return GameStateBase;
	return nullptr;
}

APlayerState* UGameCoreCasts::GetLocalPlayerState(const UObject* WorldContext, TSubclassOf<APlayerState> InCast)
{
	if (!InCast)
		InCast = APlayerState::StaticClass();

	if (APlayerController* Controller = GetLocalController(WorldContext, APlayerController::StaticClass()))
	{
		APlayerState* PlayerState = Controller->PlayerState;
		if (PlayerState && PlayerState->IsA(InCast))
			return PlayerState;
	}
	return nullptr;
}

UActorComponent* UGameCoreCasts::GetLocalCoreComponent(const UObject* WorldContext, const ECoreType CoreType, const TSubclassOf<UActorComponent> Component)
{
	AActor* Actor = nullptr;
	switch (CoreType) {
		case ECoreType::Pawn:
			Actor = GetLocalPawn(WorldContext);
			break;
		case ECoreType::Controller:
			Actor = GetLocalController(WorldContext);
			break;
		case ECoreType::PlayerState:
			Actor = GetLocalPlayerState(WorldContext);
			break;
		case ECoreType::GameState:
			Actor = GetLocalGameState(WorldContext);
			break;
		case ECoreType::HUD:
			Actor = GetLocalHUD(WorldContext);
			break;
	}

	if (!Actor)
		return nullptr;

	return Actor->FindComponentByClass(Component);
}
