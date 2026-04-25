// Copyright Myra . All Rights Reserved.

#include "Tags/MyraNativeGameplayTags.h"

/**
 * UE_DEFINE_GAMEPLAY_TAG registers the tag with the global Gameplay Tag manager
 * at static initialization time. The string must match a tag in your project's
 * GameplayTagsList.ini — OR you can rely on these definitions alone since
 * NativeGameplayTags auto-adds them.
 *
 * Tag hierarchy:
 *   State.*        — Character state flags (alive, dead, spawning...)
 *   GameEvent.*    — One-shot events fired via SendGameplayEventToActor
 *   Input.Ability.*— Input binding tags matched by MyraInputComponent
 */
namespace MyraGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_State_Dead,
		"Myra.State.Dead",
		"Applied when Health reaches zero. MyraDefaultAttributeSet adds this tag; MyraCharacter listens for it.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_State_Spawning,
		"Myra.State.Spawning",
		"Applied while the character is being initialized. Cleared in OnAbilitySystemInitialized.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_State_AbilityActivating,
		"Myra.State.AbilityActivating",
		"Applied automatically by Myra while any ability is active. Use to drive animation blend.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_State_BlockAllAbilities,
		"Myra.State.BlockAllAbilities",
		"Blocks all ability activation. Apply via GE during cinematics or UI menus.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_GameEvent_Death,
		"Myra.GameEvent.Death",
		"Sent via gameplay event when Health hits 0. UMyraDefaultAttributeSet fires this.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_GameEvent_FullHeal,
		"Myra.GameEvent.FullHeal",
		"Sent when healing restores Health to MaxHealth.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_Input_Ability_Slot1,
		"Myra.Input.Ability.Slot1",
		"Bind your primary attack or ability to this tag in your MyraAbilitySet.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_Input_Ability_Slot2,
		"Myra.Input.Ability.Slot2",
		"Secondary ability slot.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_Input_Ability_Slot3,
		"Myra.Input.Ability.Slot3",
		"Ability slot 3 / Ultimate.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_Input_Ability_Slot4,
		"Myra.Input.Ability.Slot4",
		"Ability slot 4.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_Input_Ability_Confirm,
		"Myra.Input.Ability.Confirm",
		"Confirm an ability targeting session.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_Input_Ability_Cancel,
		"Myra.Input.Ability.Cancel",
		"Cancel an ability targeting session.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_Ability_Activate,
		"Myra.Ability.Activate",
		"Generic ability activation trigger - used by most abilities");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_Ability_BaseAttack,
		"Myra.Ability.BaseAttack",
		"Basic attack ability (melee or ranged)");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_Ability_Move,
		"Myra.Ability.Move",
		"Generic movement ability");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_Ability_Jump,
		"Myra.Ability.Jump",
		"Jump ability");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_Ability_Interact,
		"Myra.Ability.Interact",
		"Interact with objects / NPCs in the world");

	/** === FPS / Shooter Genre === */
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_Ability_FPS_FireWeapon,
		"Myra.Ability.FPS.FireWeapon",
		"FPS - Fire current weapon");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_Ability_FPS_Reload,
		"Myra.Ability.FPS.Reload",
		"FPS - Reload current weapon");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_Ability_FPS_AimDownSight,
		"Myra.Ability.FPS.AimDownSight",
		"FPS - Aim Down Sights");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_Ability_FPS_Grenade,
		"Myra.Ability.FPS.Grenade",
		"FPS - Throw grenade");

	/** === RPG / Action-RPG Genre === */
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_Ability_RPG_CastSpell,
		"Myra.Ability.RPG.CastSpell",
		"RPG - Cast a spell");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_Ability_RPG_UseSkill,
		"Myra.Ability.RPG.UseSkill",
		"RPG - Use an active skill");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_Ability_RPG_UseItem,
		"Myra.Ability.RPG.UseItem",
		"RPG - Use a consumable item");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_Ability_RPG_Dash,
		"Myra.Ability.RPG.Dash",
		"RPG - Dash / dodge ability");

	/** === Strategy / RTS / Tower Defense Genre === */
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_Ability_Strategy_Build,
		"Myra.Ability.Strategy.Build",
		"Strategy - Build a structure or unit");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_Ability_Strategy_Recruit,
		"Myra.Ability.Strategy.Recruit",
		"Strategy - Recruit a new unit");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_Ability_Strategy_Upgrade,
		"Myra.Ability.Strategy.Upgrade",
		"Strategy - Upgrade building or unit");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_Ability_Strategy_Select,
		"Myra.Ability.Strategy.Select",
		"Strategy - Select unit or building");

	/** === Platformer / Adventure Genre === */
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_Ability_Platformer_DoubleJump,
		"Myra.Ability.Platformer.DoubleJump",
		"Platformer - Double jump");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_Ability_Platformer_WallRun,
		"Myra.Ability.Platformer.WallRun",
		"Platformer - Wall run");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_Ability_Platformer_Glide,
		"Myra.Ability.Platformer.Glide",
		"Platformer - Glide ability");

	/** === Gameplay Events === */
	//UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_GameplayEvent_Death,
	//	"Myra.GameplayEvent.Death",
	//	"Character died (sent via SendGameplayEvent)");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_GameplayEvent_Respawn,
		"Myra.GameplayEvent.Respawn",
		"Character respawned");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_GameplayEvent_DamageTaken,
		"Myra.GameplayEvent.DamageTaken",
		"Received any damage");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_GameplayEvent_Healed,
		"Myra.GameplayEvent.Healed",
		"Received healing");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_GameplayEvent_LevelUp,
		"Myra.GameplayEvent.LevelUp",
		"Player leveled up");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_GameplayEvent_EnemyKilled,
		"Myra.GameplayEvent.EnemyKilled",
		"Enemy was killed");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_GameplayEvent_ObjectiveCompleted,
		"Myra.GameplayEvent.ObjectiveCompleted",
		"Objective / quest completed");

	/** === State Tags === */
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_State_Alive,
		"Myra.State.Alive",
		"Character is alive");

	//UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_State_Dead,
	//	"Myra.State.Dead",
	//	"Character is dead");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_State_Stunned,
		"Myra.State.Stunned",
		"Cannot act (stunned)");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_State_Invincible,
		"Myra.State.Invincible",
		"Immune to all damage");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_State_InCombat,
		"Myra.State.InCombat",
		"Currently in combat");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_State_Downed,
		"Myra.State.Downed",
		"Downed but not dead (e.g. battle royale)");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_State_Immobilized,
		"Myra.State.Immobilized",
		"Cannot move");

	/** === Effect Tags === */
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_Effect_Damage,
		"Myra.Effect.Damage",
		"Damage over time or instant damage effect");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_Effect_Heal,
		"Myra.Effect.Heal",
		"Healing effect");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_Effect_Buff,
		"Myra.Effect.Buff",
		"Positive temporary effect");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_Effect_Debuff,
		"Myra.Effect.Debuff",
		"Negative temporary effect");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_Effect_Stun,
		"Myra.Effect.Stun",
		"Stun effect");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_Effect_Poison,
		"Myra.Effect.Poison",
		"Poison / Damage-over-Time effect");

	/** === Input Tags === */
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_Input_Move,
		"Myra.Input.Move",
		"Bind to Enhanced Input Move action");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_Input_Look,
		"Myra.Input.Look",
		"Bind to Enhanced Input Look / camera action");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_Input_Jump,
		"Myra.Input.Jump",
		"Bind to Enhanced Input Jump action");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_Input_Attack,
		"Myra.Input.Attack",
		"Bind to basic attack input");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_Input_AbilityPrimary,
		"Myra.Input.AbilityPrimary",
		"Bind your primary ability / slot 1 here in MyraAbilitySet");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Myra_Input_AbilitySecondary,
		"Myra.Input.AbilitySecondary",
		"Bind your secondary ability / slot 2 here in MyraAbilitySet");

} // namespace MyraGameplayTags
