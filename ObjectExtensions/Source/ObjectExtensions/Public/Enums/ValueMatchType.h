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