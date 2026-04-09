// Copyright Myra . All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "MyraAbilitySystemComponent.generated.h"

class UMyraGameplayAbility;
class UMyraAbilitySet;

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

	/**
	 * Apply a Gameplay Effect to self by class.
	 * Wrapper so beginners can call this from Blueprint without needing a handle.
	 */
	UFUNCTION(BlueprintCallable, Category = "Myra |Effects")
	FActiveGameplayEffectHandle ApplyEffectToSelf(TSubclassOf<UGameplayEffect> EffectClass, float Level = 1.f);

	/**
	 * Apply an attribute initialization effect only once per ASC.
	 * Use this for your one chosen initialization path: Character, CharacterData, or AbilitySet.
	 * Duplicate applications of the same effect class are skipped with a warning.
	 */
	FActiveGameplayEffectHandle ApplyInitializationEffectOnce(
		TSubclassOf<UGameplayEffect> EffectClass,
		float Level,
		const UObject* SourceObject);

	// ------------------------------------------------
	//  Events
	// ------------------------------------------------

	/** Fires whenever any attribute on this ASC changes. Useful for driving UI. */
	UPROPERTY(BlueprintAssignable, Category = "Myra |Events")
	FMyraAttributeChangedDelegate OnAttributeChanged;


	/**
	 * Called by UMyraAttributeSet::PostAttributeChange to broadcast OnAttributeChanged.
	 * PostAttributeChange is a virtual on UAttributeSet (not UAbilitySystemComponent),
	 * so the AttributeSet calls this method to push the event up to the ASC where
	 * Blueprint UI widgets can bind to it.
	 */
	void NotifyAttributeChanged(const FGameplayAttribute& Attribute, float OldValue, float NewValue);

protected:


	/** Tracks which AbilitySets have been granted so we can remove them cleanly. */
	UPROPERTY()
	TArray<TObjectPtr<UMyraAbilitySet>> GrantedAbilitySets;

	/** Handles returned when granting abilities from sets, keyed by AbilitySet pointer. */
	TMap<const UMyraAbilitySet*, TArray<FGameplayAbilitySpecHandle>> GrantedAbilityHandles;
	TMap<const UMyraAbilitySet*, TArray<FActiveGameplayEffectHandle>> GrantedEffectHandles;

	/** Tracks attribute init effect classes already applied so only one init path wins. */
	TSet<const UClass*> AppliedInitializationEffectClasses;
};
