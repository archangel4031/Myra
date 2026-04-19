// Copyright Myra . All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagAssetInterface.h"
#include "GameplayEffectTypes.h"
#include "MyraCharacter.generated.h"

class UMyraAbilitySystemComponent;
class UMyraAttributeSet;
class UMyraAbilitySet;
class UGameplayEffect;
class UInputComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMyraOnDeath, AMyraCharacter*, DeadCharacter, AActor*, Killer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FMyraOnHealthChanged, AMyraCharacter*, Character, float, OldValue, float, NewValue);

/**
 * AMyraCharacter
 *
 * Base character class for Myra . Supports two initialization patterns:
 *
 *  A) MULTIPLAYER (default): ASC lives on PlayerState.
 *     The Character only holds a cached pointer to the PlayerState's ASC.
 *     Set bUsePlayerStateASC = true (default).
 *
 *  B) SINGLE-PLAYER / AI: ASC lives on the Character.
 *     Set bUsePlayerStateASC = false on your subclass defaults.
 *     The Character owns and initializes its own ASC.
 *
 * SETUP CHECKLIST:
 *   [ ] Subclass AMyraCharacter
 *   [ ] Set your PlayerStateClass to AMyraPlayerState (if multiplayer)
 *   [ ] Add UMyraAbilitySet assets to DefaultAbilitySets (if single-player / AI)
 *   [ ] Add your GE_Init GameplayEffect to initialize attribute values
 */
UCLASS(Abstract)
class MYRA_API AMyraCharacter : public ACharacter,
	public IAbilitySystemInterface,
	public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:

	AMyraCharacter();

	//~ Begin IAbilitySystemInterface
	/**
	 * Returns the Ability System Component associated with this character.
	 * This is the standard implementation from IAbilitySystemInterface.
	 * Most engine and GAS systems will call this function internally.
	 *
	 * @return The base UAbilitySystemComponent (may be nullptr if not initialized).
	 */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~ End IAbilitySystemInterface

	//~ Begin IGameplayTagAssetInterface
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	//~ End IGameplayTagAssetInterface

	// ------------------------------------------------
	//  Accessors
	// ------------------------------------------------

	/**
	 * Returns the custom Myra Ability System Component for this character.
	 *
	 * This is a convenience function specific to the Myra plugin.
	 * It returns the typed UMyraAbilitySystemComponent instead of the base class,
	 * so you can directly access Myra-specific features without casting.
	 *
	 * Use this in your game code and Blueprints when working within the Myra framework.
	 * Always check for nullptr in multiplayer or before the character is fully initialized.
	 *
	 * @return The UMyraAbilitySystemComponent owned by this character (or nullptr if none).
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Myra ")
	UMyraAbilitySystemComponent* GetMyraAbilitySystemComponent() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Myra ")
	const UMyraAttributeSet* GetBaseAttributeSet() const;

	// ------------------------------------------------
	//  Attribute Helpers (Blueprint-friendly)
	// ------------------------------------------------

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Myra |Attributes")
	float GetHealth() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Myra |Attributes")
	float GetMaxHealth() const;

	/** Normalized health [0..1]. Perfect for health bar UI. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Myra |Attributes")
	float GetHealthPercent() const;

	// ------------------------------------------------
	//  State Queries
	// ------------------------------------------------

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Myra |State")
	bool IsAlive() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Myra |State")
	bool IsDead() const { return !IsAlive(); }

	// ------------------------------------------------
	//  Events
	// ------------------------------------------------

	/** Fires on server and owning client when Health reaches zero. */
	UPROPERTY(BlueprintAssignable, Category = "Myra |Events")
	FMyraOnDeath OnDeathEvent;

	/** Fires whenever Health changes (damage or healing). Useful for UI. */
	UPROPERTY(BlueprintAssignable, Category = "Myra |Events")
	FMyraOnHealthChanged OnHealthChanged;

	// ------------------------------------------------
	//  Configuration
	// ------------------------------------------------

	/**
	 * If true (default), the ASC is taken from the PlayerState.
	 * Set false to skip PlayerState logic and keep the ASC directly on this Character.
	 * If true but no compatible Myra PlayerState is available, Myra falls back to the Character ASC.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Myra |Configuration")
	bool bUsePlayerStateASC = true;

	/**
	 * AbilitySets to grant when this character initializes its own ASC
	 * (i.e. when bUsePlayerStateASC is false). Ignored when using PlayerState ASC.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Myra |Configuration",
		meta = (EditCondition = "!bUsePlayerStateASC"))
	TArray<TObjectPtr<UMyraAbilitySet>> DefaultAbilitySets;

	/**
	 * Gameplay Effect applied on init to set starting attribute values when this Character
	 * owns the active ASC.
	 * Create a Blueprint GE of type "Instant" with modifiers that Override each attribute.
	 * Use only one attribute initialization path for a given character:
	 * this property, PawnData, or an AbilitySet entry marked as initialization.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Myra |Configuration")
	TSubclassOf<UGameplayEffect> DefaultAttributeInitEffect;

protected:

	//~ Begin AActor / ACharacter Interface
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;       // Server
	virtual void UnPossessed() override;                                 // Server
	virtual void OnRep_PlayerState() override;                           // Client
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	//~ End AActor / ACharacter Interface

	/**
	 * Called on both server and client once the ASC avatar is fully set up.
	 * Override in subclasses to bind to attribute change delegates, etc.
	 */
	virtual void OnAbilitySystemInitialized();

	/** Called when Health reaches 0. Override to play death animations, ragdoll, etc. */
	UFUNCTION(BlueprintNativeEvent, Category = "Myra |Events")
	void OnDeath(AActor* Killer);
	virtual void OnDeath_Implementation(AActor* Killer);

private:

	// The currently active ASC for this pawn. Exposed so Character Blueprints can always reach it.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Myra |Components",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMyraAbilitySystemComponent> ResolvedAbilitySystemComponent;

	// Owned ASC — created up front for beginner-friendly BP access, but only used as the active ASC
	// when bUsePlayerStateASC is false or when Myra falls back to Character-owned initialization.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Myra |Components",
		meta = (AllowPrivateAccess = "true", EditCondition = "!bUsePlayerStateASC"))
	TObjectPtr<UMyraAbilitySystemComponent> OwnedAbilitySystemComponent;

	// Owned base attribute set paired with the Character-owned ASC.
	UPROPERTY()
	TObjectPtr<UMyraAttributeSet> OwnedAttributeSet;

	bool InitAbilitySystemForPlayerState();
	void InitAbilitySystemOwned();
	void HandlePlayerStateAbilitySystemRemoved();
	void BindAttributeChangeCallbacks();
	void ApplyDefaultAttributeInitEffect();
	bool IsUsingPlayerStateAbilitySystem() const;

	void HandleHealthChanged(const FOnAttributeChangeData& ChangeData);
	void HandleDeathTag(const FGameplayTag GameplayTag, int32 NewCount);

	bool bAbilitySystemInitialized = false;
};
