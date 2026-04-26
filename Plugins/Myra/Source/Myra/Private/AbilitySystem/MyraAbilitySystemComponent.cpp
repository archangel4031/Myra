// Copyright Myra . All Rights Reserved.

#include "AbilitySystem/MyraAbilitySystemComponent.h"
#include "DataAssets/MyraAbilitySet.h"
#include "AbilitySystem/MyraDefaultAttributeSet.h"
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

void UMyraAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);
	EnsureDefaultAttributeSet();
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
//  NotifyAttributeChanged is called by UMyraBaseAttributeSet::PostAttributeChange,
//  which is a valid UAttributeSet virtual. The AttributeSet calls this method
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

void UMyraAbilitySystemComponent::EnsureDefaultAttributeSet()
{
	if (HasDefaultAttributeSet())
	{
		return;
	}

	AActor* MyraOwnerActor = GetOwner();
	if (!MyraOwnerActor)
	{
		return;
	}

	TArray<UObject*> OwnerSubobjects;
	GetObjectsWithOuter(MyraOwnerActor, OwnerSubobjects, false);

	for (UObject* Object : OwnerSubobjects)
	{
		if (!Object)
		{
			continue;
		}

		if (Object->HasAnyFlags(RF_DefaultSubObject))
		{
			if (UMyraDefaultAttributeSet* DefaultAttributeSet = Cast<UMyraDefaultAttributeSet>(Object))
			{
				AddAttributeSetSubobject(DefaultAttributeSet);
				return;
			}
		}
	}

	UMyraDefaultAttributeSet* DefaultAttributeSet = NewObject<UMyraDefaultAttributeSet>(
		MyraOwnerActor,
		UMyraDefaultAttributeSet::StaticClass(),
		TEXT("MyraDefaultAttributeSet"));

	AddAttributeSetSubobject(DefaultAttributeSet);
}

bool UMyraAbilitySystemComponent::HasDefaultAttributeSet() const
{
	return GetSet<UMyraDefaultAttributeSet>() != nullptr;
}

// ------------------------------------------------
//  Ability Info Queries (for UI)
// ------------------------------------------------

float UMyraAbilitySystemComponent::GetAbilityCostByInputTag(FGameplayTag InputTag) const
{
	if (!InputTag.IsValid()) { return 0.f; }

	for (const FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		if (!Spec.GetDynamicSpecSourceTags().HasTagExact(InputTag)) { continue; }

		// Spec.Ability is the CDO — cost reads from the GE CDO so this is safe.
		if (const UMyraGameplayAbility* Ability = Cast<UMyraGameplayAbility>(Spec.Ability))
		{
			return Ability->GetAbilityCostAmount();
		}
	}
	return 0.f;
}

float UMyraAbilitySystemComponent::GetAbilityCooldownDurationByInputTag(FGameplayTag InputTag) const
{
	if (!InputTag.IsValid()) { return 0.f; }

	for (const FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		if (!Spec.GetDynamicSpecSourceTags().HasTagExact(InputTag)) { continue; }

		// Duration reads from the GE CDO — no per-instance state needed.
		if (const UMyraGameplayAbility* Ability = Cast<UMyraGameplayAbility>(Spec.Ability))
		{
			return Ability->GetAbilityCooldownDuration();
		}
	}
	return 0.f;
}

float UMyraAbilitySystemComponent::GetAbilityCooldownRemainingByInputTag(FGameplayTag InputTag) const
{
	if (!InputTag.IsValid()) { return 0.f; }

	for (const FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		if (!Spec.GetDynamicSpecSourceTags().HasTagExact(InputTag)) { continue; }

		const UMyraGameplayAbility* AbilityCDO = Cast<UMyraGameplayAbility>(Spec.Ability);
		if (!AbilityCDO) { continue; }

		// Prefer the live instance — GetAbilityCooldownTimeRemaining() needs CurrentActorInfo
		// which is only set on the instance, not the CDO.
		if (UGameplayAbility* Instance = Spec.GetPrimaryInstance())
		{
			if (UMyraGameplayAbility* MyraInstance = Cast<UMyraGameplayAbility>(Instance))
			{
				return MyraInstance->GetAbilityCooldownTimeRemaining();
			}
		}

		// Fallback (ability not yet activated / no instance yet):
		// replicate the query directly against the ASC using the CDO's cooldown tags.
		const FGameplayTagContainer* CooldownTags = AbilityCDO->GetCooldownTags();
		if (!CooldownTags || CooldownTags->IsEmpty()) { return 0.f; }

		FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(*CooldownTags);
		TArray<float> Durations = GetActiveEffectsTimeRemaining(Query);
		if (Durations.Num() > 0)
		{
			Durations.Sort();
			return Durations.Last();
		}
		return 0.f;
	}
	return 0.f;
}

// ------------------------------------------------
//  Cooldown Queries — by Granted Tag
// ------------------------------------------------

float UMyraAbilitySystemComponent::GetCooldownRemainingByGrantedTag(FGameplayTag GrantedTag) const
{
	if (!GrantedTag.IsValid()) { return 0.f; }

	FGameplayTagContainer Tags;
	Tags.AddTag(GrantedTag);
	return GetCooldownRemainingByGrantedTags(Tags);
}

float UMyraAbilitySystemComponent::GetCooldownRemainingByGrantedTags(const FGameplayTagContainer& GrantedTags) const
{
	if (GrantedTags.IsEmpty()) { return 0.f; }

	// MakeQuery_MatchAnyOwningTags finds active effects that GRANT any of these
	// tags to the owner — the same mechanism GAS uses internally for cooldowns.
	const FGameplayEffectQuery Query =
		FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(GrantedTags);

	TArray<float> Durations = GetActiveEffectsTimeRemaining(Query);
	if (Durations.Num() > 0)
	{
		Durations.Sort();
		return Durations.Last(); // longest remaining, consistent with GetAbilityCooldownTimeRemaining
	}
	return 0.f;
}

// ------------------------------------------------
//  Cooldown Queries — by Asset Tag
// ------------------------------------------------

float UMyraAbilitySystemComponent::GetCooldownRemainingByAssetTag(FGameplayTag AssetTag) const
{
	if (!AssetTag.IsValid()) { return 0.f; }

	FGameplayTagContainer Tags;
	Tags.AddTag(AssetTag);
	return GetCooldownRemainingByAssetTags(Tags);
}

float UMyraAbilitySystemComponent::GetCooldownRemainingByAssetTags(const FGameplayTagContainer& AssetTags) const
{
	if (AssetTags.IsEmpty()) { return 0.f; }

	// MakeQuery_MatchAnyEffectTags finds active effects whose own asset tag
	// container contains any of the given tags. These tags are NOT on the owner —
	// they identify the GE class itself (same as EffectTags in FMyraGEExecutedInfo).
	const FGameplayEffectQuery Query =
		FGameplayEffectQuery::MakeQuery_MatchAnyEffectTags(AssetTags);

	TArray<float> Durations = GetActiveEffectsTimeRemaining(Query);
	if (Durations.Num() > 0)
	{
		Durations.Sort();
		return Durations.Last();
	}
	return 0.f;
}