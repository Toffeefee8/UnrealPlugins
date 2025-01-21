// Copyright Phoenix Dawn Development LLC. All Rights Reserved.

#pragma once

UENUM(BlueprintType, meta = (Bitflags))
enum class ELightingComponentType : uint8
{
	PostProcess UMETA(BitflagsEnum = "ELightingComponentType"),
	DirectionalLight UMETA(BitflagsEnum = "ELightingComponentType"),
	ExponentialHeightFog UMETA(BitflagsEnum = "ELightingComponentType"),
	SkyLight UMETA(BitflagsEnum = "ELightingComponentType"),
	SkyAtmosphere UMETA(BitflagsEnum = "ELightingComponentType"),
	VolumetricClouds UMETA(BitflagsEnum = "ELightingComponentType"),
};