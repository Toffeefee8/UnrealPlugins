#include "Extensions/AsyncUtility.h"
#include "UObject/EnumProperty.h"
#include "Kismet/KismetStringLibrary.h"


void FAsyncUtility::WaitForSeconds(float Duration, const TWeakObjectPtr<UObject>& WorldContextObject, float CheckInterval)
{
	const double EndTime = FPlatformTime::Seconds() + Duration;

	while (true)
	{
		// Check if the world context object is still valid
		if (!IsGameRunning(WorldContextObject))
			return;

		// Check if the time has elapsed
		if (FPlatformTime::Seconds() >= EndTime)
			return;

		// Sleep for the specified check interval
		FPlatformProcess::Sleep(CheckInterval);
	}
}


bool FAsyncUtility::IsGameRunning(TWeakObjectPtr<UObject> WorldContextObject)
{
	if (!WorldContextObject.IsValid())
		return false;

	UObject* Object = WorldContextObject.Get();
	if (!IsValid(Object))
		return false;
	
	UWorld* World = Object->GetWorld();
	if (!World)
		return false;
	
	if (World->WorldType == EWorldType::Editor || World->WorldType == EWorldType::EditorPreview)
		return false;

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
		return false;

	if (World->bIsTearingDown)
		return false;

	if (GWorld == nullptr)
		return false;

	return true;
}

void FAsyncUtility::Wait(const FGraphEventRef GraphEvenRef, const TWeakObjectPtr<UObject>& WorldContextObject,
                         const float CheckInterval, const float MaxWaitTime, FString DebugName)
{
	float ElapsedTime = 0.0f;
	
	while (ElapsedTime < MaxWaitTime)
	{
		if (!IsGameRunning(WorldContextObject))
			return;

		if (GraphEvenRef->IsComplete())
			return;
		
		FPlatformProcess::Sleep(CheckInterval);
		ElapsedTime += CheckInterval;
	}

	UE_LOG(AsyncUtility, Warning, TEXT("Wait operation timed out after %f seconds for %s"), MaxWaitTime,*DebugName);
}



ULevelStreamingDynamic* FAsyncUtility::GameThreadLevelSpawn(const FString Name,
                                                            const TSoftObjectPtr<UWorld>& LevelToSpawn,
                                                            const FVector& Location,
                                                            const FRotator& Rotation,
                                                            const TWeakObjectPtr<UObject>& WorldContextObject,
                                                            bool bSetInvisible)
{
	
	if (LevelToSpawn.IsNull())
	{
		UE_LOG(AsyncUtility, Error, TEXT("Level ref invalid"))
		return nullptr;
	}
	
	/*UObject* LevelObject = LevelToSpawn.LoadSynchronous();
	if (LevelObject == nullptr)
	{
		UE_LOG(AsyncUtility, Error, TEXT("Interior level object is null."));
		return nullptr;
	}

	if (!IsGameRunning(WorldContextObject))
	{
		UE_LOG(AsyncUtility, Error, TEXT("World ContextObj ref invalid"))
		return nullptr;
	}

	UE_LOG(AsyncUtility, Display, TEXT("Trying to stream level %s"),*LevelObject->GetName());*/
	
	//Load Level
	bool bLevelLoadSuccess=false;
	ULevelStreamingDynamic* LoadedLevel =
		ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(WorldContextObject.Get(),
		                                                         LevelToSpawn,
		                                                         Location,
		                                                         Rotation,bLevelLoadSuccess,
		                                                         Name);

	
	if (!bLevelLoadSuccess)
	{
		UE_LOG(LogTemp, Error, TEXT("Level could not be loaded: %s"), *Name);
		return nullptr;
	}

	LoadedLevel->SetShouldBeVisible(!bSetInvisible);

	return  LoadedLevel;
}

void FAsyncUtility::Wait(const TFunction<bool()>& ConditionFunc, const TWeakObjectPtr<UObject>& WorldContextObject,
	const float CheckInterval, const float MaxWaitTime, const FString& DebugString)
{

	if (IsInGameThread())
	{
		UE_LOG(AsyncUtility, Fatal, TEXT("Cannot block the game thread"));
		return;
	}

	float ElapsedTime = 0.0f;
	
	while (ElapsedTime < MaxWaitTime)
	{
		if (!IsGameRunning(WorldContextObject))
			return;

		if (ConditionFunc())
			return;
		
		FPlatformProcess::Sleep(CheckInterval);
		ElapsedTime += CheckInterval;
	}

	UE_LOG(AsyncUtility, Error, TEXT("Wait operation %s timed out after %f seconds"), *DebugString,MaxWaitTime);
}

void FAsyncUtility::WaitOneFrameBlocking(const TWeakObjectPtr<UObject> WorldContextObject)
{
	if (IsInGameThread())
	{
		UE_LOG(AsyncUtility, Type::Fatal, TEXT("Cannot block the game thread"));
		return;
	}
	
	const uint32 StartFrameNumber = GFrameCounter;
	
	while (true)
	{
		if (!IsGameRunning(WorldContextObject))
			return;
		
		if (GFrameCounter>StartFrameNumber)
			return;
		
		FPlatformProcess::Sleep(0.01f);
	}
}

