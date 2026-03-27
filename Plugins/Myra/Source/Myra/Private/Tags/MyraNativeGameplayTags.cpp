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
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Dead,
		"State.Dead",
		"Applied when Health reaches zero. MyraAttributeSet adds this tag; MyraCharacter listens for it.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Spawning,
		"State.Spawning",
		"Applied while the character is being initialized. Cleared in OnAbilitySystemInitialized.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_AbilityActivating,
		"State.AbilityActivating",
		"Applied automatically by Myra while any ability is active. Use to drive animation blend.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_BlockAllAbilities,
		"State.BlockAllAbilities",
		"Blocks all ability activation. Apply via GE during cinematics or UI menus.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameEvent_Death,
		"GameEvent.Death",
		"Sent via gameplay event when Health hits 0. UMyraAttributeSet fires this.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameEvent_FullHeal,
		"GameEvent.FullHeal",
		"Sent when healing restores Health to MaxHealth.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Ability_Slot1,
		"Input.Ability.Slot1",
		"Bind your primary attack or ability to this tag in your MyraAbilitySet.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Ability_Slot2,
		"Input.Ability.Slot2",
		"Secondary ability slot.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Ability_Slot3,
		"Input.Ability.Slot3",
		"Ability slot 3 / Ultimate.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Ability_Slot4,
		"Input.Ability.Slot4",
		"Ability slot 4.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Ability_Confirm,
		"Input.Ability.Confirm",
		"Confirm an ability targeting session.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Ability_Cancel,
		"Input.Ability.Cancel",
		"Cancel an ability targeting session.");

} // namespace MyraGameplayTags
