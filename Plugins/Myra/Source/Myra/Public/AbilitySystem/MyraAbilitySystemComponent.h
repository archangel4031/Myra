// Copyright Myra . All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "MyraAbilitySystemComponent.generated.h"

class UMyraGameplayAbility;
class UMyraAbilitySet;
class UMyraDefaultAttributeSet;
class UAttributeSet;

/**
 * FMyraGEExecutedInfo
 *
 * Carried by OnGameplayEffectAttributeExecuted whenever a Gameplay Effect
 * executes on an attribute (instant GE or periodic tick).
 *
 * Key difference from OnAttributeChanged:
 *   - This fires server-side only, inside PostGameplayEffectExecute.
 *   - It carries the full GE context: who caused it, its tags, raw magnitude.
 *
 * For UI updates use OnAttributeChanged instead — it fires after replication
 * on all clients and is the right signal for health bars etc.
 */
USTRUCT(BlueprintType)
struct FMyraGEExecutedInfo
{
	GENERATED_BODY()

	/** The attribute that the Gameplay Effect targeted. */
	UPROPERTY(BlueprintReadOnly, Category = "Myra|GE Info")
	FGameplayAttribute Attribute;

	/**
	 * Current value of the attribute AFTER the GE was fully processed
	 * (including meta-attribute conversion in PostGameplayEffectExecute).
	 * For meta attributes (Damage, Healing) this will be 0 because
	 * they are zeroed out after conversion — use Magnitude for the raw amount.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Myra|GE Info")
	float NewValue = 0.f;

	/** The raw magnitude the GE applied to the attribute. Always meaningful. */
	UPROPERTY(BlueprintReadOnly, Category = "Myra|GE Info")
	float Magnitude = 0.f;

	/**
	 * The actor that owns the ASC which sourced this effect.
	 * e.g. the player character who cast the ability.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Myra|GE Info")
	TObjectPtr<AActor> Instigator = nullptr;

	/**
	 * The physical actor that caused the hit (e.g. a projectile or melee weapon).
	 * May be the same as Instigator if there is no intermediary actor.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Myra|GE Info")
	TObjectPtr<AActor> EffectCauser = nullptr;

	/**
	 * Asset tags defined on the Gameplay Effect class.
	 * Use these to identify effect types (e.g. "Damage.Fire", "Buff.Speed")
	 * without hard-coding attribute names.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Myra|GE Info")
	FGameplayTagContainer EffectTags;
};

/**
 * Delegate broadcast whenever any attribute changes on this ASC.
 * UI elements can bind to this to update health bars, mana bars, etc.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FMyraAttributeChangedDelegate,
	FGameplayAttribute, Attribute,
	float, OldValue,
	float, NewValue);

/**
 * Broadcast on the server each time a Gameplay Effect executes on any attribute.
 * Carries the full execution context including instigator, causer, and effect tags.
 *
 * Note: this is a server-side event. For client UI, bind to OnAttributeChanged instead.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FMyraGEExecutedDelegate,
	const FMyraGEExecutedInfo&, Info);


/**
 * UMyraAbilitySystemComponent
 *
 * The central Ability System Component for Myra .
 * Place this on your PlayerState (multiplayer) or Character (single-player).
 *
 * Key responsibilities:
 *   - Tracks which AbilitySets have been granted (avoids duplicates)
 *   - Broadcasts Blueprint-friendly attribute change events
 *   - Exposes helper functions beginners commonly need
 */
UCLASS(ClassGroup = "Myra", meta = (BlueprintSpawnableComponent))
class MYRA_API UMyraAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:

	UMyraAbilitySystemComponent();

	virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;

	// ------------------------------------------------
	//  Ability Set Granting
	// ------------------------------------------------

	/**
	 * Grant all abilities, attribute sets, and startup effects defined in an AbilitySet.
	 * Safe to call multiple times — duplicate sets are ignored.
	 * @param AbilitySet  The data asset defining what to grant.
	 * @param SourceObject Optional object to tag grants with (useful for removal later).
	 */
	UFUNCTION(BlueprintCallable, Category = "Myra |Abilities")
	void GrantAbilitySet(UMyraAbilitySet* AbilitySet, UObject* SourceObject = nullptr);

	/**
	 * Remove all grants that were given by a specific AbilitySet.
	 */
	UFUNCTION(BlueprintCallable, Category = "Myra |Abilities")
	void RemoveAbilitySet(UMyraAbilitySet* AbilitySet);

