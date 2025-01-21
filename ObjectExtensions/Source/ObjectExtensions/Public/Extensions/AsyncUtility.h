#pragma once

#include "CoreMinimal.h"
#include "Async/AsyncWork.h"
#include "Async/Async.h"
#include "Engine/LevelStreamingDynamic.h"
#include "Math/Vector.h"
#include "Templates/SharedPointer.h"
#include "AsyncUtility.generated.h"


DEFINE_LOG_CATEGORY_STATIC(AsyncUtility,Display,Display);

/**
 * A utility class for handling asynchronous operations in Unreal Engine.
 */
USTRUCT()
struct OBJECTEXTENSIONS_API FAsyncUtility
{

	GENERATED_BODY();

public:
	static bool IsGameRunning(TWeakObjectPtr<UObject> WorldContextObject);

	template<typename T>
	static T GetFutureValue(TFuture<T>&& Future);


# pragma region Waiting
public:
	/**
	 * Waits for a future to complete asynchronously on a background thread.
	 * The function periodically checks if the future is ready and only proceeds when it is.
	 * 
	 * @param Future The future to wait for.
	 * @param WorldContextObject The context object to check for validity.
	 * @param CheckInterval The interval, in seconds, to wait between checks. Defaults to 0.01 seconds.
	 * @param MaxWaitTime
	 * @param DebugName
	 */
	template<typename Result>
	static void Wait(
	const TFuture<Result>& Future,
	const TWeakObjectPtr<UObject>& WorldContextObject,
	const float CheckInterval = 0.1f,
	float MaxWaitTime = 2,
	const FString DebugName = "DefaultDebugName");


	static void Wait(
	const FGraphEventRef GameThreadTask,
	const TWeakObjectPtr<UObject>& WorldContextObject,
	const float CheckInterval = 0.1f,
	const float MaxWaitTime = 2,
	FString DebugName  = "DefaultDebug");

	/**
	* Waits for all futures in the array to complete asynchronously on a background thread.
	* Collects their results and returns them in a future.
	* 
	* @param Futures The array of futures to wait for.
	* @param WorldContextObject The context object to check for validity.
	* @param CheckInterval
	* @param MaxWaitTime
	* @param DebugName
	* @return A future containing an array of results from the completed futures.
	*/
	template <typename ResultType>
	static TFuture<std::conditional_t<std::is_void_v<ResultType>, void, TArray<ResultType>>> WaitForFutures(
		TArray<TFuture<ResultType>> Futures,
		TWeakObjectPtr<UObject> WorldContextObject,
		const float CheckInterval = 0.1f,
		const float MaxWaitTime = 2,
		FString DebugName = "WaitForFuturesDebugName");
	
	static void WaitForSeconds(float Duration, const TWeakObjectPtr<UObject>& WorldContextObject, float CheckInterval =0.01f);
	
	/**
	* Blocks the current thread until the next frame is rendered.
	* Useful for synchronization that requires waiting for the next frame.
	*/
	static void WaitOneFrameBlocking(const TWeakObjectPtr<UObject> WorldContextObject);

	/**
	* Executes a function asynchronously that waits for a condition to be met.
	* The condition function will be checked periodically at the specified interval.
	* This function runs on the game thread and does not block background threads.
	* 
	* @param ConditionFunc The function to call that checks the condition.
	* @param WorldContextObject The context object to check for validity.
	* @param CheckInterval The interval, in seconds, to wait between checks. Defaults to 0.01 seconds.
	* @param MaxWaitTime
	* @param DebugString
	*/
	static void Wait(
	const TFunction<bool()>& ConditionFunc,
	const TWeakObjectPtr<UObject>& WorldContextObject,
	const float CheckInterval = 0.1f,
	const float MaxWaitTime = 2.f,
	const FString& DebugString = "DefaultDebugString");
	
# pragma endregion Waiting

