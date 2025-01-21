#pragma once

#include "CoreMinimal.h"
#include "UObject/UObjectGlobals.h" 
#include "Containers/Map.h"

template<typename T>
TArray<TWeakObjectPtr<T>> ToWeakObjectPointer(const TArray<T*>& Array)
{
	TArray<TWeakObjectPtr<T>> WeakPointerArray;
	WeakPointerArray.Reserve(Array.Num());  // Reserve space to avoid multiple allocations
    
	for (T* Element : Array)
	{
		WeakPointerArray.Add(Element);  // Automatically converts T* to TWeakObjectPtr<T>
	}
    
	return WeakPointerArray;
}

template<typename T>
TArray<T*> ToRawPtr(const TArray<TWeakObjectPtr<T>>& Array,bool bDiscardInvalidPtrs = true)
{
	TArray<T*> ValidStructures{};
	for (TWeakObjectPtr<T> Structure: Array)
	{
		if (Structure.IsValid()||!bDiscardInvalidPtrs)
			ValidStructures.Add(Structure.Get());
	}
	return ValidStructures;
}

template<typename T>
static void ShuffleArrayWithRandomStream(TArray<T>& Array, FRandomStream RandomStream)
{
	const int32 Count = Array.Num();
	for (int32 i = Count - 1; i > 0; --i)
	{
		// Generate a random index from 0 to i
		const int32 RandomIndex = RandomStream.RandRange(0, i);
        
		// Swap the current element with the randomly chosen one
		Array.Swap(i, RandomIndex);
	}
}

template<typename T>
void RemoveDuplicates(TArray<T>& TargetArray, TFunctionRef<bool(const T&, const T&)> Predicate)
{
	TArray<T> UniqueElements;  // Array to store unique elements

	for (const T& Element : TargetArray)
	{
		bool bIsDuplicate = false;

		for (const T& UniqueElement : UniqueElements)
		{
			if (Predicate(Element, UniqueElement))
			{
				bIsDuplicate = true;
				break;
			}
		}

		if (!bIsDuplicate)
		{
			UniqueElements.Add(Element);
		}
	}

	// Replace the original array with the unique elements
	TargetArray = UniqueElements;
}

template<typename T>
void RemoveDuplicates(TArray<T>& TargetArray)
{
	RemoveDuplicates<T>(TargetArray,[](T A, T B)
	{
		return A == B;
	});
}

/**
 * @brief Transforms a map by applying a function to each key-value pair.
 * 
 * This function iterates through each key-value pair in the provided map and applies
 * the given transformation function. The transformation function should return a new
 * key-value pair with potentially different types, which will be added to the resulting map.
 * 
 * @tparam KeyType The type of keys in the original map.
 * @tparam ValueType The type of values in the original map.
 * @tparam TransformedKeyType The type of keys in the resulting map.
 * @tparam TransformedValueType The type of values in the resulting map.
 * @param Map The original map to transform.
 * @param TransformFunc The transformation function to apply to each key-value pair.
 * @return TMap<TransformedKeyType, TransformedValueType> A new map with transformed key-value pairs.
 * 
 * @example
 * TMap<int32, FString> OriginalMap = {{1, TEXT("One")}, {2, TEXT("Two")}, {3, TEXT("Three")}};
 * TMap<FString, FString> TransformedMap = Select<int32, FString, FString, FString>(
 *     OriginalMap,
 *     [](const TPair<int32, FString>& Pair) { return TPair<FString, FString>(FString::FromInt(Pair.Key), Pair.Value); }
 * );
 * // TransformedMap will contain {"1": "One", "2": "Two", "3": "Three"}.
 */
template <typename KeyType, typename ValueType, typename TransformedKeyType, typename TransformedValueType>
TMap<TransformedKeyType, TransformedValueType> Select(
	const TMap<KeyType, ValueType>& Map,
	TFunction<TPair<TransformedKeyType, TransformedValueType>(const TPair<KeyType, ValueType>&)> TransformFunc)
{
	TMap<TransformedKeyType, TransformedValueType> Result;

	for (const auto& Pair : Map)
		Result.Add(TransformFunc(Pair));

	return Result;
}

