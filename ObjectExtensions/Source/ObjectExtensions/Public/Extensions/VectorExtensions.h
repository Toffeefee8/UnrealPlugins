#pragma once

#include "CoreMinimal.h"
#include "HelperFunctionLibrary.h"
#include "Enums/GlobalDirection.h"
#include "Math/IntVector.h"


static void BoxToLocationAndExtent(FBox2f Box, FVector2f& Center, FVector2f& Extent)
{
	const FVector2f Min = Box.Min;
	const FVector2f Max= Box.Max;
	
	Center = (Min + Max) * 0.5f;
	Extent = (Max - Min) * 0.5f;
};

static EGlobalDirection GetVectorAsGlobalDir(FVector2f VectorToConvert)
{
	// Normalize the vector to avoid any issues with vector lengths
	VectorToConvert.Normalize();

	// Define unit direction vectors for comparison
	FVector2f Left(-1, 0);
	FVector2f Right(1, 0);
	FVector2f Up(0, 1);
	FVector2f Down(0, -1);

	// Calculate dot products to determine the closest direction
	float DotLeft = FVector2f::DotProduct(VectorToConvert, Left);
	float DotRight = FVector2f::DotProduct(VectorToConvert, Right);
	float DotUp = FVector2f::DotProduct(VectorToConvert, Up);
	float DotDown = FVector2f::DotProduct(VectorToConvert, Down);

	// Find the maximum dot product value
	float MaxDot = FMath::Max(DotLeft, DotRight);
	MaxDot = FMath::Max(MaxDot,DotUp);
	MaxDot = FMath::Max(MaxDot,DotDown);

	// Return the direction with the highest dot product
	if (MaxDot == DotLeft)
	{
		return EGlobalDirection::Left;
	}
	if (MaxDot == DotRight)
	{
		return EGlobalDirection::Right;
	}
	if (MaxDot == DotUp)
	{
		return EGlobalDirection::Up;
	}
	if (MaxDot == DotDown)
	{
		return EGlobalDirection::Down;
	}

	// Return None if somehow no direction matches (this should not happen)
	return EGlobalDirection::None;
}

static FIntVector2 AsFintVec2(FVector Vector)
{
	return FIntVector2(FMath::RoundToInt(Vector.X), FMath::RoundToInt(Vector.Y));
}

static TArray<FIntVector2> AsFintVec2(TArray<FVector>  Array)
{
	TArray<FIntVector2> Result;
	for(FVector Vector:Array)
	{
		Result.Add(AsFintVec2(Vector));
	}
	
	return Result;
}

static FIntVector2 AsFintVec2(FVector2f Vector2)
{
	return FIntVector2(Vector2.X,Vector2.Y);
}

static TArray<FIntVector2>  AsFintVec2(TArray<FVector2f> Array)
{
	TArray<FIntVector2> Result;
	for(FVector2f Vector2:Array)
	{
		Result.Add(AsFintVec2(Vector2));
	}
	
	return Result;
}

static FVector2f AsFVector2f(FVector Vector2)
{
	return FVector2f(Vector2.X,Vector2.Y);
}

static FVector2f AsFVector2f(FIntVector2 Vector2)
{
	return FVector2f(Vector2.X,Vector2.Y);
}

static TArray<FVector2f> AsFVector2f(TArray<FIntVector2> Array)
{
	TArray<FVector2f> Result;
	for(const FIntVector2 Vector2:Array)
	{
		Result.Add(AsFVector2f(Vector2));
	}
	
	return Result;
}



static FIntVector AsFIntVec(const FVector& Vector)
{
	return FIntVector(FMath::RoundToInt(Vector.X) ,FMath::RoundToInt(Vector.Y), FMath::RoundToInt(Vector.Z));
}

static FIntVector AsFIntVec(const FIntVector2& Vector, const float Z)
{
	return FIntVector(Vector.X, Vector.Y, Z);
}


static TArray<FIntVector> AsFIntVec(const TArray<FIntVector2>& Array, const float Z)
{
	TArray<FIntVector> Result;
	for(FIntVector2 Vector2:Array)
	{
		Result.Add(AsFIntVec(Vector2,Z));
	}
	
	return Result;
}

static TArray<FIntVector> AsFIntVec(const TArray<FIntVector2>& Array)
{
	TArray<FIntVector> Result;
	for(FIntVector2 Vector2:Array)
	{
		Result.Add(AsFIntVec(Vector2,0));
	}
	
	return Result;
}

static TArray<FIntVector> AsFIntVec(const TArray<FVector>& Array)
{
	TArray<FIntVector> Result;
	Result.Reserve(Array.Num());

	for (const auto& Element : Array)
	{
		Result.Add(AsFIntVec(Element));
	}

	return Result;
}

static FIntVector AsFIntVec3(const FIntVector2& Vector2)
{
	return FIntVector(Vector2.X, Vector2.Y, 0);
}

