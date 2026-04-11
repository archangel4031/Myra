// Copyright Myra . All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayAbilitySpec.h"
#include "ActiveGameplayEffectHandle.h"
#include "MyraAbilitySet.generated.h"

class UMyraGameplayAbility;
class UMyraAbilitySystemComponent;
class UAttributeSet;
class UGameplayEffect;

// -------------------------------------------------------
//  Entry structs — one per type of grant
// -------------------------------------------------------

/**
 * Defines a single ability to grant: which class, at what level, bound to which input tag.
 */
USTRUCT(BlueprintType)
struct FMyraAbilitySet_GameplayAbility
{
	GENERATED_BODY()

	/** The ability class to grant. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	TSubclassOf<UMyraGameplayAbility> AbilityClass;

	/** Starting level for this ability. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	int32 AbilityLevel = 1;

	/**
	 * Input tag used to bind this ability. The character will activate this ability
	 * when an input event fires with a matching Gameplay Tag.
	 * Example: "Input.Ability.PrimaryAttack"
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability",
		meta = (Categories = "Input"))
	FGameplayTag InputTag;
};

/**
 * Defines an AttributeSet class to instantiate and add to the ASC.
 */
USTRUCT(BlueprintType)
struct FMyraAbilitySet_AttributeSet
{
	GENERATED_BODY()

	/** The AttributeSet class to add. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AttributeSet")
	TSubclassOf<UAttributeSet> AttributeSetClass;
};

/**
 * Defines a Gameplay Effect to apply on grant (useful for startup buffs or stat initialisation).
 */
USTRUCT(BlueprintType)
struct FMyraAbilitySet_GameplayEffect
{
	GENERATED_BODY()

	/** The Gameplay Effect class to apply. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect")
	TSubclassOf<UGameplayEffect> GameplayEffectClass;

	/** Level to apply the effect at. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect")
	float EffectLevel = 1.f;

	/**
	 * Mark true if this effect is your attribute initialization effect.
	 * Myra will apply it through the guarded init path and skip duplicate applications
	 * from Character or CharacterData setup.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect")
	bool bTreatAsAttributeInitializationEffect = false;
};

// -------------------------------------------------------
//  UMyraAbilitySet
// -------------------------------------------------------

/**
 * UMyraAbilitySet
 *
 * A data asset that bundles abilities, attribute sets, and startup Gameplay Effects.
 * Inspired by Lyra's ULyraAbilitySet.
 *
 * HOW TO USE:
 *   1. Right-click in the Content Browser → Myra  → Ability Set.
 *   2. Fill in the arrays — abilities, attribute sets, effects.
 *   3. Drag the asset into your PlayerState's "Default Ability Sets" array,
 *      or call GrantAbilitySet() on the ASC from code.
 *
 * WHY USE THIS INSTEAD OF GRANTING ABILITIES ONE BY ONE?
 *   - All related grants are in one place (easy to audit and modify).
 *   - You can grant or remove an entire feature set at runtime (e.g. equipping a weapon
 *     grants weapon-specific abilities; unequipping removes them all).
 *   - Avoids forgetting to remove individual abilities when a feature is unloaded.
 */
UCLASS(BlueprintType, Const, meta = (DisplayName = "Myra Ability Set"))
class MYRA_API UMyraAbilitySet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UMyraAbilitySet();

	// ------------------------------------------------
	//  Configuration — fill these in the editor
	// ------------------------------------------------

	/** Abilities to grant when this set is applied. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Myra |Ability Set",
		meta = (TitleProperty = "AbilityClass"))
	TArray<FMyraAbilitySet_GameplayAbility> GrantedGameplayAbilities;

	/** Attribute sets to instantiate when this set is applied. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Myra |Ability Set",
		meta = (TitleProperty = "AttributeSetClass"))
	TArray<FMyraAbilitySet_AttributeSet> GrantedAttributeSets;

	/** Gameplay Effects to apply when this set is applied. Mark init effects explicitly so duplicates can be skipped. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Myra |Ability Set",
		meta = (TitleProperty = "GameplayEffectClass"))
	TArray<FMyraAbilitySet_GameplayEffect> GrantedGameplayEffects;

	// ------------------------------------------------
	//  Runtime API
	// ------------------------------------------------

	/**
	 * Grant everything in this set to the provided ASC.
	 * Called internally by UMyraAbilitySystemComponent::GrantAbilitySet().
	 * OutAbilityHandles and OutEffectHandles receive the handles needed for removal later.
	 */
	void GiveToAbilitySystem(
		UMyraAbilitySystemComponent* ASC,
		UObject* SourceObject,
		TArray<FGameplayAbilitySpecHandle>& OutAbilityHandles,
		TArray<FActiveGameplayEffectHandle>& OutEffectHandles,
		TArray<TWeakObjectPtr<UAttributeSet>>& OutAttributeSetHandles) const;

	// UPrimaryDataAsset interface
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};
