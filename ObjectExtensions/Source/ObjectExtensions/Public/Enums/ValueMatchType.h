// Copyright Phoenix Dawn Development LLC. All Rights Reserved.

#pragma once

UENUM(Blueprintable)
enum class EValueMatchType : uint8
{
	Equal,
	Greater,
	Less,
	GreaterEquals,
	LessEquals,
	NotEqual,
};