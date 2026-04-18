// Copyright Myra . All Rights Reserved.

#include "Character/MyraInputComponent.h"
#include "AbilitySystem/MyraAbilitySystemComponent.h"
#include "DataAssets/MyraInputConfig.h"

void UMyraInputComponent::ActivateAbilityByTag(UMyraAbilitySystemComponent* ASC, FGameplayTag InputTag)
{
	if (!ASC || !InputTag.IsValid()) { return; }
	ASC->AbilityInputTagPressed(InputTag);
}

void UMyraInputComponent::ReleaseAbilityByTag(UMyraAbilitySystemComponent* ASC, FGameplayTag InputTag)
{
	if (!ASC || !InputTag.IsValid()) { return; }
	ASC->AbilityInputTagReleased(InputTag);
}

void UMyraInputComponent::BindAbilityActions(
	UEnhancedInputComponent* InputComponent,
	const UMyraInputConfig* InputConfig,
	UMyraAbilitySystemComponent* ASC,
	TArray<uint32>& OutBindHandles)
{
	if (!InputComponent || !InputConfig || !ASC)
	{
		return;
	}

	for (const FMyraInputAction& Entry : InputConfig->AbilityInputActions)
	{
		if (!Entry.InputAction || !Entry.InputTag.IsValid())
		{
			continue;
		}

		FEnhancedInputActionEventBinding& PressedBinding = InputComponent->BindAction(
			Entry.InputAction,
			ETriggerEvent::Started,
			ASC,
			&UMyraAbilitySystemComponent::AbilityInputTagPressed,
			Entry.InputTag);
		OutBindHandles.Add(PressedBinding.GetHandle());

		FEnhancedInputActionEventBinding& ReleasedBinding = InputComponent->BindAction(
			Entry.InputAction,
			ETriggerEvent::Completed,
			ASC,
			&UMyraAbilitySystemComponent::AbilityInputTagReleased,
			Entry.InputTag);
		OutBindHandles.Add(ReleasedBinding.GetHandle());

		FEnhancedInputActionEventBinding& CanceledBinding = InputComponent->BindAction(
			Entry.InputAction,
			ETriggerEvent::Canceled,
			ASC,
			&UMyraAbilitySystemComponent::AbilityInputTagReleased,
			Entry.InputTag);
		OutBindHandles.Add(CanceledBinding.GetHandle());
	}
}
