#include "RegionVolume.h"

#include "EngineUtils.h"
#include "GameplayTagsManager.h"
#include "NavigationSystem.h"
#include "RegionFunctionLibrary.h"
#include "RegionSubsystem.h"
#include "RegionSystem.h"
#include "RegionTags.h"
#include "RegionTracker.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/OverlapResult.h"
#include "Extensions/GameplayTagExtensions.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Settings/RegionSettings.h"

#if WITH_EDITOR
#include "Components/TextRenderComponent.h"
#include "Misc/DataValidation.h"
#include "GameplayTagsEditorModule.h"
#include "POI/RegionPOI.h"
#endif

#if WITH_EDITOR
const FColor ARegionVolume::DefaultVolumeColor = FColor::Cyan;
const int32 ARegionVolume::DefaultVolumeThickness = 0;
#endif

ARegionVolume::ARegionVolume()
{
	RegionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("AreaBox"));
	RootComponent = RegionBox;
	RegionBox->SetCollisionProfileName(TEXT("Trigger"));
	RegionBox->SetBoxExtent(FVector::One() * 100, false);
	// RegionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	// RegionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RegionBox->SetCollisionResponseToAllChannels(ECR_Overlap);
	RegionBox->SetGenerateOverlapEvents(true);
	RegionBox->SetCanEverAffectNavigation(false);
	RegionBox->ShapeColor = FColor::Cyan;

#if WITH_EDITOR
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
#endif
}

#if WITH_EDITOR
void ARegionVolume::ForceAutoTag()
{
	RegionTag = ParentRegionTag = FGameplayTag();
	URegionSubsystem* RegionSubsystem = URegionSubsystem::Get(this);
	if (!RegionSubsystem)
	{
		return;
	}

	RegionTag = ParentRegionTag = RegionSubsystem->GetRegionTagByVolume(GetActorLocation(), RegionBox->GetScaledBoxExtent());
	bAutoSetRegion = true;

	//Calculate RegionType
	RegionType = GetRegionType();
}

void ARegionVolume::ForceParentTag()
{
	FGameplayTag PreviousTag = RegionTag;
	RegionTag = ParentRegionTag = FGameplayTag();
	URegionSubsystem* RegionSubsystem = URegionSubsystem::Get(this);
	if (!RegionSubsystem)
	{
		return;
	}

	ParentRegionTag = RegionSubsystem->GetRegionTagByVolume(GetActorLocation(), RegionBox->GetScaledBoxExtent());
	RegionTag = PreviousTag;

	if (ParentRegionTag.IsValid() && !RegionTag.MatchesTag(ParentRegionTag))
	{
		RegionTag = ParentRegionTag;
		bAutoSetRegion = true;
	}

	//Calculate RegionType
	RegionType = GetRegionType();
}

void ARegionVolume::SetExtension()
{
	if (!RegionTag.IsValid())
	{
		return;
	}

	//Remove Leading "."
	FString TrimmedExtension = CustomRegionExtension;
	while (TrimmedExtension.StartsWith(TEXT(".")))
	{
		TrimmedExtension.RightChopInline(1);
	}

	if (TrimmedExtension.IsEmpty())
	{
		return;
	}

	//Get or create Tag
	FString NewTagString = RegionTag.ToString() + "." + TrimmedExtension;
	FGameplayTag NewTag = UGameplayTagsManager::Get().RequestGameplayTag(FName(NewTagString), false);
	if (!NewTag.IsValid())
	{
		IGameplayTagsEditorModule::Get().AddNewGameplayTagToINI(NewTagString);
		NewTag = UGameplayTagsManager::Get().RequestGameplayTag(FName(NewTagString), false);
	}

	if (NewTag.IsValid())
	{
		RegionTag = NewTag;
		CustomRegionExtension = "";

		FPropertyChangedEvent PropertyChangedEvent(FindFieldChecked<FProperty>(StaticClass(), GET_MEMBER_NAME_CHECKED(ARegionVolume, RegionTag)));
		PostEditChangeProperty(PropertyChangedEvent);
	}
}

void ARegionVolume::Bake()
{
	POIData.Empty();

	TArray<ARegionPOI*> AllPOIs;
	for (TActorIterator<ARegionPOI> It(GetWorld()); It; ++It)
	{
		ARegionPOI* POI = *It;
		if (!POI)
		{
			continue;
		}

		POI->ReevaluateRegion();
		if (POI->GetContainingRegion().MatchesTagExact(GetRegionTag()))
		{
			POIData.Add(POI->GetData());
		}
	}

	//FallBack
	if (POIData.Num() <= 0)
	{
		UNavigationSystemV1* System = UNavigationSystemV1::GetNavigationSystem(this);
		if (!System)
		{
			return;
		}

		FNavLocation Location;
		System->ProjectPointToNavigation(GetActorLocation(), Location, RegionBox->GetScaledBoxExtent());

		FRegionPOIData Data;
		Data.Location = Location;
		POIData.Add(Data);
	}
}

