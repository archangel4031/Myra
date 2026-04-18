// Copyright Myra . All Rights Reserved.

#include "DataAssets/MyraInputConfig.h"
#include "InputAction.h"

const UInputAction* UMyraInputConfig::FindNativeInputActionForTag(
	const FGameplayTag& InputTag,
	bool bLogIfNotFound) const
{
	return FindInputActionForTag(NativeInputActions, InputTag, bLogIfNotFound);
}

const UInputAction* UMyraInputConfig::FindAbilityInputActionForTag(
	const FGameplayTag& InputTag,
	bool bLogIfNotFound) const
{
	return FindInputActionForTag(AbilityInputActions, InputTag, bLogIfNotFound);
}

const UInputAction* UMyraInputConfig::FindInputActionForTag(
	const TArray<FMyraInputAction>& InputActions,
	const FGameplayTag& InputTag,
	bool bLogIfNotFound) const
{
	if (!InputTag.IsValid())
	{
		return nullptr;
	}

	for (const FMyraInputAction& Entry : InputActions)
	{
		if (Entry.InputTag == InputTag)
		{
			return Entry.InputAction;
		}
	}

	if (bLogIfNotFound)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("MyraInputConfig '%s' does not contain an input action for tag '%s'."),
			*GetNameSafe(this),
			*InputTag.ToString());
	}

	return nullptr;
}
