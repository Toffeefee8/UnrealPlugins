#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameplayTagTreeNotifier.h" // Your notifier interface header.
#include "Containers/Map.h"
#include "Containers/Array.h"
#include "Extensions/GameplayTagExtensions.h" // Must supply GetParentGameplayTag()
#include "Templates/SharedPointer.h"
#include "Templates/UnrealTypeTraits.h" // For TIsDerivedFrom, TRemovePointer
#include "UObject/Class.h"             // For Cast<>

// Helper: If T is not a pointer, return its address; if it is a pointer, return it as-is.
template<typename U>
struct TGetNotifierPointer
{
	static U* Get(U& Value)
	{
		return &Value;
	}
};

template<typename U>
struct TGetNotifierPointer<U*>
{
	static U* Get(U* Value)
	{
		return Value;
	}
};

template <typename T>
struct FGameplayTagTreeNode
{
	/** The gameplay tag for this node. */
	FGameplayTag Tag;

	/** The value stored at this node. */
	T Value;

	/** Weak pointer to the parent node (nullptr if root). */
	TWeakPtr<FGameplayTagTreeNode<T>> Parent;

	/** Child nodes. */
	TArray<TSharedPtr<FGameplayTagTreeNode<T>>> Children;

	/** Constructor. */
	FGameplayTagTreeNode(const FGameplayTag& InTag, const T& InValue)
		: Tag(InTag)
		, Value(InValue)
	{}
};

/**
 * A hierarchical wrapper that works like a TMap from gameplay tags to T.
 * The tree organizes nodes by their tag’s hierarchical name.
 *
 * In addition, the tree provides:
 *  - A Contains() function.
 *  - A Remove() function that re-parents children and notifies nodes.
 *  - A Find() that returns a pointer to the stored value.
 *  - A custom iterator so that you can use a range-based for loop. Each iteration yields
 *    a tuple (FGameplayTag, T).
 *  - An Empty() function to remove all entries.
 */