/**
 * @brief Transforms an array by applying a function to each element.
 * 
 * This function iterates through each element in the provided array and applies
 * the given transformation function. The transformation function should return a new
 * element with a potentially different type, which will be added to the resulting array.
 * 
 * @tparam InputType The type of elements in the original array.
 * @tparam OutputType The type of elements in the resulting array.
 * @param Array The original array to transform.
 * @param TransformFunc The transformation function to apply to each element.
 * @return TArray<OutputType> A new array with transformed elements.
 * 
 * @example
 * TArray<int32> OriginalArray = {1, 2, 3};
 * TArray<FString> TransformedArray = Select<int32, FString>(
 *     OriginalArray,
 *     [](const int32& Element) { return FString::FromInt(Element); }
 * );
 * // TransformedArray will contain {"1", "2", "3"}.
 */
template <typename InputType, typename OutputType>
	TArray<OutputType> Select(const TArray<InputType>& Array, TFunctionRef<OutputType(const InputType&)> TransformFunc)
{
	TArray<OutputType> Result;
	Result.Reserve(Array.Num());

	for (const auto& Element : Array)
		Result.Add(TransformFunc(Element));

	return Result;
}

/**
 * @brief Filters an array based on a predicate function.
 * 
 * This function iterates through each element in the provided array and applies
 * the given predicate function. If the predicate returns true for an element,
 * that element is added to the resulting array.
 * 
 * @tparam T The type of elements in the array.
 * @param Array The array of elements to filter.
 * @param Predicate The predicate function to apply to each element.
 * @return TArray<T> A new array with elements that satisfy the predicate.
 * 
 * @example
 * TArray<int32> Numbers = {1, 2, 3, 4, 5};
 * TArray<int32> EvenNumbers = Where<int32>(Numbers, [](const int32& Number) { return Number % 2 == 0; });
 * // EvenNumbers will contain {2, 4}.
 */
template <typename T>
static TArray<T> WhereToRef(const TArray<T>& Array, TFunctionRef<bool(const T*&)> Predicate)
{
	TArray<T> Result;
	for (const T* Element : Array)
	{
		if (Predicate(Element))
		{
			Result.Add(Element);
		}
	}
	return Result;
}

/**
 * @brief Filters an array based on a predicate function.
 * 
 * This function iterates through each element in the provided array and applies
 * the given predicate function. If the predicate returns true for an element,
 * that element is added to the resulting array.
 * 
 * @tparam T The type of elements in the array.
 * @param Array The array of elements to filter.
 * @param Predicate The predicate function to apply to each element.
 * @return TArray<T*> A new array with elements that satisfy the predicate.
 * 
 * @example
 * TArray<int32*> Numbers = {new int32(1), new int32(2), new int32(3), new int32(4), new int32(5)};
 * TArray<int32*> EvenNumbers = Where<int32>(Numbers, [](const int32* Number) { return *Number % 2 == 0; });
 * // EvenNumbers will contain pointers to {2, 4}.
 */
template <typename T>
static TArray<T*> WhereToRef(const TArray<T*>& Array, TFunctionRef<bool(const T*)> Predicate)
{
	TArray<T*> Result;
	for (const T* Element : Array)
	{
		if (Predicate(Element))
		{
			Result.Add(const_cast<T*>(Element)); // Remove constness to match TArray<T*>.
		}
	}
	return Result;
}


template <typename T>
static TArray<T> WhereNoRef(const TArray<T>& Array, TFunctionRef<bool(const T&)> Predicate)
{
	TArray<T> Result;
	for (const T Element : Array)
	{
		if (Predicate(Element))
		{
			Result.Add(Element);
		}
	}
	return Result;
}

template <typename T>
static bool ContainsCustomComparism(const TArray<T>& Array, TFunctionRef<bool(const T&)> Predicate)
{
	TArray<T> Result;
	for (const T Element : Array)
	{
		if (Predicate(Element))
		{
			return true;
		}
	}
	return false;
}


template <typename T>
static void Filter(TArray<T>& TargetArray, const TArray<T>& FilterArray)
{
	// Define a predicate that checks if an element is not present in the FilterArray
	auto Predicate = [&FilterArray](const T& Element) -> bool {
		return !FilterArray.Contains(Element);
	};

	// Use WhereNoRef to filter the TargetArray
	Filter<T>(
		TargetArray,
		FilterArray,
		[](const T A, const T B) -> bool
		{	
			return A == B;
		});
}

/**
 * Filters elements from the TargetArray based on the presence in the FilterArray.
 * Uses a custom equality function to determine the presence of elements.
 * Optionally, inverts the filter condition.
 * 
 * @param TargetArray The array to be filtered.
 * @param FilterArray The array containing elements to filter out or keep.
 * @param Equals The custom equality function to use for filtering.
 * @param Invert If true, the filter condition is inverted. Defaults to false.
 */
