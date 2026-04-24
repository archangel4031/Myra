// Copyright Myra . All Rights Reserved.

#include "AbilitySystem/MyraAbilitySystemComponent.h"
#include "DataAssets/MyraAbilitySet.h"
#include "AbilitySystem/MyraGameplayAbility.h"
#include "AttributeSet.h"
#include "UObject/UObjectGlobals.h"

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
		GrantedEffectHandles.FindOrAdd(AbilitySet),
		GrantedAttributeSetHandles.FindOrAdd(AbilitySet));
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
			RemoveTrackedGameplayEffect(Handle);
		}
		GrantedEffectHandles.Remove(AbilitySet);
	}

	// Remove spawned attribute sets created by this ability set
	if (TArray<TWeakObjectPtr<UAttributeSet>>* AttributeSetHandles = GrantedAttributeSetHandles.Find(AbilitySet))
	{
		for (const TWeakObjectPtr<UAttributeSet>& AttributeSetHandle : *AttributeSetHandles)
		{
			if (UAttributeSet* AttributeSet = AttributeSetHandle.Get())
			{
				RemoveSpawnedAttribute(AttributeSet);
			}
		}
		GrantedAttributeSetHandles.Remove(AbilitySet);
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

bool UMyraAbilitySystemComponent::HasAttributeSetOfClass(TSubclassOf<UAttributeSet> AttributeSetClass) const
{
	UClass* AttributeSetClassPtr = AttributeSetClass.Get();
	if (!AttributeSetClassPtr)
	{
		return false;
	}

	for (const UAttributeSet* ExistingSet : GetSpawnedAttributes())
	{
		if (ExistingSet && ExistingSet->GetClass() == AttributeSetClassPtr)
		{
			return true;
		}
	}

	if (const AActor* MyraOwnerActor = GetOwner())
	{
		TArray<UObject*> OwnerSubobjects;
		GetObjectsWithOuter(const_cast<AActor*>(MyraOwnerActor), OwnerSubobjects, false);

		for (UObject* Object : OwnerSubobjects)
		{
			// Only treat constructor-created default subobjects as permanently owned sets.
			// Runtime-spawned sets that were removed from the ASC can still exist under the same
			// owner until GC, and must not block a later re-grant of the same AbilitySet.
			if (!Object || !Object->HasAnyFlags(RF_DefaultSubObject))
			{
				continue;
			}

			const UAttributeSet* ExistingSet = Cast<UAttributeSet>(Object);
			if (ExistingSet && ExistingSet->GetClass() == AttributeSetClassPtr)
			{
				return true;
			}
		}
	}

	return false;
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

FActiveGameplayEffectHandle UMyraAbilitySystemComponent::ApplyInitializationEffectOnce(
	TSubclassOf<UGameplayEffect> EffectClass,
	float Level,
	const UObject* SourceObject)
{
	if (!EffectClass)
	{
		return FActiveGameplayEffectHandle();
	}

	const UClass* EffectClassPtr = EffectClass.Get();
	for (const TPair<FActiveGameplayEffectHandle, const UClass*>& AppliedEffectPair : AppliedInitializationEffects)
	{
		if (AppliedEffectPair.Value == EffectClassPtr)
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("Myra: Skipping duplicate attribute initialization effect '%s' on ASC '%s' from '%s'. Use only one initialization path for a given init effect."),
				*GetNameSafe(EffectClassPtr),
				*GetNameSafe(GetOwner()),
				*GetNameSafe(SourceObject));
			return FActiveGameplayEffectHandle();
		}
	}

	const FActiveGameplayEffectHandle Handle = ApplyEffectToSelf(EffectClass, Level);
	if (Handle.IsValid())
	{
		AppliedInitializationEffects.Add(Handle, EffectClassPtr);
	}

	return Handle;
}

void UMyraAbilitySystemComponent::RemoveTrackedGameplayEffect(const FActiveGameplayEffectHandle& EffectHandle)
{
	if (!EffectHandle.IsValid())
	{
		return;
	}

	RemoveActiveGameplayEffect(EffectHandle);
	AppliedInitializationEffects.Remove(EffectHandle);
}

void UMyraAbilitySystemComponent::AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate;

	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (!AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			continue;
		}

		AbilitySpecInputPressed(AbilitySpec);

		if (!AbilitySpec.IsActive())
		{
			AbilitiesToActivate.AddUnique(AbilitySpec.Handle);
		}
	}

	for (const FGameplayAbilitySpecHandle& AbilityHandle : AbilitiesToActivate)
	{
		TryActivateAbility(AbilityHandle);
	}
}

void UMyraAbilitySystemComponent::AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (!AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag) || !AbilitySpec.IsActive())
		{
			continue;
		}

		AbilitySpecInputReleased(AbilitySpec);
	}
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

void UMyraAbilitySystemComponent::NotifyGameplayEffectExecuted(const FMyraGEExecutedInfo& Info)
{
	OnGameplayEffectAttributeExecuted.Broadcast(Info);
}