template <typename T>
class TGameplayTagTree
{
public:
	//-------------------------------------------------
	// Insertion (with notifications & reparenting)
	//-------------------------------------------------
	bool Insert(const FGameplayTag& InTag, const T& InValue)
	{
		// Do not allow an invalid or duplicate tag.
		if (!InTag.IsValid() || NodeMap.Contains(InTag))
		{
			return false;
		}

		// Create the new node.
		TSharedPtr<FGameplayTagTreeNode<T>> NewNode = MakeShared<FGameplayTagTreeNode<T>>(InTag, InValue);

		// Helper lambdas for notifications.
		auto NotifyParentChanged = [](TSharedPtr<FGameplayTagTreeNode<T>> Node,
                                      const FGameplayTag& OldParentTag, const T& OldParentValue,
                                      const FGameplayTag& NewParentTag, const T& NewParentValue)
        {
            if constexpr (TIsDerivedFrom<typename TRemovePointer<T>::Type, UObject>::Value)
            {
                if (IGameplayTagTreeNotifier* Notifier = Cast<IGameplayTagTreeNotifier>(TGetNotifierPointer<T>::Get(Node->Value)))
                {
                    Notifier->OnParentChanged(OldParentTag, reinterpret_cast<UObject*>(OldParentValue), NewParentTag, reinterpret_cast<UObject*>(NewParentValue));
                }
            }
            else
            {
                if (IGameplayTagTreeNotifier* Notifier = dynamic_cast<IGameplayTagTreeNotifier*>(TGetNotifierPointer<T>::Get(Node->Value)))
                {
                    Notifier->OnParentChanged(OldParentTag, reinterpret_cast<UObject*>(OldParentValue), NewParentTag, reinterpret_cast<UObject*>(NewParentValue));
                }
            }
        };

        auto NotifyChildChanged = [](TSharedPtr<FGameplayTagTreeNode<T>> Node,
                                     const FGameplayTag& OldChildTag, const T& OldChildValue,
                                     const FGameplayTag& NewChildTag, const T& NewChildValue)
        {
            if constexpr (TIsDerivedFrom<typename TRemovePointer<T>::Type, UObject>::Value)
            {
                if (IGameplayTagTreeNotifier* Notifier = Cast<IGameplayTagTreeNotifier>(TGetNotifierPointer<T>::Get(Node->Value)))
                {
                    Notifier->OnChildChanged(OldChildTag, reinterpret_cast<UObject*>(OldChildValue), NewChildTag, reinterpret_cast<UObject*>(NewChildValue));
                }
            }
            else
            {
                if (IGameplayTagTreeNotifier* Notifier = dynamic_cast<IGameplayTagTreeNotifier*>(TGetNotifierPointer<T>::Get(Node->Value)))
                {
                    Notifier->OnChildChanged(OldChildTag, reinterpret_cast<UObject*>(OldChildValue), NewChildTag, reinterpret_cast<UObject*>(NewChildValue));
                }
            }
        };

		// Find the closest existing ancestor.
		TSharedPtr<FGameplayTagTreeNode<T>> ParentNode = FindClosestAncestor(InTag);
		if (ParentNode.IsValid())
		{
			// Attach NewNode as a child.
			NewNode->Parent = ParentNode;
			ParentNode->Children.Add(NewNode);

			// Notify NewNode that its parent has been set.
			NotifyParentChanged(NewNode, FGameplayTag(), T(), ParentNode->Tag, ParentNode->Value);
			// Notify ParentNode that it has gained a new child.
			NotifyChildChanged(ParentNode, FGameplayTag(), T(), NewNode->Tag, NewNode->Value);

			// Re-parent any children of ParentNode that now belong under NewNode.
			for (int32 i = ParentNode->Children.Num() - 1; i >= 0; --i)
			{
				TSharedPtr<FGameplayTagTreeNode<T>> Child = ParentNode->Children[i];
				if (Child == NewNode)
				{
					continue;
				}
				// If the child's tag is a descendant of NewNode's tag…
				if (Child->Tag.MatchesTag(NewNode->Tag))
				{
					// Notify the child that its parent is changing.
					NotifyParentChanged(Child, ParentNode->Tag, ParentNode->Value, NewNode->Tag, NewNode->Value);
					// Notify ParentNode that it is losing this child.
					NotifyChildChanged(ParentNode, Child->Tag, Child->Value, FGameplayTag(), T());

					ParentNode->Children.RemoveAt(i);
					NewNode->Children.Add(Child);
					Child->Parent = NewNode;

					// Notify NewNode that it has gained this child.
					NotifyChildChanged(NewNode, FGameplayTag(), T(), Child->Tag, Child->Value);
				}
			}
		}
		else
		{
			// No ancestor exists; this is a root.
			// Check if any current roots are descendants of NewNode.
			for (int32 i = RootNodes.Num() - 1; i >= 0; --i)
			{
				TSharedPtr<FGameplayTagTreeNode<T>> RootNode = RootNodes[i];
				if (RootNode->Tag.MatchesTag(NewNode->Tag))
				{
					NotifyParentChanged(RootNode, FGameplayTag(), T(), NewNode->Tag, NewNode->Value);
					NotifyChildChanged(NewNode, FGameplayTag(), T(), RootNode->Tag, RootNode->Value);

					RootNodes.RemoveAt(i);
					NewNode->Children.Add(RootNode);
					RootNode->Parent = NewNode;
				}
			}
			RootNodes.Add(NewNode);
			NotifyParentChanged(NewNode, FGameplayTag(), T(), FGameplayTag(), T());
		}

		NodeMap.Add(InTag, NewNode);
		return true;
	}

	//-------------------------------------------------
	// Contains: Check if a tag is in the tree.
	//-------------------------------------------------
	bool Contains(const FGameplayTag& InTag) const
	{
		return NodeMap.Contains(InTag);
	}

	//-------------------------------------------------
	// Find: Return a pointer to the stored value for a tag.
	//-------------------------------------------------
	T* Find(const FGameplayTag& InTag) const
	{
		if (const TSharedPtr<FGameplayTagTreeNode<T>>* FoundNode = NodeMap.Find(InTag))
		{
			return &((*FoundNode)->Value);
		}
		return nullptr;
	}

