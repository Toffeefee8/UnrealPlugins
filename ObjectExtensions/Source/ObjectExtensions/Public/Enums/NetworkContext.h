// Copyright Phoenix Dawn Development LLC. All Rights Reserved.

#pragma once

UENUM(Blueprintable, BlueprintType)
enum class ENetworkContext : uint8
{
	Server,
	Client,
	Both,
};