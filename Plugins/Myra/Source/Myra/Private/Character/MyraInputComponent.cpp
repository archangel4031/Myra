// Copyright Myra . All Rights Reserved.

#include "Character/MyraInputComponent.h"
#include "AbilitySystem/MyraAbilitySystemComponent.h"

void UMyraInputComponent::ActivateAbilityByTag(UMyraAbilitySystemComponent* ASC, FGameplayTag InputTag)
{
	if (!ASC || !InputTag.IsValid()) { return; }
	ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(InputTag));
}

void UMyraInputComponent::ReleaseAbilityByTag(UMyraAbilitySystemComponent* ASC, FGameplayTag InputTag)
{
	if (!ASC || !InputTag.IsValid()) { return; }
	for (FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		// Updated to use the new GetDynamicSpecSourceTags() API
		if (Spec.GetDynamicSpecSourceTags().HasTagExact(InputTag) && Spec.IsActive())
		{
			ASC->AbilitySpecInputReleased(Spec);
		}
	}
}
