#include "Extensions/HelperFunctionLibrary.h"

#include "GameplayTagsManager.h"
#include "GameplayTagContainer.h"
#include "NavigationSystem.h"
#include "Engine/LevelStreamingDynamic.h"
#include "Kismet/GameplayStatics.h"

ERelevanceType UHelperFunctionLibrary::GetRelevancePawn(APawn* Pawn)
{
	if (Pawn)
	{
		if (Pawn->GetController())
		{
			return GetRelevanceController(Pawn->GetController());
		}
	}
	return Simulated;
}
ERelevanceType UHelperFunctionLibrary::GetRelevanceController(AController* Controller)
{
	if (Controller->HasAuthority())
	{
		if (Controller->IsLocalController())
		{
			return LocalServer;
		}

		return Server;
	}

	if (Controller->IsLocalController())
	{
		return Local;
	}
	
	return Simulated;
}

FString UHelperFunctionLibrary::RemoveParentTagsFromTag(FGameplayTag SourceTag, FGameplayTag ParentToRemove)
{
	const FString SourceString = SourceTag.ToString();
	const FString ParentString = ParentToRemove.ToString();
	
	if (!SourceString.StartsWith(ParentString))
	{
		return SourceString;
	}
	
	if (SourceString.Equals(ParentString, ESearchCase::CaseSensitive))
	{
		return SourceString;
	}
	
	const int32 ParentLen = ParentString.Len();


	int32 RemoveLen = ParentLen;
	if (SourceString.IsValidIndex(ParentLen) && SourceString[ParentLen] == TEXT('.'))
	{
		RemoveLen = ParentLen + 1;
	}
	
	return SourceString.Mid(RemoveLen);
}

bool UHelperFunctionLibrary::GetLocalAuthority(const UObject* WorldContextObject)
{
	return UGameplayStatics::GetPlayerController(WorldContextObject, 0)->HasAuthority();
}

int UHelperFunctionLibrary::ConvertEnumToInteger(uint8 Byte)
{
	return FMath::Pow(2,static_cast<float>(Byte));
}

bool UHelperFunctionLibrary::BitmaskContainsEnum(uint8 EnumByte, int Bitmask)
{
	int EnumAsInt = ConvertEnumToInteger(EnumByte);
	return (Bitmask & EnumAsInt) > 0;
}

float UHelperFunctionLibrary::ClampAbsoluteValue(const float Value, const float MinValue, float MaxValue)
{
	// Get the absolute value
	float AbsValue = FMath::Abs(Value);

	// Clamp the absolute value
	float ClampedAbsValue = FMath::Clamp(AbsValue, MinValue, MaxValue);;

	// Restore the original sign
	const float Multiplier = Value==0?1: FMath::Sign(Value);
	const float ClampedValue =  Multiplier * ClampedAbsValue;

	return ClampedValue;
}

float UHelperFunctionLibrary::MaxAbsoluteValue(const float ValueToUseAbsolute, const float B)
{
	// Get the absolute value
	const float AbsValue = FMath::Abs(ValueToUseAbsolute);

	// Clamp the absolute value
	const float ClampedAbsValue = FMath::Max(AbsValue, B);

	// Restore the original sign
	const float Multiplier = ValueToUseAbsolute==0?1: FMath::Sign(ValueToUseAbsolute);
	const float ClampedValue =  Multiplier * ClampedAbsValue;

	return ClampedValue;
}

void UHelperFunctionLibrary::GetAllActorsOfClassFromStreamLevel(
	ULevelStreaming* LevelStreaming,
	TSubclassOf<AActor> ActorClass,
	TArray<AActor *> & OutActors)
{
	if (!LevelStreaming)
	{
		OutActors.Empty();
		return;
	}

	if (!LevelStreaming->GetLoadedLevel())
	{
		OutActors.Empty();
		return;
	}

	OutActors = LevelStreaming->GetLoadedLevel()->Actors.FilterByPredicate([ActorClass](const auto Actor)
		{
			return IsValid(Actor) && Actor->IsA(ActorClass);
		});
}

void UHelperFunctionLibrary::GetAllActorsOfClassFromLevel(
	ULevel* Level,
	TSubclassOf<AActor> ActorClass,
	TArray<AActor *> & OutActors)
{
	OutActors = Level->Actors.FilterByPredicate([ActorClass](const auto Actor)
		{
			return Actor && Actor->IsA(ActorClass);
		});
}

bool UHelperFunctionLibrary::IsActorFullyInBox2D(const AActor* Actor, const FBox2f& Box2D)
{
	if (!Actor)
	{
		return false;
	}

	// Get the actor's bounding box
	FBox ActorBox = Actor->GetComponentsBoundingBox();

	// Convert FBox2f to FBox
	FVector Min(Box2D.Min.X, Box2D.Min.Y, -FLT_MAX);
	FVector Max(Box2D.Max.X, Box2D.Max.Y, FLT_MAX);
	FBox Box3D(Min, Max);

	// Check if the actor's bounding box is fully inside the 3D box
	return Box3D.IsInside(ActorBox);
}

TSoftObjectPtr<UWorld> UHelperFunctionLibrary::GetWorldSoftObject(ULevel* Level)
{
	if (!Level)
	{
		return nullptr;
	}
	
	UWorld* World = Level->GetWorld();
	if (!World)
	{
		return nullptr;
	}
	
	UPackage* Package = World->GetOutermost();
	
	FString PackageName = Package->GetName();
	FSoftObjectPath WorldSoftObjectPath(PackageName + "." + World->GetName());
	
	return TSoftObjectPtr<UWorld>(WorldSoftObjectPath);
}

