#pragma once

UENUM(Blueprintable)
enum class ERegionTypes : uint8
{
	Section,
	Subsection,
	Room,
	Master UMETA(Hidden),
};
