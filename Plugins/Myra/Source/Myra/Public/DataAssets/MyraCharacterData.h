// Copyright Myra . All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MyraCharacterData.generated.h"

class UMyraAbilitySet;
class UGameplayEffect;
class UAnimInstance;
class USkeletalMesh;

/**
 * UMyraCharacterData
 *
 * Inspired by Lyra's ULyraPawnData.
 *
 * A data asset that fully describes one type of character — their visual mesh,
 * their abilities, and their default attribute values — all in one place.
 *
 * WHY?
 *   Without this, character configuration is scattered: mesh on the Blueprint,
 *   abilities on the PlayerState, init effects wired in BeginPlay. With UMyraCharacterData,
 *   a designer changes one asset to entirely swap out a character archetype.
 *
 * HOW TO USE:
 *   1. Right-click → Myra  → Character Data.
 *   2. Fill in mesh, ability sets, and init effect.
 *   3. On your AMyraCharacter subclass, set CharacterData to this asset.
 *   4. The character's InitCharacterData() function applies everything automatically.
 */
UCLASS(BlueprintType, meta = (DisplayName = "Myra Character Data"))
class MYRA_API UMyraCharacterData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	// ------------------------------------------------
	//  Visuals
	// ------------------------------------------------

	/** Default skeletal mesh for this character type. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Myra |Character|Visuals")
	TSoftObjectPtr<USkeletalMesh> CharacterMesh;

	/** Animation Blueprint class to use. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Myra |Character|Visuals")
	TSoftClassPtr<UAnimInstance> AnimationBlueprint;

	// ------------------------------------------------
	//  Abilities & Attributes
	// ------------------------------------------------

	/**
	 * Ability sets to grant to this character type.
	 * All abilities, attribute sets, and startup effects in these sets will be applied.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Myra |Character|Abilities",
		meta = (TitleProperty = "{AssetName}"))
	TArray<TObjectPtr<UMyraAbilitySet>> AbilitySets;

	/**
	 * Gameplay Effect that sets the starting attribute values for this character type.
	 * Create a Blueprint GE with Duration = Instant and Modifiers that Override the
	 * attributes defined by your base or custom attribute sets.
	 * Example: Override Health = 200, Override MaxHealth = 200.
	 * Use only one attribute initialization path for a given character:
	 * CharacterData, the Character's DefaultAttributeInitEffect, or an AbilitySet init effect.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Myra |Character|Abilities")
	TSubclassOf<UGameplayEffect> DefaultAttributeInitEffect;

	// ------------------------------------------------
	//  Movement (optional — fill in if using Modular Movement)
	// ------------------------------------------------

	/** Base walk speed for this character type. 0 = use Character defaults. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Myra |Character|Movement",
		meta = (ClampMin = "0"))
	float WalkSpeed = 0.f;

	/** Sprint speed. 0 = use Character defaults. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Myra |Character|Movement",
		meta = (ClampMin = "0"))
	float SprintSpeed = 0.f;

	// UPrimaryDataAsset
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};