template <typename T>
static void Filter(
	TArray<T>& TargetArray,
	const TArray<T>& FilterArray,
	TFunctionRef<bool(const T&, const T&)> Equals,
	bool Invert = false)
{
	// Define a predicate that uses the custom equality function
	auto Predicate = [&FilterArray, &Equals,Invert](const T& Element) -> bool
	{
		for (const T& FilterElement : FilterArray)
		{
			const bool IsMatch = Equals(Element, FilterElement);

			if (IsMatch && !Invert)
				return false;

			if (IsMatch && Invert)
				return true;
		}
		return !Invert;
	};

	// Use WhereNoRef to filter the TargetArray
	TargetArray = WhereNoRef<T>(TargetArray, Predicate);
}

template <typename KeyType, typename ValueType>
static TArray<ValueType> GetValuesAsArray(const TMap<KeyType, TArray<ValueType>>& Map)
{
	TArray<ValueType> ValuesArray;
	for (const auto& Pair : Map)
	{
		ValuesArray.Append(Pair.Value); // Append the array of values to ValuesArray
	}
	return ValuesArray;
}


/**
 * @brief Checks if any element in the array satisfies the given predicate.
 * 
 * This function iterates through each element in the provided array and applies
 * the given predicate function. If the predicate returns true for any element,
 * the function returns true. If the predicate returns false for all elements,
 * the function returns false.
 * 
 * @tparam T The type of elements in the array.
 * @param Array The array of elements to check.
 * @param Predicate The predicate function to apply to each element.
 * @return true If any element in the array satisfies the predicate.
 * @return false If no elements in the array satisfy the predicate.
 * 
 * @example
 * TArray<int32> Numbers = {1, 2, 3, 4, 5};
 * bool HasEven = Any<int32>(Numbers, [](const int32& Number) { return Number % 2 == 0; });
 * // HasEven will be true because 2 and 4 are even numbers.
 */
template <typename T>
static bool Any(const TArray<T>& Array, TFunctionRef<bool(const T&)> Predicate)
{
	for (const T& Element : Array)
	{
		if (Predicate(Element))
		{
			return true;
		}
	}
	return false;
}

template <typename T>
TArray<T*> ConvertToPointerArray(TArray<T>& InArray)
{
	TArray<T*> OutArray;
	OutArray.Reserve(InArray.Num());

	for (T& Item : InArray)
	{
		OutArray.Add(&Item);
	}

	return OutArray;
}

template <typename T>
bool CheckEmptyArray(const TArray<T*>& InArray)
{
	static_assert(std::is_base_of_v<UObject, T>, "T must be a subclass of UObject");

	for (T* Item : InArray)
	{
		if (Item != nullptr)
			return false;
	}

	return true;
}

template <typename T, typename FromT>
TArray<T*> CastArray(const TArray<FromT*>& InArray)
{
	static_assert(std::is_base_of_v<UObject, T>, "T must be a subclass of UObject");

	TArray<T*> OutArray;
	OutArray.Reserve(InArray.Num());

	for (FromT* Item : InArray)
	{
		if (T* CastedItem = Cast<T>(Item))
		{
			OutArray.Add(CastedItem);
		}
	}

	return OutArray;
}

template <typename ElementType>
static ElementType* PickRandomFromArray(const TArray<ElementType*>& Objects, const FRandomStream& RandomStream)
{
	if (Objects.Num() == 0)
	{
		return nullptr;
	}
	int32 RandomIndex = RandomStream.RandRange(0, Objects.Num() - 1);
	
	return Objects[RandomIndex];
}


template <typename ElementType>
static ElementType PickRandomFromArray(const TArray<ElementType>& Objects, const FRandomStream& RandomStream)
{
	if (Objects.Num() == 0)
	{
		return ElementType();
	}
	int32 RandomIndex = RandomStream.RandRange(0, Objects.Num() - 1);
	
	return Objects[RandomIndex];
}

template <typename ElementType>
static TArray<ElementType> RemoveGroup(const TArray<ElementType>& Source, const TArray<ElementType>& Removes)
{
	TArray<ElementType> ReturnArray = Source;
	for (ElementType Element : Removes)
	{
		if (ReturnArray.Contains(Element))
			ReturnArray.Remove(Element);
	}
	
	return ReturnArray;
}
