#include "DebugConfig.h"

bool FDebugConfig::AllowsDebugType(EDebugDisplayType DisplayType) const
{
	switch (DisplayType) {
		case EDebugDisplayType::Log:
			return bLog;
		case EDebugDisplayType::Print:
			return bPrint;
		case EDebugDisplayType::Visual:
			return bVisual;
		case EDebugDisplayType::Sound:
			return bSound;
	}
	return false;
}
