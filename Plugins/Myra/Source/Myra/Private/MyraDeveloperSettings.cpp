// Copyright Myra . All Rights Reserved.

#include "MyraDeveloperSettings.h"

UMyraDeveloperSettings::UMyraDeveloperSettings()
{
	// Default tag values — these should match tags in your project's GameplayTagsList.ini.
	// The wizard will remind users to add these tags if they don't exist.
	DeadTag            = FGameplayTag::RequestGameplayTag(FName("State.Dead"), false);
	AbilityActivatingTag = FGameplayTag::RequestGameplayTag(FName("State.AbilityActivating"), false);
}
