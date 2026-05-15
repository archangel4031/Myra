// Copyright Myra . All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "MyraGameplayAbility.generated.h"

class APawn;
class UMyraAbilitySystemComponent;
class UMyraPawnAbilityComponent;

/**
 * Enumerates when this ability should be activated automatically.
 */
UENUM(BlueprintType)
enum class EMyraAbilityActivationPolicy : uint8
{
	/** Activated manually by input or code. Most common. */
	OnInputTriggered UMETA(DisplayName = "On Input Triggered"),

	/** Activated once when granted to the ASC (passive abilities, buffs). */
	OnGranted        UMETA(DisplayName = "On Granted"),

	/** Activated every time the owning actor spawns. */
	OnSpawn          UMETA(DisplayName = "On Spawn"),
};

/**
 * UMyraGameplayAbility
 *
 * Base gameplay ability for Myra . Inherit from this instead of
 * UGameplayAbility. Provides:
 *
 *  - Easy access to the owning pawn avatar and ASC without casting every time.
 *  - An ActivationPolicy enum so designers can make passive abilities without code.
 *  - Blueprint-friendly cost/cooldown query functions.
 *  - A clean CanActivateAbility override hook.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class MYRA_API UMyraGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:

	UMyraGameplayAbility();

	// ------------------------------------------------
	//  Configuration
	// ------------------------------------------------

	/**
	 * When should this ability activate?
	 *   OnInputTriggered — player presses a button (default).
	 *   OnGranted        — activates immediately when added to the ASC (passive).
	 *   OnSpawn          — activates when the pawn avatar spawns.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Myra |Ability")
	EMyraAbilityActivationPolicy ActivationPolicy = EMyraAbilityActivationPolicy::OnInputTriggered;

	/**
	 * Gameplay tags that block this ability when present on the owner.
	 * Populated automatically by looking at BlockAbilitiesWithTag — this is just
	 * a designer-friendly mirror for documentation purposes.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Myra |Ability",
		meta = (Categories = "BlockAbility"))
	FGameplayTagContainer AdditionalBlockTags;

	// ------------------------------------------------
	//  Overrides
	// ------------------------------------------------

	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilitySpec& Spec) override;

	// ------------------------------------------------
	//  Typed Accessors (avoid casting in every Blueprint)
	// ------------------------------------------------

	/** Returns the owning pawn avatar. Can be null. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Myra |Ability")
	APawn* GetMyraPawn() const;

	/** Returns the owning ASC. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Myra |Ability")
	UMyraAbilitySystemComponent* GetMyraAbilitySystemComponent() const;

	/** Returns the shared Myra pawn ability component for the current avatar. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Myra |Ability")
	UMyraPawnAbilityComponent* GetMyraPawnAbilityComponent() const;

	/** Returns true if this ability asset tags container includes the provided gameplay tag. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Myra |Ability")
	bool HasAbilityTag(FGameplayTag AbilityTag) const;

	// ------------------------------------------------
	//  Cost & Cooldown Info (for UI)
	// ------------------------------------------------

		/**
	 * Returns the flat cost magnitude for this ability at the current level.
	 * Reads the first non-zero modifier on the CostGameplayEffect.
	 * Returns 0 if no cost effect is set.
	 *
	 * NOTE: Named GetAbilityCostAmount (not GetCostAmount) to avoid shadowing
	 * unrelated parent-class members and to be explicit about scope.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Myra|Ability|Info")
	float GetAbilityCostAmount() const;

	/**
	 * Returns the total cooldown duration for this ability at the current level.
	 * Reads the DurationMagnitude on the CooldownGameplayEffect.
	 * Returns 0 if no cooldown is set.
	 *
	 * NOTE: Named GetAbilityCooldownDuration — UGameplayAbility already has a
	 * GetCooldownGameplayEffect(); this is a convenience wrapper for the float value.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Myra|Ability|Info")
	float GetAbilityCooldownDuration() const;

	/**
	 * Returns how many seconds remain on the active cooldown.
	 * Returns 0 if the ability is not currently on cooldown.
	 *
	 * NOTE: Renamed from GetCooldownTimeRemaining — the parent UGameplayAbility
	 * declares that exact name as a UFUNCTION, so UHT forbids redeclaring it.
	 * This version queries the live ASC rather than the CDO.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Myra|Ability|Info")
	float GetAbilityCooldownTimeRemaining() const;

protected:

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
};