void ARegionVolume::HideVolume()
{
	RegionBox->SetVisibility(false);
	RegionText->SetVisibility(false);
}

void ARegionVolume::ShowVolume()
{
	RegionBox->SetVisibility(true);
	RegionText->SetVisibility(true);
	RegionBox->SetLineThickness(DefaultVolumeThickness);
	RegionBox->ShapeColor = DefaultVolumeColor;
}

void ARegionVolume::SetHighlight(uint8 State)
{
	ShowVolume();
	if (State == 0)
	{
		RegionBox->SetLineThickness(5);
		RegionBox->ShapeColor = DefaultVolumeColor;
	}
	if (State == 1)
	{
		RegionBox->SetLineThickness(DefaultVolumeThickness);
		RegionBox->ShapeColor = FColor::Red;
	}
}

void ARegionVolume::SetRegionText()
{
	if (IsValid(RegionText))
	{
		FString RegionName = "";
		if (RegionTags::Areas::Name.GetTag().MatchesTag(GetRegionTag()))
		{
			RegionName = "INVALID";
		}
		else
		{
			RegionName = UGameplayTagExtensions::RemoveParentTagFromName(RegionTag, RegionTags::Areas::Name).ToString();
		}
		RegionText->SetText(FText::FromString(RegionName));
		RegionText->SetWorldRotation(FRotator(90, 0, 0));
		RegionText->SetRelativeLocation(FVector(0, 0, RegionBox->GetUnscaledBoxExtent().Z));
		RegionText->SetWorldScale3D(FVector(GetTextScale()));
	}
}
#endif

void ARegionVolume::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	RegisterSelfWithSubsystem();

#if WITH_EDITOR
	SetRegionText();

	if (!RegionTag.IsValid() || RegionTags::Name.GetTag().MatchesTag(RegionTag))
	{
		RegionTag = RegionTags::Areas::Name;

		FPropertyChangedEvent PropertyChangedEvent(FindFieldChecked<FProperty>(StaticClass(), GET_MEMBER_NAME_CHECKED(ARegionVolume, RegionTag)));
		PostEditChangeProperty(PropertyChangedEvent);
	}
#endif
}

void ARegionVolume::Destroyed()
{
	DeregisterSelfWithSubsystem();

	Super::Destroyed();
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

TArray<UObject*> ARegionVolume::GetOverlappingInterfaceObjects(TSubclassOf<UInterface> InterfaceClass) const
{
	if (!InterfaceClass)
		return TArray<UObject*>();
	
	TArray<AActor*> OverlappingActors = GetAllActorsInRegion();
	TArray<UObject*> ReturnArray;
	
	for (AActor* Actor : OverlappingActors)
	{
		if (Actor && Actor->GetClass()->ImplementsInterface(InterfaceClass))
			ReturnArray.Add(Actor);
		
		for (UActorComponent* Component : Actor->GetComponentsByInterface(InterfaceClass))
			ReturnArray.Add(Component);
	}

	return ReturnArray;
}

#if WITH_EDITOR
void ARegionVolume::PostEditMove(bool bFinished)
{
	Super::PostEditMove(bFinished);

	if (bAutoSetRegion)
	{
		ForceAutoTag();
	}
	else
	{
		ForceParentTag();
	}
}

void ARegionVolume::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	ForceParentTag();
	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(ARegionVolume, RegionTag))
	{
		if (ParentRegionTag.IsValid() && !RegionTag.MatchesTag(ParentRegionTag))
		{
			RegionTag = ParentRegionTag;
		}
		else
		{
			bAutoSetRegion = false;
		}

		if (!RegionTag.IsValid())
		{
			bAutoSetRegion = true;
		}

		//Calculate RegionType
		RegionType = GetRegionType();
	}

	if (IsValid(RegionText))
	{
		SetRegionText();
	}
}
#endif