	// ------------------------------------------------
	//  Blueprint Helpers
	// ------------------------------------------------

	/** Returns current value of any attribute by name (e.g. "Health"). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Myra |Attributes")
	float GetAttributeValue(FGameplayAttribute Attribute) const;

	/** Returns true if the attribute exists on this ASC. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Myra |Attributes")
	bool HasAttribute(FGameplayAttribute Attribute) const;

	/** Returns true if this ASC already owns an AttributeSet instance of the exact class. */
	bool HasAttributeSetOfClass(TSubclassOf<UAttributeSet> AttributeSetClass) const;

	/**
	 * Apply a Gameplay Effect to self by class.
	 * Wrapper so beginners can call this from Blueprint without needing a handle.
	 */
	UFUNCTION(BlueprintCallable, Category = "Myra |Effects")
	FActiveGameplayEffectHandle ApplyEffectToSelf(TSubclassOf<UGameplayEffect> EffectClass, float Level = 1.f);

	/**
	 * Apply an attribute initialization effect only once per ASC.
	 * Use this for your one chosen initialization path: Character, PawnData, or AbilitySet.
	 * Duplicate applications of the same effect class are skipped with a warning.
	 */
	FActiveGameplayEffectHandle ApplyInitializationEffectOnce(
		TSubclassOf<UGameplayEffect> EffectClass,
		float Level,
		const UObject* SourceObject);

	/** Removes a tracked gameplay effect and releases any Myra init-effect tracking attached to it. */
	void RemoveTrackedGameplayEffect(const FActiveGameplayEffectHandle& EffectHandle);

	/** Forwards an input press tag to matching abilities and activates them if needed. */
	UFUNCTION(BlueprintCallable, Category = "Myra |Abilities")
	void AbilityInputTagPressed(FGameplayTag InputTag);

	/** Forwards an input release tag to any active matching abilities. */
	UFUNCTION(BlueprintCallable, Category = "Myra |Abilities")
	void AbilityInputTagReleased(FGameplayTag InputTag);

	// ------------------------------------------------
	//  Events
	// ------------------------------------------------

	/** Fires whenever any attribute on this ASC changes. Useful for driving UI. */
	UPROPERTY(BlueprintAssignable, Category = "Myra |Events")
	FMyraAttributeChangedDelegate OnAttributeChanged;

	/**
	 * Fires on the server each time any Gameplay Effect executes on an attribute
	* belonging to an AttributeSet on this ASC.
	*
	* Bind to this from the Character, a Game Mode, or any server-side object that
	* needs to react to GE execution with full context (who, what magnitude, what tags).
	*/
	UPROPERTY(BlueprintAssignable, Category = "Myra|Events")
	FMyraGEExecutedDelegate OnGameplayEffectAttributeExecuted;


	/**
	 * Called by UMyraBaseAttributeSet::PostAttributeChange to broadcast OnAttributeChanged.
	 * PostAttributeChange is a virtual on UAttributeSet (not UAbilitySystemComponent),
	 * so the AttributeSet calls this method to push the event up to the ASC where
	 * Blueprint UI widgets can bind to it.
	 */
	void NotifyAttributeChanged(const FGameplayAttribute& Attribute, float OldValue, float NewValue);

	/**
	 * Called by UMyraBaseAttributeSet::PostGameplayEffectExecute to push the execution
	 * event up to the ASC where Blueprint objects can bind to it.
	 * You should not need to call this directly.
	 */
	void NotifyGameplayEffectExecuted(const FMyraGEExecutedInfo& Info);

protected:


	/** Tracks which AbilitySets have been granted so we can remove them cleanly. */
	UPROPERTY()
	TArray<TObjectPtr<UMyraAbilitySet>> GrantedAbilitySets;

	/** Handles returned when granting abilities from sets, keyed by AbilitySet pointer. */
	TMap<const UMyraAbilitySet*, TArray<FGameplayAbilitySpecHandle>> GrantedAbilityHandles;
	TMap<const UMyraAbilitySet*, TArray<FActiveGameplayEffectHandle>> GrantedEffectHandles;
	TMap<const UMyraAbilitySet*, TArray<TWeakObjectPtr<UAttributeSet>>> GrantedAttributeSetHandles;

	/** Tracks init effects by handle so pawn-scoped init effects can be removed and re-applied later. */
	TMap<FActiveGameplayEffectHandle, const UClass*> AppliedInitializationEffects;

private:

	void EnsureDefaultAttributeSet();
	bool HasDefaultAttributeSet() const;
};
