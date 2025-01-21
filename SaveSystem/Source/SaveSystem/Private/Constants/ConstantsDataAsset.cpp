// Fill out your copyright notice in the Description page of Project Settings.


#include "Constants/ConstantsDataAsset.h"

#include "Constants/Configs/ConstantConfigs.h"

TArray<UConstantConfigs*> UConstantsDataAsset::GetAllConfigsOfType(const TSubclassOf<UConstantConfigs>& Class) const
{
	TArray<UConstantConfigs*> ReturnConfigs {};
	for (auto Config : Configs)
	{
		if (!Config)
			continue;

		if (Config->GetClass()->IsChildOf(Class))
			ReturnConfigs.Add(Config);
	}
	return ReturnConfigs;
}