TArray<AActor*> ARegionVolume::GetAllActorsInRegion() const
{
	TArray<AActor*> OutActors {};
	
	bool bForceBoxCheck = false;

	if (const URegionSettings* Settings = URegionSettings::Get())
		bForceBoxCheck = Settings->bForceVolumeBoxChecks;

#if WITH_EDITOR
	if (GetWorld() && GetWorld()->WorldType == EWorldType::Editor)
		bForceBoxCheck = true;
#endif

	if (bForceBoxCheck)
	{
		TArray<FOverlapResult> OverlapResults;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);

		GetWorld()->OverlapMultiByObjectType(
			OverlapResults,
			RegionBox->GetComponentLocation(),
			GetActorRotation().Quaternion(),
			FCollisionObjectQueryParams(FCollisionObjectQueryParams::AllObjects),
			FCollisionShape::MakeBox(RegionBox->GetScaledBoxExtent()),
			QueryParams
		);
		
		#if WITH_EDITOR
		if (GetWorld() && GetWorld()->WorldType == EWorldType::Editor)
		{
			DrawDebugBox(
				GetWorld(),
				RegionBox->GetComponentLocation(),
				RegionBox->GetScaledBoxExtent(),
				GetActorRotation().Quaternion(),
				FColor::Green,
				false,
				2.0f
			);
		}
		#endif

		for (auto OverlapResult : OverlapResults)
		{
			OutActors.Add(OverlapResult.GetActor());
		}
	}
	else if (RegionBox)
	{
		RegionBox->GetOverlappingActors(OutActors);
	}
	
	return OutActors;
}

FGameplayTag ARegionVolume::GetRegionTag() const
{
	return RegionTag;
}

int8 ARegionVolume::GetRegionDepth() const
{
	int32 Depth = UGameplayTagExtensions::CreatePartsFromTag(RegionTag).Num();
	return Depth;
}

ERegionTypes ARegionVolume::GetRegionType() const
{
	return URegionFunctionLibrary::GetRegionTypeByTag(GetRegionTag());
}

bool ARegionVolume::Contains(FVector Location) const
{
	//So that region doesn't find itself when forcing Auto Region
	if (!RegionTag.IsValid())
	{
		return false;
	}

	const FTransform BoxTransform = RegionBox->GetComponentTransform();
	const FVector LocalPoint = BoxTransform.InverseTransformPosition(Location);
	const FVector BoxExtent = RegionBox->GetUnscaledBoxExtent();

	return FMath::Abs(LocalPoint.X) <= BoxExtent.X &&
		FMath::Abs(LocalPoint.Y) <= BoxExtent.Y &&
		FMath::Abs(LocalPoint.Z) <= BoxExtent.Z;
}

bool ARegionVolume::ContainsFully(const FVector Location, const FVector BoxExtent) const
{
	if (!RegionTag.IsValid())
	{
		return false;
	}

	const FTransform BoxTransform = RegionBox->GetComponentTransform();
	const FVector LocalCenter = BoxTransform.InverseTransformPosition(Location);
	const FVector LocalTestExtent = BoxTransform.InverseTransformVector(BoxExtent).GetAbs();
	const FVector RegionBoxExtent = RegionBox->GetUnscaledBoxExtent();

	return (FMath::Abs(LocalCenter.X) + LocalTestExtent.X <= RegionBoxExtent.X) &&
		(FMath::Abs(LocalCenter.Y) + LocalTestExtent.Y <= RegionBoxExtent.Y) &&
		(FMath::Abs(LocalCenter.Z) + LocalTestExtent.Z <= RegionBoxExtent.Z);
}

TArray<FRegionPOIData> ARegionVolume::GetPOIData() const
{
	if (POIData.Num() <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("REGION VOLUME HAD NO POI!"))
	}
	return POIData;
}

FVector ARegionVolume::GetBoxExtent() const
{
	return RegionBox->GetScaledBoxExtent();
}

void ARegionVolume::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                   bool bFromSweep, const FHitResult& SweepResult)
{
	if (URegionTracker* Tracker = OtherActor->GetComponentByClass<URegionTracker>())
	{
		URegionSubsystem* Subsystem = URegionSubsystem::Get(this);
		if (Subsystem)
		{
			Subsystem->EnterRegionVolume(Tracker, this);
		}
	}
}

void ARegionVolume::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (URegionTracker* Tracker = OtherActor->GetComponentByClass<URegionTracker>())
	{
		URegionSubsystem* Subsystem = URegionSubsystem::Get(this);
		if (Subsystem)
		{
			Subsystem->ExitRegionVolume(Tracker, this);
		}
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
	return 2;
}

#if WITH_EDITOR
EDataValidationResult ARegionVolume::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

	if (!IsTemplate())
	{
		if (!RegionTag.IsValid() || RegionTag.MatchesTagExact(RegionTags::Areas::Name))
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::FromString("Region Tag not set!"));
		}
		// if (POIData.Num() <= 0)
		// {
		// 	Context.AddWarning(FText::FromString("Region has no POIs!"));
		// }
	}

	return Result;
}
#endif
