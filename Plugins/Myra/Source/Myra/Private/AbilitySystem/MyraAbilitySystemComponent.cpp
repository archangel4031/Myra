// Copyright Myra . All Rights Reserved.

#include "AbilitySystem/MyraAbilitySystemComponent.h"
#include "DataAssets/MyraAbilitySet.h"
#include "AbilitySystem/MyraDefaultAttributeSet.h"
#include "AbilitySystem/MyraGameplayAbility.h"
#include "Character/MyraPlayerState.h"
#include "MyraDeveloperSettings.h"
#include "Pawn/MyraPawnAbilityComponent.h"
#include "AttributeSet.h"
#include "UObject/UObjectGlobals.h"
#include "Logging/MyraLog.h"

UMyraAbilitySystemComponent::UMyraAbilitySystemComponent()
{
	ReplicationMode = UMyraDeveloperSettings::Get()->DefaultReplicationMode;
}

void UMyraAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);
	EnsureDefaultAttributeSet();
}

// ------------------------------------------------
//  Ability Set Granting
// ------------------------------------------------

bool UMyraAbilitySystemComponent::GrantAbilitySet(UMyraAbilitySet* AbilitySet, UObject* SourceObject)
{
	if (!AbilitySet)
	{
		UE_LOG(LogMyra, Warning, TEXT("Myra: GrantAbilitySet called with null AbilitySet."));
		return false;
	}

	// Abilities can only be granted on the server (or in standalone).
	// Calling this on a client is a no-op — the server replicates the grants automatically.
	if (!IsOwnerActorAuthoritative())
	{
		UE_LOG(
			LogMyra,
			Warning,
			TEXT("Myra: GrantAbilitySet('%s') was called on a non-authority instance of ASC '%s'. "
				"Abilities must be granted on the server only — they replicate automatically to clients. "
				"Check your call site and ensure it runs on the server."),
			*GetNameSafe(AbilitySet),
			*GetNameSafe(GetOwner()));
		return false;
	}

	// Don't grant the same set twice
	if (GrantedAbilitySets.Contains(AbilitySet))
	{
		UE_LOG(
			LogMyra,
			Warning,
			TEXT("Myra: Duplicate AbilitySet grant prevented. AbilitySet '%s' is already granted to ASC '%s'; duplicate request from '%s' was ignored to avoid duplicate abilities, effects, and attribute sets."),
			*GetNameSafe(AbilitySet),
			*GetNameSafe(GetOwner()),
			*GetNameSafe(SourceObject));
		return false;
	}

	GrantedAbilitySets.Add(AbilitySet);

	TArray<FGameplayAbilitySpecHandle>& AbilityHandles = GrantedAbilityHandles.FindOrAdd(AbilitySet);
	TArray<FActiveGameplayEffectHandle>& EffectHandles = GrantedEffectHandles.FindOrAdd(AbilitySet);
	TArray<TWeakObjectPtr<UAttributeSet>>& AttributeSetHandles = GrantedAttributeSetHandles.FindOrAdd(AbilitySet);

	AbilitySet->GiveToAbilitySystem(this, SourceObject, AbilityHandles, EffectHandles, AttributeSetHandles);

	UE_LOG(
		LogMyra,
		Log,
		TEXT("Myra: Granted AbilitySet '%s' to ASC '%s' from '%s'. Granted %d abilities, %d effects, and %d attribute sets."),
		*GetNameSafe(AbilitySet),
		*GetNameSafe(GetOwner()),
		*GetNameSafe(SourceObject),
		GrantedAbilityHandles[AbilitySet].Num(),
		GrantedEffectHandles[AbilitySet].Num(),
		GrantedAttributeSetHandles[AbilitySet].Num());

	return true;
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

TArray<FMyraGrantedAbilityInfo> UMyraAbilitySystemComponent::GetGrantedAbilityInfos() const
{
	TArray<FMyraGrantedAbilityInfo> Result;

	// GetActivatableAbilities() returns the ASC's live spec array.
	// It is const-safe to iterate here because we only read from it.
	for (const FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		UMyraGameplayAbility* AbilityCDO = Cast<UMyraGameplayAbility>(Spec.Ability);
		if (!AbilityCDO)
		{
			// Skip non-Myra abilities (engine internals, etc.)
			continue;
		}

		FMyraGrantedAbilityInfo Info;
		Info.AbilityCDO = AbilityCDO;
		Info.AbilityLevel = Spec.Level;
		Info.SpecHandle = Spec.Handle;

		// Cooldown GE class
		if (const UGameplayEffect* CooldownGE = AbilityCDO->GetCooldownGameplayEffect())
		{
			Info.CooldownEffectClass = CooldownGE->GetClass();
		}

		// Cooldown granted tags — use GetCooldownTags() on the ability CDO.
		// This is the canonical accessor: it handles both pre-5.3 direct-field style
		// AND UE5.3+ GE Component style (UTargetTagsGameplayEffectComponent)
		// without us needing to touch UGameplayEffect internals at all.
		const FGameplayTagContainer* CooldownTagsPtr = AbilityCDO->GetCooldownTags();
		if (CooldownTagsPtr && !CooldownTagsPtr->IsEmpty())
		{
			Info.CooldownGrantedTags = *CooldownTagsPtr;
		}

		// Cost GE class
		if (const UGameplayEffect* CostGE = AbilityCDO->GetCostGameplayEffect())
		{
			Info.CostEffectClass = CostGE->GetClass();
		}

		// Input tag is stored in the spec's DynamicAbilityTags — this is set by
		// UMyraAbilitySet::GiveToAbilitySystem when it calls
		// AbilitySpec.GetDynamicSpecSourceTags().AddTag(Entry.InputTag).
		// We grab the first tag under Myra.Input (or just any tag if none match).
		// If the ability has no input tag, InputTag stays invalid, which is fine.
		const FGameplayTagContainer& DynamicTags = Spec.GetDynamicSpecSourceTags();
		for (const FGameplayTag& Tag : DynamicTags)
		{
			// Accept any valid tag — InputTag is the only thing stored in DynamicSpecSourceTags
			// by Myra's ability granting path, so any tag found here IS the input tag.
			if (Tag.IsValid())
			{
				Info.InputTag = Tag;
				break;
			}
		}

		Result.Add(MoveTemp(Info));
	}

	return Result;
}

UMyraGameplayAbility* UMyraAbilitySystemComponent::GetGrantedAbilityCDOByInputTag(FGameplayTag InputTag) const
{
	if (!InputTag.IsValid())
	{
		return nullptr;
	}

	for (const FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		if (!Spec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			continue;
		}

		return Cast<UMyraGameplayAbility>(Spec.Ability);
	}

	return nullptr;
}

FGameplayAbilitySpecHandle UMyraAbilitySystemComponent::GiveAbilityWithInputTag(
	TSubclassOf<UGameplayAbility> AbilityClass,
	int32 Level,
	FGameplayTag InputTag)
{
	// Abilities can only be granted on the authoritative side; they replicate automatically.
	if (!IsOwnerActorAuthoritative())
	{
		UE_LOG(LogMyra, Warning,
			TEXT("Myra: GiveAbilityWithInputTag called on non-authoritative actor '%s'. "
				"Call this on the server only."),
			*GetNameSafe(GetOwner()));
		return FGameplayAbilitySpecHandle();
	}

	if (!AbilityClass)
	{
		UE_LOG(LogMyra, Warning,
			TEXT("Myra: GiveAbilityWithInputTag called with a null AbilityClass."));
		return FGameplayAbilitySpecHandle();
	}

	FGameplayAbilitySpec Spec(AbilityClass, Level);

	// Store the input tag in DynamicSpecSourceTags so AbilityInputTagPressed /
	// AbilityInputTagReleased and BindAbilityActions can find this ability.
	// This mirrors what UMyraAbilitySet::GiveToAbilitySystem does internally.
	if (InputTag.IsValid())
	{
		Spec.GetDynamicSpecSourceTags().AddTag(InputTag);
	}
	else
	{
		UE_LOG(LogMyra, Warning,
			TEXT("Myra: GiveAbilityWithInputTag for class '%s' received an invalid InputTag. "
				"The ability will be granted but will not respond to Myra input binding. "
				"Call SetAbilityInputTag later if you want to add a binding."),
			*GetNameSafe(AbilityClass));
	}

	return GiveAbility(Spec);
}

bool UMyraAbilitySystemComponent::SetAbilityInputTag(
	FGameplayAbilitySpecHandle SpecHandle, FGameplayTag NewInputTag)
{
	// Tag changes must originate on the authority and replicate via the spec.
	if (!IsOwnerActorAuthoritative())
	{
		UE_LOG(LogMyra, Warning,
			TEXT("Myra: SetAbilityInputTag must be called on the authoritative actor."));
		return false;
	}

	FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(SpecHandle);
	if (!Spec)
	{
		UE_LOG(LogMyra, Warning,
			TEXT("Myra: SetAbilityInputTag could not find a spec for the provided handle."));
		return false;
	}

	// Replace all existing DynamicSpecSourceTags with the new input tag.
	// Within Myra, DynamicSpecSourceTags is exclusively used for input tags,
	// so clearing and re-adding is always safe.
	FGameplayTagContainer& DynamicTags = Spec->GetDynamicSpecSourceTags();
	DynamicTags.Reset();

	if (NewInputTag.IsValid())
	{
		DynamicTags.AddTag(NewInputTag);
	}

	// Mark dirty so the spec replicates to clients.
	MarkAbilitySpecDirty(*Spec);
	return true;
}

bool UMyraAbilitySystemComponent::ClearAbilityInputTag(FGameplayAbilitySpecHandle SpecHandle)
{
	// Delegate to SetAbilityInputTag with an empty tag — it handles the authority
	// check and dirty marking, so this stays a thin wrapper.
	return SetAbilityInputTag(SpecHandle, FGameplayTag());
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
				LogMyra,
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
	// Void wrapper — keeps Enhanced Input's BindAction happy (requires void return).
	// The actual logic and return value live in TryActivateAbilityByInputTag.
	TryActivateAbilityByInputTag(InputTag);
}

bool UMyraAbilitySystemComponent::TryActivateAbilityByInputTag(FGameplayTag InputTag)
{
	if (!InputTag.IsValid())
	{
		return false;
	}

	TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate;

	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (!AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			continue;
		}

		// Notify the ability that its input was pressed regardless of whether
		// it is already active (important for hold/release abilities).
		AbilitySpecInputPressed(AbilitySpec);

		if (!AbilitySpec.IsActive())
		{
			AbilitiesToActivate.AddUnique(AbilitySpec.Handle);
		}
	}

	// Try activating every matching spec; report success if any one of them activates.
	bool bAnyActivated = false;
	for (const FGameplayAbilitySpecHandle& AbilityHandle : AbilitiesToActivate)
	{
		bAnyActivated |= TryActivateAbility(AbilityHandle);
	}

	return bAnyActivated;
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

int32 UMyraAbilitySystemComponent::SetGrantedAbilityLevelByClass(
	TSubclassOf<UGameplayAbility> AbilityClass,
	int32 NewLevel)
{
	if (!IsOwnerActorAuthoritative())
	{
		UE_LOG(LogMyra, Warning, TEXT("Myra: SetGrantedAbilityLevelByClass must be called on the authoritative ASC '%s'."), *GetNameSafe(this));
		return 0;
	}

	UClass* AbilityClassPtr = AbilityClass.Get();
	if (!AbilityClassPtr)
	{
		return 0;
	}

	int32 UpdatedSpecCount = 0;

	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (!AbilitySpec.Ability || !AbilitySpec.Ability->IsA(AbilityClassPtr))
		{
			continue;
		}

		if (AbilitySpec.Level == NewLevel)
		{
			continue;
		}

		AbilitySpec.Level = NewLevel;
		MarkAbilitySpecDirty(AbilitySpec);
		++UpdatedSpecCount;
	}

	return UpdatedSpecCount;
}

