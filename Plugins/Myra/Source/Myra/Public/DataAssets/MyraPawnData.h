// Copyright Myra . All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MyraPawnData.generated.h"

class UInputMappingContext;
class UGameplayEffect;
class UMyraAbilitySet;
class UMyraInputConfig;

/**
 * Input mapping context entry used by Myra Pawn Data.
 */
USTRUCT(BlueprintType)
struct FMyraInputMappingContext
{
	GENERATED_BODY()

	/** Input mapping context to add for this pawn. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Myra |Input")
	TObjectPtr<UInputMappingContext> MappingContext = nullptr;

	/** Higher priority mapping contexts win over lower priority ones. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Myra |Input")
	int32 Priority = 0;
};

/**
 * Generic pawn configuration asset used by Myra.
 * This replaces the old character-specific data asset so the plugin can be used
 * for characters, vehicles, turrets, or any other controllable pawn type.
 */
UCLASS(BlueprintType, meta = (DisplayName = "Myra Pawn Data"))
class MYRA_API UMyraPawnData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	/** Ability sets granted to this pawn when Myra initializes it. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Myra |Pawn",
		meta = (TitleProperty = "{AssetName}"))
	TArray<TObjectPtr<UMyraAbilitySet>> AbilitySets;

	/**
	 * Gameplay Effect that initializes the pawn's attributes.
	 * Use only one attribute initialization path for a given pawn:
	 * PawnData, the Character's DefaultAttributeInitEffect, or an AbilitySet init effect.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Myra |Pawn")
	TSubclassOf<UGameplayEffect> DefaultAttributeInitEffect;

	/** Input config describing native and ability input tags for this pawn. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Myra |Input")
	TObjectPtr<UMyraInputConfig> InputConfig;

	/** Mapping contexts to add for the owning local player when this pawn becomes active. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Myra |Input",
		meta = (TitleProperty = "MappingContext"))
	TArray<FMyraInputMappingContext> DefaultInputMappings;

	// UPrimaryDataAsset
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};
