// Copyright Myra . All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "AbilitySystemComponent.h"    // EGameplayEffectReplicationMode is defined here
#include "GameplayTagContainer.h"      // FGameplayTag
#include "MyraDeveloperSettings.generated.h"

/**
 * UMyraDeveloperSettings
 *
 * Plugin-wide settings that appear under Edit → Project Settings → Myra .
 * Config=Game means they're stored in DefaultGame.ini.
 *
 * Beginners can configure the most important Myra options here without touching C++.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Myra "))
class MYRA_API UMyraDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	UMyraDeveloperSettings();

	/** Returns the singleton instance (shortcut for GetDefault<UMyraDeveloperSettings>()). */
	static const UMyraDeveloperSettings* Get()
	{
		return GetDefault<UMyraDeveloperSettings>();
	}

	// ------------------------------------------------
	//  Replication
	// ------------------------------------------------

	/**
	 * Default ASC replication mode for new characters.
	 *   Full    — All gameplay effect info replicated to every client. Use for single player.
	 *   Mixed   — GEs replicated to owner only; tags/predictions replicated to all. Best for most multiplayer games.
	 *   Minimal — Only tags replicated. Use for AI or characters where clients don't need GE details.
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Replication")
	EGameplayEffectReplicationMode DefaultReplicationMode = EGameplayEffectReplicationMode::Mixed;

	// ------------------------------------------------
	//  PlayerState vs Character ASC
	// ------------------------------------------------

	/**
	 * When true, new AMyraCharacter subclasses default to using the PlayerState ASC.
	 * Recommended for multiplayer. Disable for single-player or AI-only projects.
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Architecture")
	bool bDefaultToPlayerStateASC = true;

	// ------------------------------------------------
	//  Attribute Initialization
	// ------------------------------------------------

	/**
	 * Default Gameplay Effect class used to initialize attributes.
	 * All new characters will reference this unless they override it with their own.
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Attributes",
		meta = (AllowedClasses = "/Script/GameplayAbilities.GameplayEffect"))
	FSoftClassPath DefaultAttributeInitEffect;

	// ------------------------------------------------
	//  Gameplay Tags
	// ------------------------------------------------

	/** Gameplay tag applied when a character is dead. Used to block abilities. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Tags")
	FGameplayTag DeadTag;

	/** Gameplay tag applied while an ability is actively executing. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Tags")
	FGameplayTag AbilityActivatingTag;

	// UDeveloperSettings interface
	virtual FName GetCategoryName() const override { return TEXT("Plugins"); }
	virtual FName GetSectionName() const override  { return TEXT("Myra "); }
};