int32 UMyraAbilitySystemComponent::SetGrantedAbilityLevelByAbilityTag(
	FGameplayTag AbilityTag,
	int32 NewLevel)
{
	if (!IsOwnerActorAuthoritative())
	{
		UE_LOG(LogMyra, Warning, TEXT("Myra: SetGrantedAbilityLevelByAbilityTag must be called on the authoritative ASC '%s'."), *GetNameSafe(this));
		return 0;
	}

	if (!AbilityTag.IsValid())
	{
		return 0;
	}

	int32 UpdatedSpecCount = 0;

	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		const UMyraGameplayAbility* MyraAbility = Cast<UMyraGameplayAbility>(AbilitySpec.Ability);
		if (!MyraAbility || !MyraAbility->HasAbilityTag(AbilityTag))
		{
			continue;
		}

		if (AbilitySpec.Level == NewLevel)
		{
			continue;
		}

		AbilitySpec.Level = NewLevel;
		MarkAbilitySpecDirty(AbilitySpec);
		++UpdatedSpecCount;
	}

	return UpdatedSpecCount;
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

float UMyraAbilitySystemComponent::ModifyDamageBeforeApplication_Implementation(float InDamage)
{
	// Prefer the active pawn avatar so damage routing follows the currently possessed body.
	// If no pawn-specific override is present, fall back to the owning PlayerState.
	float OutDamage = InDamage;

	if (const UMyraPawnAbilityComponent* PawnAbilityComponent =
		UMyraPawnAbilityComponent::FindPawnAbilityComponent(GetAvatarActor()))
	{
		if (PawnAbilityComponent->ModifyDamageBeforeApplication(InDamage, OutDamage))
		{
			return OutDamage;
		}
	}

	if (AMyraPlayerState* MyraPlayerState = Cast<AMyraPlayerState>(GetOwnerActor()))
	{
		if (MyraPlayerState->ModifyDamageBeforeApplication(InDamage, OutDamage))
		{
			return OutDamage;
		}
	}

	// Final fallback: apply the full damage amount to Health.
	return InDamage;
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

	if (!IsOwnerActorAuthoritative())
	{
		return;
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
