#pragma once

#include "CoreMinimal.h"
#include "ObjectWrapperArray.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ObjectWrapperFunctionLibrary.generated.h"

struct FObjectWrapper;
/**
 * 
 */
UCLASS()
class OBJECTEXTENSIONS_API UObjectWrapperFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

#pragma region ObjectWrappers
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ObjectWrapper")
	static bool IsValid(const FObjectWrapper& Wrapper);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ObjectWrapper")
	static bool IsClass(const FObjectWrapper& Wrapper, const TSubclassOf<UObject> Class, bool AllowChildClasses = true);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ObjectWrapper")
	static FString GetSelectedName(const FObjectWrapper& Wrapper);

	UFUNCTION(BlueprintCallable, Category = "ObjectWrapper")
	static UObject* GetObject(const FObjectWrapper& Wrapper, UObject* Outer = nullptr);
	UFUNCTION(BlueprintCallable, Category = "ObjectWrapper", meta = (DeterminesOutputType = "WrapperClass"))
	static UObject* GetObjectAs(const FObjectWrapper& Wrapper, TSubclassOf<UObject> WrapperClass, UObject* Outer = nullptr);

	UFUNCTION(BlueprintCallable, Category = "ObjectWrapper")
	static FObjectWrapper CreateObjectWrapper(UObject* Object);
#pragma endregion

	
#pragma region ObjectWrapperArray
	UFUNCTION(BlueprintCallable, Category = "ObjectWrapperArray")
	static bool AddNewObjectWrapperOfClass(UPARAM(ref) FObjectWrapperArray& WrapperArray, const TSubclassOf<UObject> ObjectClass, bool IncludeRequired = false);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ObjectWrapperArray")
	static TArray<TSubclassOf<UObject>> GetCurrentCachedClasses(const FObjectWrapperArray& WrapperArray);
	
	UFUNCTION(BlueprintCallable, Category = "ObjectWrapperArray")
	static FObjectWrapper GetObjectWrapper(const FObjectWrapperArray& WrapperArray, TSubclassOf<UObject> ObjectClass);
	UFUNCTION(BlueprintCallable, Category = "ObjectWrapperArray", meta = (DeterminesOutputType = "ObjectClass"))
	static UObject* GetObjectFromArray(const FObjectWrapperArray& WrapperArray, const TSubclassOf<UObject> ObjectClass, UObject* Outer = nullptr);

	UFUNCTION(BlueprintCallable, Category = "ObjectWrapperArray")
	static TArray<FObjectWrapper> GetObjectWrappers(const FObjectWrapperArray& WrapperArray, const TSubclassOf<UObject> ObjectClass);
	UFUNCTION(BlueprintCallable, Category = "ObjectWrapperArray", meta = (DeterminesOutputType = "ObjectClass"))
	static TArray<UObject*> GetObjects(const FObjectWrapperArray& WrapperArray, const TSubclassOf<UObject> ObjectClass, UObject* Outer = nullptr);

	//All Gets
	UFUNCTION(BlueprintCallable, Category = "ObjectWrapperArray")
	static TArray<FObjectWrapper> GetCopyArray(const FObjectWrapperArray& WrapperArray);
	UFUNCTION(BlueprintCallable, Category = "ObjectWrapperArray")
	static TArray<UObject*> GetAllObjects(const FObjectWrapperArray& WrapperArray, UObject* Outer = nullptr);
#pragma endregion


};