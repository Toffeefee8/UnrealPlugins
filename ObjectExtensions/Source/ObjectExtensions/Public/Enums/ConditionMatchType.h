// Copyright Phoenix Dawn Development LLC. All Rights Reserved.

#pragma once

UENUM(Blueprintable)
enum class EConditionMatchType : uint8
{
	Any,
	All,
	NAny,
	None,
};