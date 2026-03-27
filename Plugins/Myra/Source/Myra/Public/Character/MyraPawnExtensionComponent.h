// Copyright Myra . All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MyraPawnExtensionComponent.generated.h"

class UMyraAbilitySystemComponent;
class UMyraCharacterData;

/**
 * UMyraPawnExtensionComponent
 *
 * Inspired by ULyraPawnExtensionComponent from Lyra Starter Game.
 *
 * PROBLEM THIS SOLVES:
 *   In multiplayer, the Pawn and the PlayerState replicate independently.
 *   On a joining client, the Pawn might arrive before the PlayerState — meaning
 *   OnRep_PlayerState fires before InitAbilityActorInfo can be called because the
 *   PlayerState pointer is still null.
 *
 *   Similarly, ability sets may need to be granted before BeginPlay runs, but the
 *   ASC might not yet have its avatar set.
 *
 *   This component tracks which "readiness conditions" are met and calls
 *   CheckPawnReadyToInitialize once all of them are satisfied — regardless of order.
 *
 * HOW TO USE:
 *   1. Add UMyraPawnExtensionComponent to your Character Blueprint (or constructor).
 *   2. Assign a UMyraCharacterData asset to CharacterData.
 *   3. The component auto-initializes Myra when both the PlayerState ASC and
 *      the Pawn's avatar are ready.
 *   4. Bind to OnPawnReadyToInitialize to do any additional setup.
 *
 * SIMPLIFICATION vs LYRA:
 *   Lyra's version handles Game Feature plugins loading asynchronously.
 *   This version tracks two simpler conditions: PlayerState ready + Avatar set.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMyraOnPawnReadyToInitialize);

UCLASS(ClassGroup = "Myra", meta = (BlueprintSpawnableComponent),
	DisplayName = "Myra Pawn Extension Component")
class MYRA_API UMyraPawnExtensionComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UMyraPawnExtensionComponent();

	/** Convenience getter — finds this component on any actor. Returns null if not present. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Myra |Pawn",
		meta = (DefaultToSelf = "Actor"))
	static UMyraPawnExtensionComponent* FindPawnExtensionComponent(const AActor* Actor);

	// ------------------------------------------------
	//  Configuration
	// ------------------------------------------------

	/**
	 * The data asset describing this pawn's abilities, mesh, and initial stats.
	 * Assign this in the Blueprint defaults to fully configure the character without code.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Myra |Configuration")
	TObjectPtr<UMyraCharacterData> CharacterData;

	// ------------------------------------------------
	//  Initialization API
	//  Call these from your Character class as conditions become ready.
	// ------------------------------------------------

	/**
	 * Call this when the PlayerState is available (from PossessedBy on server,
	 * OnRep_PlayerState on client). Triggers readiness check.
	 */
	UFUNCTION(BlueprintCallable, Category = "Myra |Pawn")
	void HandlePlayerStateReplicated();

	/**
	 * Call this when the pawn's avatar actor has been set on the ASC.
	 * (Internally called by InitAbilityActorInfo once avatar is valid.)
	 */
	UFUNCTION(BlueprintCallable, Category = "Myra |Pawn")
	void HandleAvatarSet();

	/**
	 * Call this from your controller's OnPossess / AcknowledgePossession.
	 * Marks the controller as ready (needed for locally-controlled pawns).
	 */
	UFUNCTION(BlueprintCallable, Category = "Myra |Pawn")
	void HandleControllerChanged();

	/** Returns true when all readiness conditions have been met and Myra is initialized. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Myra |Pawn")
	bool IsReadyToInitialize() const { return bPawnReadyToInitialize; }

	/** Returns the ASC from CharacterData context (PlayerState or owned, depending on character setup). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Myra |Pawn")
	UMyraAbilitySystemComponent* GetMyraAbilitySystemComponent() const;

	// ------------------------------------------------
	//  Events
	// ------------------------------------------------

	/**
	 * Broadcasts once when all initialization conditions are met.
	 * Bind here to do any setup that requires both the ASC and the avatar to exist.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Myra |Pawn")
	FMyraOnPawnReadyToInitialize OnPawnReadyToInitialize;

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	/**
	 * Checks whether all conditions are met. If so, broadcasts OnPawnReadyToInitialize
	 * and applies the CharacterData (granting ability sets, applying init GE, etc.).
	 */
	void CheckPawnReadyToInitialize();

	/** Applies everything in CharacterData to the ASC (ability sets, init GE, etc.). */
	void ApplyCharacterData();

	bool bPlayerStateReady  = false;
	bool bAvatarReady       = false;
	bool bControllerReady   = false;
	bool bPawnReadyToInitialize = false;
};