static TArray<FIntVector> AsFIntVec3(const TArray<FIntVector2>& Array)
{
	TArray<FIntVector> Result;
	Result.Reserve(Array.Num());

	for (const auto& Element : Array)
	{
		Result.Add(AsFIntVec3(Element));
	}

	return Result;
}

static FVector AsFVec(const FVector2f& Vector2, const float Z = 0)
{
	return FVector(Vector2.X, Vector2.Y, Z);
}



static FVector AsFVec(const FIntVector2& Vector2, const float Z)
{
	return FVector(Vector2.X, Vector2.Y, Z);
}

static TArray<FVector> AsFVec(const TArray<FIntVector2>& Array, const float Z)
{
	TArray<FVector> ConvertedArray;
	for (FIntVector2 Vector: Array)
	{
		ConvertedArray.Add(AsFVec(Vector,Z));
	}
	
	return ConvertedArray;
}

static FVector AsFVec(const FIntVector& Vector)
{
	return FVector(Vector.X,Vector.Y,Vector.Z);
}

static FVector AsFVec(const FIntVector2& Vector2)
{
	return AsFVec(Vector2,0);
}

static TArray<FVector> AsFVec(const TArray<FVector2f>& Array)
{
	TArray<FVector> Result;
	Result.Empty();
	//Result.Reserve(Array.Num());

	for (const auto& Element : Array)
	{
		Result.Add(AsFVec(Element,0));
	}

	return Result;
}

static TArray<FVector> AsFVec(const TArray<FVector2f>& Array, const float Z)
{
	TArray<FVector> Result;
	Result.Empty();

	for (const auto& Element : Array)
	{
		Result.Add(AsFVec(Element,Z));
	}

	return Result;
}

static TArray<FVector> AsFVec(const TArray<FIntVector2>& Array)
{
	TArray<FVector> Result;
	//Result.Reserve(Array.Num());
	Result.Empty();

	for (const auto& Element : Array)
	{
		Result.Add(AsFVec(Element));
	}

	return Result;
}


static int MaxXValue(const TArray<FIntVector2>& Positions)
{
	int maxX = MIN_int32;

	for (const FIntVector2& Tile : Positions)
	{
		if (Tile.X > maxX)
		{
			maxX = Tile.X;
		}
	}

	return maxX;
}

static int MaxYValue(const TArray<FIntVector2>& Positions)
{
	int maxY = MIN_int32;

	for (const FIntVector2& Tile : Positions)
	{
		if (Tile.Y > maxY)
		{
			maxY = Tile.Y;
		}
	}

	return maxY;
}

static FIntVector2 RoundVectorToInt(const FVector2f Vector)
{
	return FIntVector2(FMath::RoundToInt(Vector.X),FMath::RoundToInt(Vector.Y));
}

static TArray<FIntVector2>  RoundVectorToInt(const TArray<FVector2f>  Array)
{
	TArray<FIntVector2> ReturnArray;
	ReturnArray.Empty();
	for (const FVector2f Vector:Array)
	{
		ReturnArray.Add(RoundVectorToInt(Vector));
	}
	
	return ReturnArray;
}

static FIntVector RoundVectorToInt(const FVector Vector)
{
	return  FIntVector(
					FMath::RoundToInt(Vector.X),
					FMath::RoundToInt(Vector.Y),
					FMath::RoundToInt(Vector.Z));
}

template<typename Value>
static TMap<FIntVector,Value> RoundVectorToInt(TMap<FVector,Value> Map)
{
	TMap<FIntVector,Value> RoundedVectorMap;
	RoundedVectorMap.Empty();
	for (TTuple<FVector,Value> Item: Map)
	{
		const FVector Vector = Item.Key;
		FIntVector RoundedVector = RoundVectorToInt(Vector);
		
		RoundedVectorMap.Add(RoundedVector,Item.Value);
	}

	return RoundedVectorMap;
}

static TArray<FIntVector2> MultiplyVectors (const TArray<FVector2f> VectorArray, const float Multiplier)
{
	TArray<FIntVector2> MultipliedVectors;
	MultipliedVectors.Empty();
	for (const FVector2f Vector:VectorArray)
	{
		MultipliedVectors.Add(FIntVector2(Vector.X*Multiplier,Vector.Y*Multiplier) );
	}

	return MultipliedVectors;
}

static TArray<FIntVector2> MultiplyVectors (const TArray<FIntVector2> VectorArray, const float Multiplier)
{
	TArray<FIntVector2> MultipliedVectors;
	MultipliedVectors.Empty();
	for (const FIntVector2 Vector:VectorArray)
	{
		MultipliedVectors.Add(Vector*Multiplier);
	}

	return MultipliedVectors;
}

static TArray<FVector> MultiplyVectors (const TArray<FVector> VectorArray, const float Multiplier)
{
	TArray<FVector> MultipliedVectors;
	MultipliedVectors.Empty();
	for (const FVector Vector:VectorArray)
	{
		MultipliedVectors.Add(Vector*Multiplier);
	}

	return MultipliedVectors;
}