UObject* UHelperFunctionLibrary::GetDefaultObjectBP(TSubclassOf<UObject> ActorClass)
{
	if (!IsValid(ActorClass))
		return nullptr;
	
	return ActorClass->GetDefaultObject();
}

int UHelperFunctionLibrary::GetMaxDecimalPlaces(TArray<float> FloatArray)
{
	int MaxDecimalPlaces = 0;
	for (const float Number : FloatArray)
	{
		FString NumberStr = FString::SanitizeFloat(Number);
		const int DecimalPointIndex = NumberStr.Find(TEXT("."));
		if (DecimalPointIndex != INDEX_NONE)
		{
			int DecimalPlaces = NumberStr.Len() - DecimalPointIndex - 1;
			MaxDecimalPlaces = FMath::Max(MaxDecimalPlaces, DecimalPlaces);
		}
	}
	return MaxDecimalPlaces;
}

bool UHelperFunctionLibrary::IsRoundNumber(float Number)
{
	return Number == FMath::RoundToFloat(Number);
}

void UHelperFunctionLibrary::DestroyAllValidActors(TArray<AActor*> Actors)
{
	DestroyAllValidActors(Actors,nullptr);
}

TArray<FGameplayTag> UHelperFunctionLibrary::GetFilteredGameplayTags(const FString& Prefix)
{
	TArray<FGameplayTag> FilteredTags;
	const UGameplayTagsManager& TagManager = UGameplayTagsManager::Get();
	FGameplayTagContainer AllTags;
	TagManager.RequestAllGameplayTags(AllTags, true);
	// Filter tags based on the prefix
	for (const FGameplayTag& Tag : AllTags)
	{
		if (Tag.ToString().StartsWith(Prefix))
		{
			FilteredTags.Add(Tag);
		}
	}
	return FilteredTags;
}

void UHelperFunctionLibrary::DestroyAllValidActors(TArray<AActor*> Actors,TFunction<bool(AActor*)> AdditionalConditions)
{
	for (AActor* Actor : Actors)
	{
		if (IsValid(Actor) 
			&& !Actor->IsPendingKillPending()
			&& (AdditionalConditions == nullptr || AdditionalConditions(Actor)))
		{
			Actor->Destroy();
		}
	}
}

void UHelperFunctionLibrary::UnloadAllValidLevels(TArray<ULevelStreamingDynamic*> Levels,
	TFunction<bool(ULevelStreamingDynamic*)> AdditionalConditions)
{
	for (ULevelStreamingDynamic* Level: Levels)
	{
		if (IsValid(Level)
			&& (AdditionalConditions == nullptr || AdditionalConditions(Level)))
			Level->SetIsRequestingUnloadAndRemoval(true);
	}
}

TArray<FVector> UHelperFunctionLibrary::GeneratePointsOnCircleProjected(
	const UObject* WorldContextObject,
	const FVector& Center,
	float CircleRadius,
	const FVector& Normal,
	const int32 NumPoints,
	const bool bProjectToNavMesh,
	const float ProjectionRadius)
{
	TArray<FVector> Points;
	if (NumPoints <= 0) return Points;

	FVector NormalizedNormal = Normal.GetSafeNormal();
	FQuat RotationQuat = FQuat::FindBetweenNormals(FVector::UpVector, NormalizedNormal);
	float AngleStep = 2.0f * PI / NumPoints;

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World) return Points;

	UNavigationSystemV1* NavSys = bProjectToNavMesh ? FNavigationSystem::GetCurrent<UNavigationSystemV1>(World) : nullptr;

	for (int32 i = 0; i < NumPoints; ++i)
	{
		float Angle = i * AngleStep;
		FVector PointPosition = FVector(FMath::Cos(Angle) * CircleRadius, FMath::Sin(Angle) * CircleRadius, 0.0f);
		FVector RotatedPoint = RotationQuat.RotateVector(PointPosition);
		FVector StartPoint = Center + RotatedPoint;
		FVector EndPoint = StartPoint + (NormalizedNormal * ProjectionRadius); // Raycast in normal direction

		FHitResult HitResult;
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(GeneratePointsOnCircleProjected), true);
		if (World->LineTraceSingleByChannel(HitResult, StartPoint, EndPoint, ECC_Visibility, QueryParams))
		{
			FVector ProjectedPoint = HitResult.ImpactPoint;

			// Optional NavMesh projection
			if (bProjectToNavMesh && NavSys)
			{
				FNavLocation NavLocation;
				if (NavSys->ProjectPointToNavigation(ProjectedPoint, NavLocation, FVector(ProjectionRadius)))
				{
					Points.Add(NavLocation.Location);
				}
				else
				{
					Points.Add(ProjectedPoint);
				}
			}
			else
			{
				Points.Add(ProjectedPoint);
			}
		}
		else
		{
			// If no hit, use the end point as fallback and attempt NavMesh projection if requested.
			FVector FallbackPoint = EndPoint;
			if (bProjectToNavMesh && NavSys)
			{
				FNavLocation NavLocation;
				if (NavSys->ProjectPointToNavigation(FallbackPoint, NavLocation, FVector(ProjectionRadius)))
				{
					Points.Add(NavLocation.Location);
				}
				else
				{
					Points.Add(FallbackPoint);
				}
			}
			else
			{
				Points.Add(FallbackPoint);
			}
		}
	}

	return Points;
}
 
