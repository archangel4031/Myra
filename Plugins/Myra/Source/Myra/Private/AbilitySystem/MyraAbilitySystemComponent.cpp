// Copyright Myra . All Rights Reserved.

#include "AbilitySystem/MyraAbilitySystemComponent.h"
#include "DataAssets/MyraAbilitySet.h"
#include "AbilitySystem/MyraGameplayAbility.h"

UMyraAbilitySystemComponent::UMyraAbilitySystemComponent()
{
	// Mixed replication is correct for most multiplayer games.
	// Full mode replicates all GE info to every client (only use in single player or small games).
	// Minimal mode only replicates to owner (use if owner doesn't need GE details locally).
	ReplicationMode = EGameplayEffectReplicationMode::Mixed;
}

// ------------------------------------------------
//  Ability Set Granting
// ------------------------------------------------

void UMyraAbilitySystemComponent::GrantAbilitySet(UMyraAbilitySet* AbilitySet, UObject* SourceObject)
{
	if (!AbilitySet)
	{
		UE_LOG(LogTemp, Warning, TEXT("Myra: GrantAbilitySet called with null AbilitySet."));
		return;
	}

	// Don't grant the same set twice
	if (GrantedAbilitySets.Contains(AbilitySet))
	{
		return;
	}

	GrantedAbilitySets.Add(AbilitySet);
	AbilitySet->GiveToAbilitySystem(this, SourceObject,
		GrantedAbilityHandles.FindOrAdd(AbilitySet),
		GrantedEffectHandles.FindOrAdd(AbilitySet));
}

void UMyraAbilitySystemComponent::RemoveAbilitySet(UMyraAbilitySet* AbilitySet)
{
	if (!AbilitySet || !GrantedAbilitySets.Contains(AbilitySet))
	{
		return;
	}

	// Clear ability specs
	if (TArray<FGameplayAbilitySpecHandle>* Handles = GrantedAbilityHandles.Find(AbilitySet))
	{
		for (const FGameplayAbilitySpecHandle& Handle : *Handles)
		{
			ClearAbility(Handle);
		}
		GrantedAbilityHandles.Remove(AbilitySet);
	}

	// Remove active effects
	if (TArray<FActiveGameplayEffectHandle>* EffectHandles = GrantedEffectHandles.Find(AbilitySet))
	{
		for (const FActiveGameplayEffectHandle& Handle : *EffectHandles)
		{
			RemoveActiveGameplayEffect(Handle);
		}
		GrantedEffectHandles.Remove(AbilitySet);
	}

	GrantedAbilitySets.Remove(AbilitySet);
}

// ------------------------------------------------
//  Blueprint Helpers
// ------------------------------------------------

float UMyraAbilitySystemComponent::GetAttributeValue(FGameplayAttribute Attribute) const
{
	bool bFound = false;
	const float Value = GetGameplayAttributeValue(Attribute, bFound);
	return bFound ? Value : 0.f;
}

bool UMyraAbilitySystemComponent::HasAttribute(FGameplayAttribute Attribute) const
{
	return HasAttributeSetForAttribute(Attribute);
}

FActiveGameplayEffectHandle UMyraAbilitySystemComponent::ApplyEffectToSelf(
	TSubclassOf<UGameplayEffect> EffectClass, float Level)
{
	if (!EffectClass)
	{
		return FActiveGameplayEffectHandle();
	}

	FGameplayEffectContextHandle ContextHandle = MakeEffectContext();
	ContextHandle.AddSourceObject(this);

	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingSpec(EffectClass, Level, ContextHandle);
	if (SpecHandle.IsValid())
	{
		return ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}

	return FActiveGameplayEffectHandle();
}

// ------------------------------------------------
//  Attribute Change Broadcasting
//  NotifyAttributeChanged is called by UGASAttributeSet::PostAttributeChange,
//  which IS a valid UAttributeSet virtual. The AttributeSet calls this method
//  to push the event up to the ASC's Blueprint-visible delegate.
// ------------------------------------------------

void UMyraAbilitySystemComponent::NotifyAttributeChanged(
	const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	OnAttributeChanged.Broadcast(Attribute, OldValue, NewValue);
}
