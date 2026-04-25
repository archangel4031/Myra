// Copyright Myra . All Rights Reserved.

#include "AbilitySystem/MyraBaseAttributeSet.h"
#include "AbilitySystem/MyraAbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"

void UMyraBaseAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	BroadcastGameplayEffectExecuted(Data);
}

void UMyraBaseAttributeSet::PostAttributeChange(
	const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
	BroadcastAttributeChanged(Attribute, OldValue, NewValue);
}

void UMyraBaseAttributeSet::BroadcastAttributeChanged(
	const FGameplayAttribute& Attribute, float OldValue, float NewValue) const
{
	if (UMyraAbilitySystemComponent* MyraASC = Cast<UMyraAbilitySystemComponent>(
		GetOwningAbilitySystemComponent()))
	{
		MyraASC->NotifyAttributeChanged(Attribute, OldValue, NewValue);
	}
}

void UMyraBaseAttributeSet::BroadcastGameplayEffectExecuted(
	const FGameplayEffectModCallbackData& Data) const
{
	if (UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent())
	{
		if (ASC->GetOwnerRole() == ROLE_Authority)
		{
			if (UMyraAbilitySystemComponent* MyraASC = Cast<UMyraAbilitySystemComponent>(ASC))
			{
				FMyraGEExecutedInfo Info;
				Info.Attribute = Data.EvaluatedData.Attribute;
				Info.Magnitude = Data.EvaluatedData.Magnitude;
				Info.NewValue = Data.EvaluatedData.Attribute.GetNumericValue(this);

				const FGameplayEffectContextHandle& Context = Data.EffectSpec.GetContext();
				Info.Instigator = Context.GetInstigator();
				Info.EffectCauser = Context.GetEffectCauser();

				if (Data.EffectSpec.Def)
				{
					Info.EffectTags = Data.EffectSpec.Def->GetAssetTags();
				}

				MyraASC->NotifyGameplayEffectExecuted(Info);
			}
		}
	}
}