TFuture<ULevelStreamingDynamic*> FAsyncUtility::TryStreamLevel(const FString& Name,
	const TSoftObjectPtr<UWorld>& InteriorLevel, const FVector& Location,const FRotator& Rotation,
	const TWeakObjectPtr<UObject> WorldContextObject, bool bSetInvisible, float MaxLevelWaitTime)
{
	return FAsyncUtility::RunOnAnyThread<ULevelStreamingDynamic*>(
		WorldContextObject,
		[
			Name,
			InteriorLevel,
			Location,
			Rotation,
			WorldContextObject,
			bSetInvisible,
			MaxLevelWaitTime
			
		]()->ULevelStreamingDynamic*
		{
			ULevelStreamingDynamic* LevelStreamingDynamic = nullptr;
			for (int i= 0; i<3;i++)
			{
				TFuture<ULevelStreamingDynamic*> StreamLevelFuture =
					StreamLevel(Name, InteriorLevel, Location, Rotation, WorldContextObject, bSetInvisible,
				                                                                 MaxLevelWaitTime);
				FString DebugName = "Waiting For Level To Spawn";
				FAsyncUtility::Wait(StreamLevelFuture,WorldContextObject,0.1f,MaxLevelWaitTime,DebugName);
				if(!StreamLevelFuture.IsValid())
					continue;
				LevelStreamingDynamic = StreamLevelFuture.Get();
				if (!IsValid(LevelStreamingDynamic))
				{
					UE_LOG(AsyncUtility, Error, TEXT("Failed to load level. Already tried %i times. Trying again"), i);
				}
				return LevelStreamingDynamic;
			}

			UE_LOG(AsyncUtility, Error, TEXT("Failed to load level."));
			return nullptr;
		});
}


TFuture<ULevelStreamingDynamic*> FAsyncUtility::StreamLevel(const FString& Name,
                                                            const TSoftObjectPtr<UWorld>& InteriorLevel, const FVector& Location,const FRotator& Rotation,
                                                            const TWeakObjectPtr<UObject> WorldContextObject, bool bSetInvisible, float MaxLevelWaitTime)
{
	return FAsyncUtility::RunOnAnyThread<ULevelStreamingDynamic*>(WorldContextObject,
		[Name,
			InteriorLevel,
			Location,
			Rotation,
			WorldContextObject,
			bSetInvisible,
			MaxLevelWaitTime]() mutable-> ULevelStreamingDynamic*
		{
			//Spawn Level On Gamethread
			TFuture<ULevelStreamingDynamic*> LevelSpawnFuture =
				FAsyncUtility::RunOnGameThread<ULevelStreamingDynamic*>(WorldContextObject,
			[Name,
					InteriorLevel,
					Location,
					Rotation,
					WorldContextObject,
					bSetInvisible,
					MaxLevelWaitTime]() mutable-> ULevelStreamingDynamic*
				{
					return GameThreadLevelSpawn(Name,InteriorLevel,Location,Rotation, WorldContextObject, bSetInvisible);
				});
			FAsyncUtility::Wait(LevelSpawnFuture,WorldContextObject, 0.1f,MaxLevelWaitTime,"LevelSpawnFuture");

			if (!IsGameRunning(WorldContextObject))
				return nullptr;

			if (!LevelSpawnFuture.IsReady())
				return nullptr;
			
			ULevelStreamingDynamic* LevelStreamingDynamic = LevelSpawnFuture.Get();
			if (LevelStreamingDynamic == nullptr)
			{
				return nullptr;
			}
			

			//Wait For Level To Fully Initialise
			FAsyncUtility::Wait ([LevelStreamingDynamic,bSetInvisible, WorldContextObject]()->bool
			{
				TWeakObjectPtr<ULevelStreamingDynamic> WeakLevelStreamingDyn = LevelStreamingDynamic;
				
				TFuture<bool> CheckFuture = 
					FAsyncUtility::RunOnGameThread<bool>(WorldContextObject,[WeakLevelStreamingDyn,bSetInvisible,WorldContextObject]()->bool
					{
						if (!IsGameRunning(WorldContextObject))
							return true;

						if (!IsValid(WeakLevelStreamingDyn.Get()))
							return true;

						const bool bVisibilityCondition = WeakLevelStreamingDyn->IsLevelVisible()||bSetInvisible;
						const bool bLoaded = WeakLevelStreamingDyn->IsLevelLoaded() && IsValid(WeakLevelStreamingDyn->GetLoadedLevel());
						const bool bUnlocked = !WeakLevelStreamingDyn->bLocked;

						const FString VisibleLevel = UKismetStringLibrary::Conv_BoolToString(WeakLevelStreamingDyn->IsLevelVisible()||bSetInvisible);
						const FString LoadedLevel = UKismetStringLibrary::Conv_BoolToString(WeakLevelStreamingDyn->IsLevelLoaded());
						const FString LoadedLevelValid  = UKismetStringLibrary::Conv_BoolToString(IsValid(WeakLevelStreamingDyn->GetLoadedLevel()));
						const FString UnlockedLevel =UKismetStringLibrary::Conv_BoolToString(!WeakLevelStreamingDyn->bLocked);
						UE_LOG(LogTemp, Log, TEXT("Level is Visible: %s, Loaded: %s,Loaded Level valid: %s ,Unlocked %s "),*VisibleLevel,*LoadedLevel,*LoadedLevelValid,*UnlockedLevel)

						return bVisibilityCondition && bLoaded &&bUnlocked;
					});
				

				FAsyncUtility::Wait(CheckFuture,WorldContextObject);
				return CheckFuture.Get();
				
			},WorldContextObject,0.5f,MaxLevelWaitTime,"Trying to load Level from AsyncUtility");

			TWeakObjectPtr<ULevelStreamingDynamic> WeakLevelStreamingDyn = LevelStreamingDynamic;

			
			if (!IsGameRunning(WorldContextObject))
			{
				return nullptr;
			}

			return LevelStreamingDynamic;
		});
}



