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
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_State_Dead);

	/** Applied while the character is being respawned / initializing. */
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_State_Spawning);

	/** Applied while ANY ability is actively running. */
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_State_AbilityActivating);

	/** Applied to block all abilities (e.g. during a cinematic). */
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_State_BlockAllAbilities);

	// -------------------------------------------------------
	//  Event Tags
	//  Sent via UAbilitySystemBlueprintLibrary::SendGameplayEventToActor.
	// -------------------------------------------------------

	/** Fired when Health hits zero (from PostGameplayEffectExecute on MyraAttributeSet). */
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_GameEvent_Death);

	/** Fired when healing fully restores Health to MaxHealth. */
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_GameEvent_FullHeal);

	// -------------------------------------------------------
	//  Input Tags
	//  Match these against the InputTag set on FMyraAbilitySet_GameplayAbility
	//  entries. MyraInputComponent uses these to bind ability activation to input.
	// -------------------------------------------------------

	/** Primary attack / ability slot 1 */
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_Input_Ability_Slot1);

	/** Secondary attack / ability slot 2 */
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_Input_Ability_Slot2);

	/** Ultimate / ability slot 3 */
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_Input_Ability_Slot3);

	/** Ability slot 4 */
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_Input_Ability_Slot4);

	/** Confirm an ability target (e.g. confirm placement of an AoE). */
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_Input_Ability_Confirm);

	/** Cancel an ability mid-execution. */
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_Input_Ability_Cancel);

	// -------------------------------------------------------
	// Other common Gameplay Tags can be added here as needed.
	// -------------------------------------------------------

	/** Generic ability activation trigger - used by most abilities */
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_Ability_Activate);

	/** Basic attack ability (melee or ranged) */
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_Ability_Attack);

	/** Generic movement ability */
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_Ability_Move);

	/** Jump ability */
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_Ability_Jump);

	/** Interact with objects / NPCs in the world */
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_Ability_Interact);

	/** === FPS / Shooter Genre === */
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_Ability_FPS_FireWeapon);
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_Ability_FPS_Reload);
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_Ability_FPS_AimDownSight);
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_Ability_FPS_Grenade);

	/** === RPG / Action-RPG Genre === */
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_Ability_RPG_CastSpell);
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_Ability_RPG_UseSkill);
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_Ability_RPG_UseItem);
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_Ability_RPG_Dash);

	/** === Strategy / RTS / Tower Defense Genre === */
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_Ability_Strategy_Build);
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_Ability_Strategy_Recruit);
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_Ability_Strategy_Upgrade);
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_Ability_Strategy_Select);

	/** === Platformer / Adventure Genre === */
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_Ability_Platformer_DoubleJump);
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_Ability_Platformer_WallRun);
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_Ability_Platformer_Glide);

	/** === Gameplay Events (sent via SendGameplayEventToActor) === */
	//MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_GameplayEvent_Death);
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_GameplayEvent_Respawn);
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_GameplayEvent_DamageTaken);
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_GameplayEvent_Healed);
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_GameplayEvent_LevelUp);
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_GameplayEvent_EnemyKilled);
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_GameplayEvent_ObjectiveCompleted);

	/** === State Tags (used with loose tags, ability activation rules, or GameplayEffects) === */
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_State_Alive);
	//MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_State_Dead);
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_State_Stunned);
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_State_Invincible);
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_State_InCombat);
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_State_Downed);
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_State_Immobilized);

	/** === Effect Tags (used when granting or querying GameplayEffects) === */
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_Effect_Damage);
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_Effect_Heal);
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_Effect_Buff);
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_Effect_Debuff);
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_Effect_Stun);
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_Effect_Poison);

	/** === Input Tags (map these in your EnhancedInput + MyraInputConfig) === */
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_Input_Move);
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_Input_Look);
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_Input_Jump);
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_Input_Attack);
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_Input_Ability1);
	MYRA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Myra_Input_Ability2);

} // namespace MyraGameplayTags
