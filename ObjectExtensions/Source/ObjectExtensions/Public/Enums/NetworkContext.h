#pragma once

UENUM(Blueprintable, BlueprintType)
enum class ENetworkContext : uint8
{
	Server,
	Client,
	Both,
};