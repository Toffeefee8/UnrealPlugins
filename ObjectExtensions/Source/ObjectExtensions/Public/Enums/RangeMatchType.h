#pragma once

UENUM(Blueprintable)
enum class ERangeMatchType : uint8
{
	Inside,
	Outside,
	InsideEqual,
	OutsideEqual,
};