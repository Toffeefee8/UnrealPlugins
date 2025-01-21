// Copyright Phoenix Dawn Development LLC. All Rights Reserved.

#include "RegionVolume.h"

#include "RegionSubsystem.h"
#include "RegionTracker.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/TextRenderComponent.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

ARegionVolume::ARegionVolume()
{
	RegionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("AreaBox"));
	RootComponent = RegionBox;
	RegionBox->SetCollisionProfileName(TEXT("Trigger"));
	RegionBox->SetBoxExtent(FVector::One() * 100, false);
	RegionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	RegionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RegionBox->SetCanEverAffectNavigation(false);
	RegionBox->ShapeColor = FColor::Cyan;

	RegionCenter = CreateDefaultSubobject<USphereComponent>(TEXT("RegionCenter"));
	RegionCenter->SetupAttachment(RegionBox);
	RegionCenter->SetHiddenInGame(true);
	RegionCenter->SetAbsolute(false, false, true); 
	RegionCenter->SetLineThickness(1);
	RegionCenter->ShapeColor = FColor::Cyan;

	RegionText = CreateDefaultSubobject<UTextRenderComponent>("RegionText");
	RegionText->SetupAttachment(RegionBox);
	RegionText->SetHiddenInGame(true);
	RegionText->SetHorizontalAlignment(EHTA_Center);
	RegionText->SetVerticalAlignment(EVRTA_TextCenter);
	RegionText->SetTextRenderColor(FColor::Cyan);
	RegionText->SetAbsolute(false, false, true);
	RegionText->SetWorldScale3D(FVector(GetTextScale()));
	RegionText->SetWorldRotation(FRotator(90, 0, 0));
	RegionText->SetRelativeLocation(FVector(0, 0, RegionBox->GetUnscaledBoxExtent().Z));
}

void ARegionVolume::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (IsValid(RegionText))
	{
		RegionText->SetText(FText::FromString(RegionTag.ToString().StartsWith(TEXT("Game.Region.")) ? RegionTag.ToString().Mid(12) : RegionTag.ToString()));
		RegionText->SetWorldRotation(FRotator(90, 0, 0));
		RegionText->SetRelativeLocation(FVector(0, 0, RegionBox->GetUnscaledBoxExtent().Z));
		RegionText->SetWorldScale3D(FVector(GetTextScale()));
	}
}

void ARegionVolume::BeginPlay()
{
	Super::BeginPlay();

	RegionBox->OnComponentBeginOverlap.AddDynamic(this, &ARegionVolume::OnOverlapBegin);
	RegionBox->OnComponentEndOverlap.AddDynamic(this, &ARegionVolume::OnOverlapEnd);

	RegisterSelfWithSubsystem();
}

void ARegionVolume::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DeregisterSelfWithSubsystem();

	Super::EndPlay(EndPlayReason);
}

FGameplayTag ARegionVolume::GetRegionTag() const
{
	return RegionTag;
}

int32 ARegionVolume::GetRegionDepth() const
{
	return URegionSubsystem::CreatePartsFromTag(RegionTag).Num();
}

bool ARegionVolume::IsParentRegion(FGameplayTag InRegionTag) const
{
	return InRegionTag.MatchesTag(RegionTag);
}

bool ARegionVolume::IsChildRegion(FGameplayTag InRegionTag) const
{
	return RegionTag.MatchesTag(InRegionTag);
}

bool ARegionVolume::IsRegionExact(FGameplayTag InRegionTag) const
{
	return RegionTag.MatchesTagExact(InRegionTag);
}

bool ARegionVolume::Contains(FVector Location) const
{
	const FTransform BoxTransform = RegionBox->GetComponentTransform();
	const FVector LocalPoint = BoxTransform.InverseTransformPosition(Location);
	const FVector BoxExtent = RegionBox->GetScaledBoxExtent();

	return FMath::Abs(LocalPoint.X) <= BoxExtent.X &&
		FMath::Abs(LocalPoint.Y) <= BoxExtent.Y &&
		FMath::Abs(LocalPoint.Z) <= BoxExtent.Z;
}

FVector ARegionVolume::GetBoxExtent() const
{
	return RegionBox->GetScaledBoxExtent();
}

FVector ARegionVolume::GetCenterPoint() const
{
	if (RegionCenter)
	{
		return RegionCenter->GetComponentLocation();
	}

	return GetActorLocation();
}

void ARegionVolume::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                   bool bFromSweep, const FHitResult& SweepResult)
{
	if (URegionTracker* Tracker = OtherActor->GetComponentByClass<URegionTracker>())
	{
		Tracker->AddRegion(this);
	}
}

void ARegionVolume::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (URegionTracker* Tracker = OtherActor->GetComponentByClass<URegionTracker>())
	{
		Tracker->RemoveRegion(this);
	}
}

void ARegionVolume::RegisterSelfWithSubsystem()
{
	if (URegionSubsystem* Subsystem = URegionSubsystem::Get(this))
	{
		Subsystem->RegisterVolume(this);
	}
}

void ARegionVolume::DeregisterSelfWithSubsystem()
{
	if (URegionSubsystem* Subsystem = URegionSubsystem::Get(this))
	{
		Subsystem->DeregisterVolume(this);
	}
}

float ARegionVolume::GetTextScale() const
{
	return 1;
}

#if WITH_EDITOR
void ARegionVolume::DisplayHierarchy()
{
	URegionSubsystem::Get(this)->HighlightHierarchy(this, RegionTag);
}

void ARegionVolume::ClearHighlights()
{
	URegionSubsystem::Get(this)->ResetAllHighlights(this);
}

EDataValidationResult ARegionVolume::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

	if (!IsTemplate())
	{
		if (!RegionTag.IsValid())
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::FromString("Region Tag not set!"));
		}
	}

	return Result;
}
#endif
