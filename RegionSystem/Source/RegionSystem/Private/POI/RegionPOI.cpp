#include "POI/RegionPOI.h"

#include "RegionSubsystem.h"
#include "RegionVolume.h"
#include "Components/BillboardComponent.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "POI/RegionRendering.h"
#include "POI/Processors/CustomGrid.h"
#include "POI/Processors/POITypeProcessor.h"
#include "Settings/RegionSettings.h"

bool FLocationCache::IsLocal() const
{
	return bIsLocal;
}

bool FLocationCache::IsGlobal() const
{
	return !bIsLocal;
}

TArray<FVector> FLocationCache::GetAsLocal() const
{
	if (bIsLocal)
		return RelevantPoints;

	TArray<FVector> LocalRelevantPoints;
	for (const FVector& Vector : RelevantPoints)
	{
		LocalRelevantPoints.Add(Vector - Origin);
	}
	return LocalRelevantPoints;
}

TArray<FVector> FLocationCache::GetAsGlobal() const
{
	if (!bIsLocal)
		return RelevantPoints;

	TArray<FVector> GlobalRelevantPoints;
	for (const FVector& Vector : RelevantPoints)
	{
		GlobalRelevantPoints.Add(Vector + Origin);
	}
	return GlobalRelevantPoints;
}

void FLocationCache::SetNewOrigin(const FVector& InOrigin)
{
	MakeLocal();
	Origin = InOrigin;
}

void FLocationCache::MakeLocal()
{
	if (bIsLocal)
		return;
	
	for (FVector& Vector : RelevantPoints)
	{
		Vector -= Origin;
	}
	bIsLocal = true;
}

void FLocationCache::MakeGlobal()
{
	if (!bIsLocal)
		return;
	
	for (FVector& Vector : RelevantPoints)
	{
		Vector += Origin;
	}
	bIsLocal = false;
}

void FLocationCache::Clear()
{
	RelevantPoints.Empty();
	Origin = FVector();
	bIsLocal = false;
}

void FLocationCache::AddPoint(const FVector& Point, bool bLocal)
{
	FVector AdjustedPoint = Point;
	if (bIsLocal && !bLocal)
		AdjustedPoint -= Origin;
	if (!bIsLocal && bLocal)
		AdjustedPoint += Origin;
	
	RelevantPoints.AddUnique(AdjustedPoint);
}

ARegionPOI::ARegionPOI()
{
	bIsEditorOnlyActor = true;
#if WITH_EDITOR
	// POITypeProcessor = NewObject<UGridPOI>();

	RootComponent = Root = CreateDefaultSubobject<USceneComponent>("Root");
	
	Billboard = CreateDefaultSubobject<UBillboardComponent>("Billboard");
	Billboard->SetupAttachment(Root);
	
	POIRenderer = CreateDefaultSubobject<UPOIRenderingComponent>("POIRenderer");
	POIRenderer->SetupAttachment(Root);
#endif
}

#if WITH_EDITOR

void ARegionPOI::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	Recalculate();
	POIRenderer->MarkRenderStateDirty();

	if (URegionSettings::Get()->bAutoBakePOIs)
		BakeRegion();
}

void ARegionPOI::PostEditMove(bool bFinished)
{
	Super::PostEditMove(bFinished);

	if (!bFinished)
		return;
	
	Recalculate();
	ReevaluateRegion();
	POIRenderer->MarkRenderStateDirty();

	if (URegionSettings::Get()->bAutoBakePOIs)
		BakeAllRegions();
}

FGameplayTag ARegionPOI::GetContainingRegion() const
{
	return ContainingRegion;
}

FRegionPOIData ARegionPOI::GetData() const
{
	FRegionPOIData RegionData;
	RegionData.Location = GetActorLocation();
	RegionData.RelevantLocations = RelevantPoints;
	return RegionData;
}

void ARegionPOI::BakeRegion()
{
	URegionSubsystem* RegionSubsystem = URegionSubsystem::Get(this);
	if (!RegionSubsystem)
		return;

	RegionSubsystem->BakeRegion(ContainingRegion);
}

void ARegionPOI::BakeAllRegions()
{
	URegionSubsystem* RegionSubsystem = URegionSubsystem::Get(this);
	if (!RegionSubsystem)
		return;

	RegionSubsystem->BakeRegions();
}

void ARegionPOI::ReevaluateRegion()
{
	URegionSubsystem* RegionSubsystem = URegionSubsystem::Get(this);
	if (IsValid(RegionSubsystem))
		ContainingRegion = RegionSubsystem->GetRegionTagByLocation(GetActorLocation());
}

void ARegionPOI::Recalculate()
{
	if (POITypeProcessor)
	{
		RelevantPoints.Clear();
		RelevantPoints = POITypeProcessor->GetPOIRelevantPoints(this);
		POIRenderer->MarkRenderStateDirty();
	}	
}
#endif