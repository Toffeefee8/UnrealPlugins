#pragma once
#include "DebugDisplayType.h"

#include "DebugConfig.generated.h"

USTRUCT(BlueprintType)
struct DEBUGSYSTEM_API FDebugConfig
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (InlineEditConditionToggle))
	bool bLog = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Log", meta = (EditCondition = bLog))
	float LogDuration = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (InlineEditConditionToggle))
	bool bPrint = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Print", meta = (EditCondition = bPrint))
	float PrintDuration = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (InlineEditConditionToggle))
	bool bVisual = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Visual", meta = (EditCondition = bVisual))
	float VisualDuration = 1.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (InlineEditConditionToggle))
	bool bSound = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Sound", meta = (EditCondition = bSound))
	float SoundVolume = 1.f;

	bool AllowsDebugType(EDebugDisplayType DisplayType) const;

};
