// Copyright Myra . All Rights Reserved.
#pragma once

#include "NativeGameplayTags.h"

/**
 * MyraNativeGameplayTags
 *
 * Declares all Gameplay Tags used internally by Myra  as compile-time
 * native tags. Using UE_DECLARE_GAMEPLAY_TAG_EXTERN / UE_DEFINE_GAMEPLAY_TAG
 * means:
 *   1. Tags are registered automatically at module startup — no manual GameplayTagsList.ini editing.
 *   2. Usage is a simple extern variable reference, not an FName string lookup.
 *   3. Typos cause compile errors, not silent runtime mismatches.
 *
 * HOW TO USE IN YOUR CODE:
 *   #include "Tags/MyraNativeGameplayTags.h"
 *   ASC->AddLooseGameplayTag(MyraGameplayTags::State_Dead);
 *
 * HOW TO ADD YOUR OWN TAGS:
 *   Add UE_DECLARE_GAMEPLAY_TAG_EXTERN here and UE_DEFINE_GAMEPLAY_TAG in the .cpp.
 *   Or just use the Project Settings → Gameplay Tags editor for tags that don't
 *   need to be referenced from C++.
 */
namespace MyraGameplayTags
{
	// -------------------------------------------------------
	//  State Tags
	//  Applied to the ASC to represent the character's current state.
	// -------------------------------------------------------

	/** Applied when the character's Health reaches zero. Blocks all abilities. */
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Dead);

	/** Applied while the character is being respawned / initializing. */
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Spawning);

	/** Applied while ANY ability is actively running. */
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_AbilityActivating);

	/** Applied to block all abilities (e.g. during a cinematic). */
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_BlockAllAbilities);

	// -------------------------------------------------------
	//  Event Tags
	//  Sent via UAbilitySystemBlueprintLibrary::SendGameplayEventToActor.
	// -------------------------------------------------------

	/** Fired when Health hits zero (from PostGameplayEffectExecute on MyraAttributeSet). */
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameEvent_Death);

	/** Fired when healing fully restores Health to MaxHealth. */
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameEvent_FullHeal);

	// -------------------------------------------------------
	//  Input Tags
	//  Match these against the InputTag set on FMyraAbilitySet_GameplayAbility
	//  entries. MyraInputComponent uses these to bind ability activation to input.
	// -------------------------------------------------------

	/** Primary attack / ability slot 1 */
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Slot1);

	/** Secondary attack / ability slot 2 */
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Slot2);

	/** Ultimate / ability slot 3 */
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Slot3);

	/** Ability slot 4 */
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Slot4);

	/** Confirm an ability target (e.g. confirm placement of an AoE). */
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Confirm);

	/** Cancel an ability mid-execution. */
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Cancel);

} // namespace MyraGameplayTags