	//-------------------------------------------------
	// Remove: Remove a node (with notifications and reparenting).
	//-------------------------------------------------
	bool Remove(const FGameplayTag& InTag)
	{
		TSharedPtr<FGameplayTagTreeNode<T>> NodeToRemove = FindNode(InTag);
		if (!NodeToRemove.IsValid())
		{
			return false;
		}

		auto NotifyParentChanged = [](TSharedPtr<FGameplayTagTreeNode<T>> Node,
                                      const FGameplayTag& OldParentTag, const T& OldParentValue,
                                      const FGameplayTag& NewParentTag, const T& NewParentValue)
        {
            if constexpr (TIsDerivedFrom<typename TRemovePointer<T>::Type, UObject>::Value)
            {
                if (IGameplayTagTreeNotifier* Notifier = Cast<IGameplayTagTreeNotifier>(TGetNotifierPointer<T>::Get(Node->Value)))
                {
                    Notifier->OnParentChanged(OldParentTag, reinterpret_cast<UObject*>(OldParentValue), NewParentTag, reinterpret_cast<UObject*>(NewParentValue));
                }
            }
            else
            {
                if (IGameplayTagTreeNotifier* Notifier = dynamic_cast<IGameplayTagTreeNotifier*>(TGetNotifierPointer<T>::Get(Node->Value)))
                {
                    Notifier->OnParentChanged(OldParentTag, reinterpret_cast<UObject*>(OldParentValue), NewParentTag, reinterpret_cast<UObject*>(NewParentValue));
                }
            }
        };

        auto NotifyChildChanged = [](TSharedPtr<FGameplayTagTreeNode<T>> Node,
                                     const FGameplayTag& OldChildTag, const T& OldChildValue,
                                     const FGameplayTag& NewChildTag, const T& NewChildValue)
        {
            if constexpr (TIsDerivedFrom<typename TRemovePointer<T>::Type, UObject>::Value)
            {
                if (IGameplayTagTreeNotifier* Notifier = Cast<IGameplayTagTreeNotifier>(TGetNotifierPointer<T>::Get(Node->Value)))
                {
                    Notifier->OnChildChanged(OldChildTag, reinterpret_cast<UObject*>(OldChildValue), NewChildTag, reinterpret_cast<UObject*>(NewChildValue));
                }
            }
            else
            {
                if (IGameplayTagTreeNotifier* Notifier = dynamic_cast<IGameplayTagTreeNotifier*>(TGetNotifierPointer<T>::Get(Node->Value)))
                {
                    Notifier->OnChildChanged(OldChildTag, reinterpret_cast<UObject*>(OldChildValue), NewChildTag, reinterpret_cast<UObject*>(NewChildValue));
                }
            }
        };

		TSharedPtr<FGameplayTagTreeNode<T>> ParentNode = NodeToRemove->Parent.Pin();
		if (ParentNode.IsValid())
		{
			// Notify the parent that it is losing this child.
			NotifyChildChanged(ParentNode, NodeToRemove->Tag, NodeToRemove->Value, FGameplayTag(), T());
			ParentNode->Children.Remove(NodeToRemove);
		}
		else
		{
			RootNodes.Remove(NodeToRemove);
		}

		// Reparent NodeToRemove's children to its parent (or make them roots if no parent exists).
		for (TSharedPtr<FGameplayTagTreeNode<T>> Child : NodeToRemove->Children)
		{
			if (ParentNode.IsValid())
			{
				NotifyParentChanged(Child, NodeToRemove->Tag, NodeToRemove->Value, ParentNode->Tag, ParentNode->Value);
				ParentNode->Children.Add(Child);
			}
			else
			{
				NotifyParentChanged(Child, NodeToRemove->Tag, NodeToRemove->Value, FGameplayTag(), T());
				RootNodes.Add(Child);
			}
			Child->Parent = ParentNode;
		}
		NodeToRemove->Children.Empty();
		NodeMap.Remove(InTag);
		return true;
	}

