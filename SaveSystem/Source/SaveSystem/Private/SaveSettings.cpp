// Copyright Phoenix Dawn Development LLC. All Rights Reserved.


#include "SaveSettings.h"

USaveSettings* USaveSettings::Get()
{
	return StaticClass()->GetDefaultObject<USaveSettings>();
}
