#include "SaveSettings.h"

USaveSettings* USaveSettings::Get()
{
	return StaticClass()->GetDefaultObject<USaveSettings>();
}
