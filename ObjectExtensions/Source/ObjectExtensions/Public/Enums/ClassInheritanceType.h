// Copyright Phoenix Dawn Development LLC. All Rights Reserved.

#pragma once

UENUM(Blueprintable)
enum class EClassInheritanceType : uint8
{
	Same,
	Parent,
	Child,
	Different,
};