	/**
	* Streams a level asynchronously and returns a future containing the streaming dynamic level.
	* 
	* @param Name The name of the level instance.
	* @param InteriorLevel The soft object pointer to the world asset.
	* @param Location The location where the level should be spawned.
	* @param Rotation
	* @param WorldContextObject The context object to check for validity.
	* @param bSetInvisible
	* @param MaxLevelWaitTime
	* @return A future containing the dynamically streamed level.
	 */
	static TFuture<ULevelStreamingDynamic*> StreamLevel(
		const FString& Name,
		const TSoftObjectPtr<UWorld>& InteriorLevel,
		const FVector& Location,
		const FRotator& Rotation,
		const TWeakObjectPtr<UObject> WorldContextObject,
		bool bSetInvisible = false,
		float MaxLevelWaitTime = 4);


	static TFuture<ULevelStreamingDynamic*> TryStreamLevel(
		const FString& Name,
		const TSoftObjectPtr<UWorld>& InteriorLevel,
		const FVector& Location,
		const FRotator& Rotation,
		TWeakObjectPtr<UObject> WorldContextObject,
		bool bSetInvisible = false,
		float MaxLevelWaitTime = 4);
	
	
	template <class ThreadSafetyMode,typename... VarTypes>
	static void WaitForEventBlocking(const TWeakObjectPtr<UObject>& WorldContextObject,
	                  TBaseDynamicMulticastDelegate<ThreadSafetyMode,void,VarTypes...>& DelegateToWaitFor,
	                  float CheckInterval = 0.1f);;
	
	template <class ThreadSafetyMode, class ... VarTypes>
	static  TFuture<void> WaitForEvent(
		const TWeakObjectPtr<UObject>& WorldContextObject,
		TBaseDynamicMulticastDelegate<ThreadSafetyMode,void,VarTypes...>& DelegateToWaitFor, float CheckInterval = 0.1f);


	template <class ResultType>
	static  TFuture<ResultType> RunOnGameThread(TWeakObjectPtr<UObject> WorldContextObject,TUniqueFunction<ResultType()> InFunction);

	template <class ResultType>
	static  TFuture<ResultType> RunOnAnyThread(TWeakObjectPtr<UObject> WorldContextObject,TUniqueFunction<ResultType()> InFunction, bool bEndsWithGame = true);

private:
	static ULevelStreamingDynamic* GameThreadLevelSpawn(
		const FString Name,
		const TSoftObjectPtr<UWorld>& LevelToSpawn,
		const FVector& Location,
		const FRotator& Rotation, const TWeakObjectPtr<UObject>& WorldContextObject, bool bSetInvisible);


};



template <typename T>
T FAsyncUtility::GetFutureValue(TFuture<T>&& Future)
{
	if (Future.IsValid() && Future.IsReady())
	{
		return Future.Get(); 
	}
	return T{};
}


template <typename Result>
void FAsyncUtility::Wait(const TFuture<Result>& Future, const TWeakObjectPtr<UObject>& WorldContextObject,
	const float CheckInterval, const float MaxWaitTime, const FString DebugName)
{
	if (IsInGameThread())
	{
		UE_LOG(AsyncUtility, Type::Fatal, TEXT("Cannot block the game thread"));
		return;
	}

	double TotalWaitTime = 0.0;

	while (true)
	{
		if (!IsGameRunning(WorldContextObject) || !Future.IsValid())
			return;

		if (!Future.IsValid())
			return;
		
		if (Future.IsReady())
			return;

		if (TotalWaitTime >= MaxWaitTime)
		{
			UE_LOG(AsyncUtility, Warning, TEXT("Max wait of %f time reached for Future %s"), MaxWaitTime,*DebugName);
			return;
		}

		FPlatformProcess::Sleep(CheckInterval);
		TotalWaitTime += CheckInterval;
	}
}