static TArray<float> AsFloats (const FVector2f Vector)
{
	TArray<float> FloatArray;
	FloatArray.Empty();
	FloatArray.Add(Vector.X);
	FloatArray.Add(Vector.Y);
	return  FloatArray;
}

static TArray<float> AsFloats (const FIntVector2 Vector)
{
	TArray<float> FloatArray;
	FloatArray.Empty();
	FloatArray.Add(Vector.X);
	FloatArray.Add(Vector.Y);
	return  FloatArray;
}

static TArray<float> AsFloats (const FIntVector Vector)
{
	TArray<float> FloatArray;
	FloatArray.Empty();
	FloatArray.Add(Vector.X);
	FloatArray.Add(Vector.Y);
	FloatArray.Add(Vector.Z);
	return  FloatArray;
}

static TArray<float> AsFloats (const TArray<FIntVector> VectorArray)
{
	TArray<float> FloatArray;
	FloatArray.Empty();
	for (const FIntVector Vector: VectorArray)
	{
		FloatArray.Add(Vector.X);
		FloatArray.Add(Vector.Y);
		FloatArray.Add(Vector.Z);
	}

	return  FloatArray;
}


static TArray<float> AsFloats (const TArray<FVector> VectorArray)
{
	TArray<float> FloatArray;
	FloatArray.Empty();
	for (FVector Vector: VectorArray)
	{
		FloatArray.Add(Vector.X);
		FloatArray.Add(Vector.Y);
		FloatArray.Add(Vector.Z);
	}

	return  FloatArray;
}

static TArray<float> AsFloats (const TArray<FVector2f> VectorArray)
{
	TArray<float> FloatArray;
	FloatArray.Empty();
	for (FVector2f Vector: VectorArray)
	{
		FloatArray.Append(AsFloats(Vector));
	}

	return  FloatArray;
}

static FIntVector2 ScaleToInt (const FVector2f Vector2, int ScaleFactor)
{
	return FIntVector2(FMath::RoundToInt(Vector2.X*ScaleFactor),FMath::RoundToInt(Vector2.Y*ScaleFactor));
}

static TArray<FIntVector2> ScaleToInt (const TArray<FVector2f> VectorArray, int &ScaleFactor, int MaxScaleFactor  = 4)
{
	ScaleFactor =  UHelperFunctionLibrary::GetMaxDecimalPlaces(AsFloats(VectorArray));

	ScaleFactor = FMath::Min(ScaleFactor,MaxScaleFactor);

	TArray<FIntVector2> UpscaledVectors;
	UpscaledVectors.Empty();
	for (const FVector2f Vector2:VectorArray)
	{
		UpscaledVectors.Add(
			FIntVector2(
				FMath::RoundToInt(Vector2.X*ScaleFactor),FMath::RoundToInt(Vector2.Y*ScaleFactor)));
	}

	return UpscaledVectors;
}

static TArray<FIntVector> ScaleToInt (const TArray<FVector> VectorArray, int &ScaleFactor)
{
	ScaleFactor =  UHelperFunctionLibrary::GetMaxDecimalPlaces(AsFloats(VectorArray));

	TArray<FIntVector> UpscaledVectors;
	UpscaledVectors.Empty();
	for (const FVector Vector:VectorArray)
	{
		UpscaledVectors.Add(
			FIntVector(
				FMath::RoundToInt(Vector.X*ScaleFactor),
				FMath::RoundToInt(Vector.Y*ScaleFactor),
				FMath::RoundToInt(Vector.Z*ScaleFactor) ));
	}

	return UpscaledVectors;
}

static TArray<FVector2f> ScaleDown (const TArray<FVector2f> VectorArray, const float ScaleFactor)
{
	TArray<FVector2f> DownscaledVectors;
	DownscaledVectors.Empty();
	for (const FVector2f Vector2:VectorArray)
	{
		DownscaledVectors.Add(
			FVector2f(Vector2.X/ScaleFactor,Vector2.Y/ScaleFactor));
	}

	return DownscaledVectors;
}

static TArray<FVector2f> ScaleDown (const TArray<FIntVector2> VectorArray, const float ScaleFactor)
{
	TArray<FVector2f> DownscaledVectors;
	DownscaledVectors.Empty();
	for (const FIntVector2 Vector2:VectorArray)
	{
		DownscaledVectors.Add(
			FVector2f(Vector2.X/ScaleFactor,Vector2.Y/ScaleFactor));
	}

	return DownscaledVectors;
}

static TArray<FVector> ScaleDown (const TArray<FIntVector> VectorArray, const float ScaleFactor)
{
	TArray<FVector> DownscaledVectors;
	DownscaledVectors.Empty();
	for (const FIntVector Vector2:VectorArray)
	{
		DownscaledVectors.Add(
			FVector(Vector2.X/ScaleFactor,Vector2.Y/ScaleFactor,Vector2.Z/ScaleFactor));
	}

	return DownscaledVectors;
}
