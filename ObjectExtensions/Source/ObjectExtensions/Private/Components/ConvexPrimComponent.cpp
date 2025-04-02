#include "Components/ConvexPrimComponent.h"

FPrimitiveSceneProxy* UConvexPrimComponent::CreateSceneProxy()
{
	class FConvexPrimRenderProxy : public FPrimitiveSceneProxy {
	public:
		FConvexPrimRenderProxy(const UConvexPrimComponent* InComponent) : FPrimitiveSceneProxy(InComponent)
		{
			if (InComponent)
			{
				bDrawBounds = InComponent->DrawBounds;
				Height = InComponent->Height.Z;
				BoundsType = InComponent->PrimType;
				CircleRadius = InComponent->CircleRadius.Size2D();
				BoxExtent2D = FVector2D(InComponent->BoxExtent);
				DrawColor = InComponent->DrawColor;
					
				bPolyTriangulationValid = false;
				PolyOutline.Reserve(InComponent->PolygonPoints.Num());
				for (const FVector& Point3D : InComponent->PolygonPoints)
				{
					PolyOutline.Add(FVector2D(Point3D));
				}
			}
			else
			{
				bDrawBounds = false;
			}
		}

		virtual SIZE_T GetTypeHash() const override 
		{
			static size_t UniquePointer;
			return reinterpret_cast<size_t>(&UniquePointer);
		}
		
		virtual uint32 GetMemoryFootprint() const override 
		{
			return (sizeof(*this) + GetAllocatedSize());
		}

		static void DrawDashedLine(FPrimitiveDrawInterface* PDI, const FVector& Start, const FVector& End, const FLinearColor& Color, double DashSize, uint8 DepthPriority, float LineThickness, float DepthBias, bool bScreenSpace)
		{
			FVector LineDir = End - Start;
			double LineLeft = (End - Start).Size();
			if (LineLeft)
			{
				LineDir /= LineLeft;
			}

			const int32 nLines = FMath::CeilToInt32(LineLeft / (DashSize*2));
			PDI->AddReserveLines(DepthPriority, nLines, DepthBias != 0);

			const FVector Dash = (DashSize * LineDir);

			FVector DrawStart = Start;
			while (LineLeft > DashSize)
			{
				const FVector DrawEnd = DrawStart + Dash;

				PDI->DrawLine(DrawStart, DrawEnd, Color, DepthPriority, LineThickness, DepthBias, bScreenSpace);

				LineLeft -= 2*DashSize;
				DrawStart = DrawEnd + Dash;
			}
			if (LineLeft > 0.0f)
			{
				const FVector DrawEnd = End;

				PDI->DrawLine(DrawStart, DrawEnd, Color, DepthPriority, LineThickness, DepthBias, bScreenSpace);
			}
		}

		virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override 
		{
			if (!bDrawBounds) 
			{
				return;
			}

			float LineThickness = 0;
			FLinearColor LineColor = DrawColor;
			constexpr float DashSize = 34;
			const FMatrix Matrix = GetLocalToWorld();
			const FVector AxisX = Matrix.GetUnitAxis(EAxis::X);
			const FVector AxisY = Matrix.GetUnitAxis(EAxis::Y);
			const FVector AxisZ = Matrix.GetUnitAxis(EAxis::Z);
			constexpr float DepthBias = 0;
			constexpr bool bDrawScreenSpace = false;
			constexpr uint8 DepthPriorityGroup = SDPG_Foreground;

			if (!IsSelected()) 
			{
				LineThickness = 2;
				LineColor.A = 0.5f;
			}
			
			for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++) 
			{
				if (VisibilityMap & (1 << ViewIndex)) 
				{
					FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);
					if (BoundsType == EConvexPrimType::Square) 
					{
						const float HalfHeight = Height * 0.5f;
						const FVector BoxCenter = GetLocalToWorld().TransformPosition(FVector(0, 0, HalfHeight));
						const FVector Extent = FVector(BoxExtent2D, HalfHeight);
						
						DrawOrientedWireBox(PDI, BoxCenter, AxisX, AxisY, AxisZ, Extent, LineColor, DepthPriorityGroup, LineThickness, DepthBias, bDrawScreenSpace);
					}
					else if (BoundsType == EConvexPrimType::Circle) 
					{
						constexpr int32 NumSides = 32;
						FVector BasePoint = GetLocalToWorld().TransformPosition(FVector::Zero());
						FVector TopPoint = GetLocalToWorld().TransformPosition(FVector(0, 0, Height));
						
						DrawCircle(PDI, BasePoint, AxisX, AxisY, LineColor, CircleRadius, NumSides, DepthPriorityGroup, LineThickness, DepthBias, bDrawScreenSpace);
						DrawCircle(PDI, TopPoint, AxisX, AxisY, LineColor, CircleRadius, NumSides, DepthPriorityGroup, LineThickness, DepthBias, bDrawScreenSpace);

						constexpr float AngleDelta = PI * 2.0f / NumSides; 
						for (int i = 0; i < NumSides; i++) 
						{
							const float AngleRad = i * AngleDelta;
							FVector Lower(FMath::Cos(AngleRad) * CircleRadius, FMath::Sin(AngleRad) * CircleRadius, 0);
							FVector Upper(Lower.X, Lower.Y, Height);
							Lower = GetLocalToWorld().TransformPosition(Lower);
							Upper = GetLocalToWorld().TransformPosition(Upper);
							DrawDashedLine(PDI, Lower, Upper, LineColor, DashSize, DepthPriorityGroup, LineThickness, DepthBias, bDrawScreenSpace);
						}
					}
					else if (BoundsType == EConvexPrimType::Custom) 
					{
						if (bPolyTriangulationValid) 
						{
							const FMatrix LocalToWorld = GetLocalToWorld();
							for (int i = 0; i < PolyOutline.Num(); i++) 
							{
								FVector2D PolyPoint0 = PolyOutline[i];
								FVector2D PolyPoint1 = PolyOutline[(i + 1) % PolyOutline.Num()];
								FVector Lo0 = LocalToWorld.TransformPosition(FVector(PolyPoint0, 0));
								FVector Hi0 = LocalToWorld.TransformPosition(FVector(PolyPoint0, Height));
								FVector Lo1 = LocalToWorld.TransformPosition(FVector(PolyPoint1, 0));
								FVector Hi1 = LocalToWorld.TransformPosition(FVector(PolyPoint1, Height));

								PDI->DrawLine(Lo0, Lo1, LineColor, DepthPriorityGroup, LineThickness, DepthBias, bDrawScreenSpace);
								PDI->DrawLine(Hi0, Hi1, LineColor, DepthPriorityGroup, LineThickness, DepthBias, bDrawScreenSpace);
								DrawDashedLine(PDI, Lo0, Hi0, LineColor, DashSize, DepthPriorityGroup, LineThickness, DepthBias, bDrawScreenSpace);
							} 
						}
						else if (PolyOutline.Num() >= 3) 
						{
							// The triangulation is invalid.  Draw the outline indicating it
							for (int i = 0; i < PolyOutline.Num(); i++) 
							{
								FVector2D PolyPoint0 = PolyOutline[i];
								FVector2D PolyPoint1 = PolyOutline[(i + 1) % PolyOutline.Num()];
								FVector Lo0 = GetLocalToWorld().TransformPosition(FVector(PolyPoint0, 0));
								FVector Lo1 = GetLocalToWorld().TransformPosition(FVector(PolyPoint1, 0));
								DrawDashedLine(PDI, Lo0, Lo1, FLinearColor(1, 0.25f, 0, 1), DashSize, DepthPriorityGroup, 0, DepthBias, true);
							} 
						}
					}
				}
			}			
		}		
		
		virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
		{
			FPrimitiveViewRelevance Result;
			Result.bDrawRelevance = IsShown(View);
			Result.bDynamicRelevance = true;
			Result.bShadowRelevance = IsShadowCast(View);
			Result.bEditorPrimitiveRelevance = UseEditorCompositing(View);
			return Result;
		}

	private:
		struct FConvexPoly
		{
			TArray<FVector2D> Points;
		};
		bool bDrawBounds {};
		EConvexPrimType BoundsType {};
		TArray<FConvexPoly> ConvexPolys;
		TArray<FVector2D> PolyOutline;
		bool bPolyTriangulationValid;
		FVector2D BoxExtent2D;
		float CircleRadius;
		float Height {};
		FLinearColor DrawColor;
	};

	return new FConvexPrimRenderProxy(this);
}

FBoxSphereBounds UConvexPrimComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	FBox LocalBounds(ForceInit);
	const float HalfHeight = Height.Z * 0.5;
	if (PrimType == EConvexPrimType::Square)
	{
		FVector Extent = BoxExtent;
		Extent.Z = HalfHeight;
		const FVector Center(0, 0, HalfHeight);
		LocalBounds = FBox(Center - Extent, Center + Extent);
	}
	else if (PrimType == EConvexPrimType::Circle)
	{
		const float ExtentDistance = CircleRadius.Size2D() * 1.4142135f;
		const FVector Extent(ExtentDistance, ExtentDistance, HalfHeight);
		const FVector Center(0, 0, HalfHeight);
		LocalBounds = FBox(Center - Extent, Center + Extent);
	}
	else if (PrimType == EConvexPrimType::Custom)
	{
		for (const FVector& Point : PolygonPoints)
		{
			LocalBounds += Point;
		}
		LocalBounds += FVector(0, 0, Height.Z);
	}
	else
	{
		return Super::CalcBounds(LocalToWorld);
	}

	const FBox WorldBox = LocalBounds.TransformBy(GetComponentTransform());
	return FBoxSphereBounds(WorldBox);
}