template <typename ResultType>
auto FAsyncUtility::WaitForFutures(
	TArray<TFuture<ResultType>> Futures,
	TWeakObjectPtr<UObject> WorldContextObject,
	const float CheckInterval,
	const float MaxWaitTime,
	FString DebugName) 
	-> TFuture<std::conditional_t<std::is_void_v<ResultType>, void, TArray<ResultType>>>
{
	if (IsInGameThread())
	{
		UE_LOG(AsyncUtility, Type::Fatal, TEXT("Cannot block the game thread"));
		if constexpr (std::is_void_v<ResultType>)
		{
			TPromise<void> EmptyPromise;
			TFuture<void> EmptyFuture = EmptyPromise.GetFuture();
			EmptyPromise.SetValue();
			return EmptyFuture;
		}
		else
		{
			TPromise<TArray<ResultType>> EmptyPromise;
			TFuture<TArray<ResultType>> EmptyFuture = EmptyPromise.GetFuture();
			EmptyPromise.SetValue(TArray<ResultType>{});
			return EmptyFuture;
		}
	}
	
	// Handle the case where ResultType is void
	if constexpr (std::is_void_v<ResultType>)
	{
		// Async task to wait for all futures to complete
		return FAsyncUtility::RunOnAnyThread<void>(WorldContextObject,
		[
			WorldContextObject,
			Futures = MoveTemp(Futures),
			CheckInterval,
			MaxWaitTime,
			DebugName]
			()->void
			{
				for (auto& Future : Futures)
				{
					FAsyncUtility::Wait(Future,WorldContextObject, CheckInterval,MaxWaitTime,DebugName);
					if(!FAsyncUtility::IsGameRunning(WorldContextObject))
						return;
				}
			}
		,true);
	}
	else
	{
		// Async task to wait for all futures and gather results
		return  FAsyncUtility::RunOnAnyThread<TArray<ResultType>>(WorldContextObject,
			 [
				WorldContextObject,
				Futures = MoveTemp(Futures),
				CheckInterval,
				MaxWaitTime,
				DebugName]()->TArray<ResultType> {

			TArray<ResultType> Results;
			Results.Reserve(Futures.Num());
			for (auto& Future : Futures)
			{
				FAsyncUtility::Wait(Future,WorldContextObject, CheckInterval,MaxWaitTime,DebugName);
				Results.Add(Future.Get());
				if(!FAsyncUtility::IsGameRunning(WorldContextObject))
					return Results;
			}
			return Results;
		},true);
	}
}

UCLASS()
class OBJECTEXTENSIONS_API UEventWaitHelper : public UObject
{
	GENERATED_BODY()

public:
	bool bEventTriggered;

	UEventWaitHelper()
		: bEventTriggered(false) {}

	UFUNCTION()
	void OnEventTriggered()
	{
		bEventTriggered = true;
	}
	
};
template <class ThreadSafetyMode, typename ... VarTypes>
void FAsyncUtility::WaitForEventBlocking(const TWeakObjectPtr<UObject>& WorldContextObject,
	TBaseDynamicMulticastDelegate<ThreadSafetyMode, void, VarTypes...>& DelegateToWaitFor, float CheckInterval)
{
	if (IsInGameThread())
	{
		UE_LOG(AsyncUtility, Type::Fatal, TEXT("Cannot block the game thread"));
		return;
	}

	const TFuture<UEventWaitHelper*> EventHelperCreation =
		RunOnGameThread<UEventWaitHelper*>(WorldContextObject,[&DelegateToWaitFor]() mutable  ->UEventWaitHelper* 
		{
			UEventWaitHelper* EventWaitHelper = NewObject<UEventWaitHelper>();
			EventWaitHelper->bEventTriggered = false;
			TScriptDelegate<ThreadSafetyMode> ScriptDelegate;
			ScriptDelegate.BindUFunction(EventWaitHelper, "OnEventTriggered");
			DelegateToWaitFor.Add(ScriptDelegate);
			return EventWaitHelper;
				
		});

	Wait (EventHelperCreation,WorldContextObject);
	UEventWaitHelper* EventWaitHelper = EventHelperCreation.Get();

	// Wait loop
	while (true)
	{
		if (!IsGameRunning(WorldContextObject))
		{
			break;
		}
	
		if (EventWaitHelper->bEventTriggered)
		{
			break;
		}
	
		FPlatformProcess::Sleep(CheckInterval);
	}
	
	// Clean up
	DelegateToWaitFor.Remove(EventWaitHelper,FName("OnEventTriggered"));
}