	//-------------------------------------------------
	// Empty: Remove all nodes from the tree.
	//-------------------------------------------------
	void Empty()
	{
		// Remove nodes until the tree is empty.
		while (NodeMap.Num() > 0)
		{
			if (RootNodes.Num() > 0)
			{
				FGameplayTag TagToRemove = RootNodes[0]->Tag;
				Remove(TagToRemove);
			}
			else
			{
				FGameplayTag TagToRemove = NodeMap.CreateConstIterator().Key();
				Remove(TagToRemove);
			}
		}
	}

	//-------------------------------------------------
	// Accessors for children, parent, and roots.
	//-------------------------------------------------
	TArray<TSharedPtr<FGameplayTagTreeNode<T>>> GetChildren(const FGameplayTag& InTag) const
	{
		if (TSharedPtr<FGameplayTagTreeNode<T>> Node = FindNode(InTag))
		{
			return Node->Children;
		}
		return TArray<TSharedPtr<FGameplayTagTreeNode<T>>>();
	}

	TSharedPtr<FGameplayTagTreeNode<T>> GetParent(const FGameplayTag& InTag) const
	{
		if (TSharedPtr<FGameplayTagTreeNode<T>> Node = FindNode(InTag))
		{
			return Node->Parent.Pin();
		}
		return nullptr;
	}

	const TArray<TSharedPtr<FGameplayTagTreeNode<T>>>& GetRootNodes() const
	{
		return RootNodes;
	}

	//-------------------------------------------------
	// GetEntries: Returns a TArray of tuples (FGameplayTag, T).
	//-------------------------------------------------
	TArray<TTuple<FGameplayTag, T>> GetEntries() const
	{
		TArray<TTuple<FGameplayTag, T>> Entries;
		for (const auto& Pair : NodeMap)
		{
			Entries.Add(MakeTuple(Pair.Key, Pair.Value->Value));
		}
		return Entries;
	}

	//-------------------------------------------------
	// Custom iterator for range-based for loops.
	// Each iteration yields a tuple (FGameplayTag, T).
	//-------------------------------------------------
	class FConstIterator
	{
	public:
		using MapIteratorType = typename TMap<FGameplayTag, TSharedPtr<FGameplayTagTreeNode<T>>>::TConstIterator;
		FConstIterator(MapIteratorType InIt) : It(InIt) {}
		FConstIterator& operator++() { ++It; return *this; }
		bool operator!=(const FConstIterator& Other) const { return It != Other.It; }
		TTuple<FGameplayTag, T> operator*() const
		{
			return MakeTuple(It.Key(), It.Value()->Value);
		}
	private:
		MapIteratorType It;
	};

	FConstIterator begin() const
	{
		return FConstIterator(NodeMap.CreateConstIterator());
	}

	FConstIterator end() const
	{
		auto It = NodeMap.CreateConstIterator();
		while (It) { ++It; }
		return FConstIterator(It);
	}

private:
	// Helper: Return the node pointer for a tag.
	TSharedPtr<FGameplayTagTreeNode<T>> FindNode(const FGameplayTag& InTag) const
	{
		if (const TSharedPtr<FGameplayTagTreeNode<T>>* FoundNode = NodeMap.Find(InTag))
		{
			return *FoundNode;
		}
		return nullptr;
	}

	// Helper: Finds the closest ancestor node for the given tag.
	TSharedPtr<FGameplayTagTreeNode<T>> FindClosestAncestor(const FGameplayTag& InTag)
	{
		FGameplayTag ParentTag = UGameplayTagExtensions::GetParentGameplayTag(InTag);
		while (ParentTag.IsValid())
		{
			TSharedPtr<FGameplayTagTreeNode<T>> AncestorNode = FindNode(ParentTag);
			if (AncestorNode.IsValid())
			{
				return AncestorNode;
			}
			ParentTag = UGameplayTagExtensions::GetParentGameplayTag(ParentTag);
		}
		return nullptr;
	}

	// Internal storage.
	TMap<FGameplayTag, TSharedPtr<FGameplayTagTreeNode<T>>> NodeMap;
	TArray<TSharedPtr<FGameplayTagTreeNode<T>>> RootNodes;
};
