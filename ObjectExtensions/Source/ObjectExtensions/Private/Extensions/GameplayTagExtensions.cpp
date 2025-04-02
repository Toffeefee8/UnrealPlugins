#include "Extensions/GameplayTagExtensions.h"

FGameplayTag UGameplayTagExtensions::CreateTagFromParts(const TArray<FString>& Parts, int32 NumDetail)
{
	NumDetail = NumDetail < 0 ? Parts.Num() : FMath::Clamp(NumDetail, 0, Parts.Num());

	FString TagString;
	for (int32 i = 0; i < NumDetail; ++i)
	{
		if (i > 0)
			TagString.Append(TEXT(".")); 
		TagString.Append(Parts[i]);
	}
	
	return FGameplayTag::RequestGameplayTag(FName(*TagString));
}

TArray<FString> UGameplayTagExtensions::CreatePartsFromTag(FGameplayTag Tag)
{
	TArray<FString> Parts;
	Tag.ToString().ParseIntoArray(Parts, TEXT("."));
	return Parts;
}

FGameplayTag UGameplayTagExtensions::GetMostDetailedTag(FGameplayTagContainer Tags)
{
	FGameplayTag MostDetailedTag = FGameplayTag();
	int32 MaxDetailLevel = 0;

	for (const FGameplayTag& Tag : Tags)
	{
		TArray<FString> Components = CreatePartsFromTag(Tag);
		
		int32 DetailLevel = Components.Num();
		
		if (DetailLevel > MaxDetailLevel)
		{
			MaxDetailLevel = DetailLevel;
			MostDetailedTag = Tag;
		}
	}

	return MostDetailedTag;
}

int32 UGameplayTagExtensions::GetTagDepth(FGameplayTag Tag)
{
	return CreatePartsFromTag(Tag).Num();
}

FGameplayTag UGameplayTagExtensions::RemoveTagDepth(FGameplayTag Tag, uint8 DepthToRemove)
{
	FString TagString = Tag.ToString();
	
	for (uint8 i = 0; i < DepthToRemove; ++i)
	{
		int32 LastDotIndex;
		if (TagString.FindLastChar(TEXT('.'), LastDotIndex))
			TagString = TagString.Left(LastDotIndex);
		else
			return FGameplayTag();
	}
	
	return FGameplayTag::RequestGameplayTag(FName(*TagString));
}

FGameplayTag UGameplayTagExtensions::GetParentGameplayTag(FGameplayTag Tag)
{
	return RemoveTagDepth(Tag);
}

FName UGameplayTagExtensions::RemoveParentTagFromName(FGameplayTag Tag, FGameplayTag TagToRemove)
{
	if (Tag.MatchesTagExact(TagToRemove))
		return {};
	
	FString TagString = Tag.ToString();
	FString RemoveString = TagToRemove.ToString();

	if (TagString.StartsWith(RemoveString + TEXT(".")))
	{
		return FName(*TagString.RightChop(RemoveString.Len() + 1));
	}

	return FName();
}

FGameplayTagContainer UGameplayTagExtensions::InverseFilter(FGameplayTagContainer Container, FGameplayTagContainer SecondContainer)
{
	FGameplayTagContainer Intersection = Container.Filter(SecondContainer);
	FGameplayTagContainer Union = Container;
	Union.AppendTags(SecondContainer);
	Union.RemoveTags(Intersection);
	return Union;
}

FGameplayTagContainer UGameplayTagExtensions::GetExclusiveTags(FGameplayTagContainer Source, FGameplayTagContainer ToRemove)
{
	FGameplayTagContainer Intersection = Source.Filter(ToRemove);
	Source.RemoveTags(Intersection);
	return Source;
}