template <class ThreadSafetyMode, typename ... VarTypes>
TFuture<void> FAsyncUtility::WaitForEvent(const TWeakObjectPtr<UObject>& WorldContextObject,
	TBaseDynamicMulticastDelegate<ThreadSafetyMode,void,VarTypes...> &DelegateToWaitFor, float CheckInterval)
{
	
	TPromise<void> Promise;
	TFuture<void> Future = Promise.GetFuture();

	if (IsInGameThread())
	{
		UE_LOG(AsyncUtility, Fatal, TEXT("Cannot block the game thread"));
		Promise.SetValue();
		return Future;
	}

	FFunctionGraphTask::CreateAndDispatchWhenReady(
		[Promise = MoveTemp(Promise),WorldContextObject,&DelegateToWaitFor,CheckInterval]() mutable->void
		{
			WaitForEventBlocking<ThreadSafetyMode,VarTypes...>(WorldContextObject,DelegateToWaitFor,CheckInterval);
			Promise.SetValue();
			
		}, TStatId(),
		nullptr,
		ENamedThreads::AnyBackgroundThreadNormalTask);

	return Future;
}



template<typename ResultType>
TFuture<ResultType> FAsyncUtility::RunOnGameThread(TWeakObjectPtr<UObject> WorldContextObject, TUniqueFunction<ResultType()> InFunction)
{
    TPromise<ResultType> Promise;
    TFuture<ResultType> Future = Promise.GetFuture();
	
    if (!IsGameRunning(WorldContextObject))
    {
        if constexpr (std::is_void_v<ResultType>)
            Promise.SetValue();
        else
            Promise.SetValue(ResultType{});

    	return Future;
    }

    FGraphEventRef GameThreadTask = FFunctionGraphTask::CreateAndDispatchWhenReady(
        [InFunction = MoveTemp(InFunction), Promise = MoveTemp(Promise), WorldContextObject]() mutable
        {
            if (!IsGameRunning(WorldContextObject))
            {
                if constexpr (std::is_void_v<ResultType>)
                    Promise.SetValue(); 
                else
                    Promise.SetValue(ResultType{});
                return;
            }
        	
            if constexpr (std::is_void_v<ResultType>)
            {
                InFunction();
                Promise.SetValue(); 
            }
            else
                Promise.SetValue(InFunction());
		},
        TStatId(),
        nullptr,
        ENamedThreads::GameThread);

    return Future;
}

template<typename ResultType>
TFuture<ResultType> FAsyncUtility::RunOnAnyThread(TWeakObjectPtr<UObject> WorldContextObject,TUniqueFunction<ResultType()> InFunction, bool bEndsWithGame)
{
	TPromise<ResultType> Promise;
	TFuture<ResultType> Future = Promise.GetFuture();
	
	if (!IsGameRunning(WorldContextObject))
	{
		if constexpr (std::is_void_v<ResultType>)
			Promise.SetValue();
		else
			Promise.SetValue(ResultType{});

		return Future;
	}

	FGraphEventRef GameThreadTask = FFunctionGraphTask::CreateAndDispatchWhenReady(
		[InFunction = MoveTemp(InFunction), Promise = MoveTemp(Promise), WorldContextObject]() mutable
		{
			if (!IsGameRunning(WorldContextObject))
			{
				if constexpr (std::is_void_v<ResultType>)
					Promise.SetValue(); 
				else
					Promise.SetValue(ResultType{});
				return;
			}

			if (!InFunction)
			{
				if constexpr (std::is_void_v<ResultType>)
					Promise.SetValue(); 
				else
					Promise.SetValue(ResultType{});
				return;
			}
			
			if constexpr (std::is_void_v<ResultType>)
			{
				InFunction();
				Promise.SetValue(); 
			}
			else
				Promise.SetValue(InFunction());
		},
		TStatId(),
		nullptr,
		ENamedThreads::AnyBackgroundThreadNormalTask);

	return Future;
}













