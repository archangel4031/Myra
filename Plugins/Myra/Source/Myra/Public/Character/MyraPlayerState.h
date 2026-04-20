// Copyright Myra . All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "MyraPlayerState.generated.h"

class UMyraAbilitySystemComponent;
class UMyraAttributeSet;
class UMyraAbilitySet;

/**
 * AMyraPlayerState
 *
 * WHY PUT THE ASC ON PLAYER STATE?
 * In multiplayer games the Character is often destroyed on death and respawned.
 * The PlayerState persists across respawns. If your ASC lives on the Character,
 * all cooldown timers, stacks, and active effects are wiped on death. Hosting the
 * ASC on PlayerState keeps everything intact across respawns and pawn swaps.
 *
 * For single-player games you can ignore this and put the ASC on your Character
 * directly — AMyraCharacter handles both cases.
 *
 * SETUP:
 *   1. Set your Game Mode's PlayerStateClass to this class (or a subclass).
 *   2. Set your Character's bUsePlayerStateASC = true (default).
 *   3. Add your AbilitySets to DefaultAbilitySets on this PlayerState.
 */
UCLASS()
class MYRA_API AMyraPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:

	AMyraPlayerState();

	//~ Begin IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~ End IAbilitySystemInterface

	/** Returns the typed ASC. Faster than GetAbilitySystemComponent() + cast. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Myra ")
	UMyraAbilitySystemComponent* GetMyraAbilitySystemComponent() const { return AbilitySystemComponent; }

	/** Convenience access to the base attribute set. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Myra ")
	UMyraAttributeSet* GetBaseAttributeSet() const { return AttributeSet; }

	/** Returns current health from the base attribute set. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Myra |Attributes")
	float GetHealth() const;

	/** Returns current max health from the base attribute set. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Myra |Attributes")
	float GetMaxHealth() const;

	/** Normalized health [0..1]. Perfect for health bar UI. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Myra |Attributes")
	float GetHealthPercent() const;

	// ------------------------------------------------
	//  Configuration (set these in your subclass defaults)
	// ------------------------------------------------

	/**
	 * AbilitySets to grant automatically when this PlayerState is initialized.
	 * Add your UMyraAbilitySet assets here to grant abilities, attribute sets,
	 * and startup effects all at once.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Myra |Configuration")
	TArray<TObjectPtr<UMyraAbilitySet>> DefaultAbilitySets;

protected:

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/**
	 * Called once the ASC's avatar actor is set (i.e. when the Character is possessed
	 * or after the PlayerState replicates to clients). Override to do post-init work.
	 */
	virtual void OnAbilityActorInfoSet();

	// ------------------------------------------------
	//  Components
	// ------------------------------------------------

	/** The Ability System Component. Lives here so it persists across respawns. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Myra |Components",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMyraAbilitySystemComponent> AbilitySystemComponent;

	/**
	 * The base attribute set (Health).
	 * Constructed automatically. Add project-specific attributes
	 * through your own attribute sets via UMyraAbilitySet.
	 */
	UPROPERTY()
	TObjectPtr<UMyraAttributeSet> AttributeSet;

//CLAUDE: 
//private:
public:

	void GrantDefaultAbilitySets();
};
