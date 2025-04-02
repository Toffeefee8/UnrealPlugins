#pragma once

#include <initializer_list>

#include "CoreMinimal.h"
#include "Components/PrimitiveComponent.h"
#include "ConvexPrimComponent.generated.h"

//THIS IS EXPERIMENTAL
UENUM(BlueprintType)
enum class EConvexPrimType : uint8
{
	Circle,
	Square,
	Custom
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class OBJECTEXTENSIONS_API UConvexPrimComponent : public UPrimitiveComponent
{
	GENERATED_BODY()

public:

	virtual bool IsEditorOnly() const override { return true; }
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool DrawBounds = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(MakeEditWidget=true))
	FLinearColor DrawColor = FLinearColor(1, 0, 1, 1);
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EConvexPrimType PrimType = EConvexPrimType::Circle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Height = FVector(0, 0, 200);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "Primtype == EConvexPrimType::Circle", EditConditionHides))
	FVector CircleRadius = FVector(200, 0, 0);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "Primtype == EConvexPrimType::Square", EditConditionHides))
	FVector BoxExtent = FVector(200, 200, 0);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "Primtype == EConvexPrimType::Custom", EditConditionHides))
	TArray<FVector> PolygonPoints;
};
