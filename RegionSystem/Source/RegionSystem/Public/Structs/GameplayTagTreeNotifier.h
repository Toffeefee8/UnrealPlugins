#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameplayTagTreeNotifier.generated.h"

/**
 * Interface for types that want to be notified when their parent or child
 * relationship in the gameplay tag tree changes.
 *
 * When a relationship change occurs, the notifier is passed the old tag/value
 * and the new tag/value. Use an invalid tag (and a default-constructed value)
 * to represent “none.”
 */

UINTERFACE(NotBlueprintable, meta = (CannotImplementInterfaceInBlueprint))
class REGIONSYSTEM_API UGameplayTagTreeNotifier : public UInterface
{
	GENERATED_BODY()
};

class REGIONSYSTEM_API IGameplayTagTreeNotifier
{
	GENERATED_BODY()

public:

	/**
	 * Called when this node’s parent changes.
	 *
	 * @param OldParentTag  The old parent’s tag (or an invalid tag if none).
	 * @param OldParentValue The old parent’s value (nullptr if none).
	 * @param NewParentTag  The new parent’s tag (or an invalid tag if none).
	 * @param NewParentValue The new parent’s value (nullptr if none).
	 */
	virtual void OnParentChanged(const FGameplayTag& OldParentTag, UObject* OldParentValue,
								 const FGameplayTag& NewParentTag, UObject* NewParentValue) = 0;
	
	/**
	 * Called when a child relationship changes.
	 *
	 * @param OldChildTag   The old child’s tag (or an invalid tag if none).
	 * @param OldChildValue The old child’s value (nullptr if none).
	 * @param NewChildTag   The new child’s tag (or an invalid tag if none).
	 * @param NewChildValue The new child’s value (nullptr if none).
	 */
	virtual void OnChildChanged(const FGameplayTag& OldChildTag, UObject* OldChildValue,
								const FGameplayTag& NewChildTag, UObject* NewChildValue) = 0;
};
