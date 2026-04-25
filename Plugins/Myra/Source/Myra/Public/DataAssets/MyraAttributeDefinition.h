// Copyright Myra . All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MyraAttributeDefinition.generated.h"

/**
 * Defines a single attribute for use in the Myra  editor wizard.
 * The wizard reads these structs and generates a UMyraBaseAttributeSet subclass.
 */
USTRUCT(BlueprintType)
struct FMyraAttributeDefinitionEntry
{
	GENERATED_BODY()

	/** The attribute name. Must be alphanumeric, no spaces. E.g. "Strength", "AttackSpeed". */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
	FName AttributeName = NAME_None;

	/** Default value before any Gameplay Effects are applied. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
	float DefaultValue = 0.f;

	/**
	 * Whether to auto-generate a paired Max attribute (e.g. MaxStrength).
	 * MaxStrength will be used to clamp Strength in PreAttributeChange.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
	bool bCreateMaxAttribute = false;

	/** Default value for the auto-generated Max attribute (if bCreateMaxAttribute is true). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute",
		meta = (EditCondition = "bCreateMaxAttribute"))
	float MaxDefaultValue = 100.f;

	/**
	 * Whether this attribute should replicate to clients.
	 * Set false for server-only simulation attributes that don't need UI feedback.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
	bool bReplicated = true;

	/**
	 * Whether this is a "Meta" attribute — transient, never replicated, zeroed after
	 * PostGameplayEffectExecute (e.g. Damage, Healing).
	 * When true, bReplicated and bCreateMaxAttribute are both forced to false.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
	bool bIsMetaAttribute = false;

	/** Editor tooltip shown in the Details panel for this attribute. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
	FText Tooltip;
};

/**
 * UMyraAttributeDefinition
 *
 * A data asset that describes a group of attributes for a single AttributeSet class.
 * The Myra  Editor Wizard reads this asset and generates the corresponding
 * C++ AttributeSet header and source files.
 *
 * HOW TO USE:
 *   1. Right-click in Content Browser → Myra  → Attribute Definition.
 *   2. Name your AttributeSet class (e.g. "Combat" generates "UMyraCombatAttributeSet").
 *   3. Add attribute entries to the Attributes array.
 *   4. Open Myra  Wizard (Tools menu) and select this asset to generate the class.
 *      The wizard writes headers to Public/AbilitySystem/CustomAttributeSets
 *      and sources to Private/AbilitySystem/CustomAttributeSets in the Myra plugin.
 *
 * IMPORTANT:
 *   The wizard generates a NEW .h/.cpp file. It never modifies existing files.
 *   After generation, compile the project to use the new AttributeSet.
 */
UCLASS(BlueprintType, meta = (DisplayName = "Myra Attribute Definition"))
class MYRA_API UMyraAttributeDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	/**
	 * Name of the generated class (without prefix or "AttributeSet" suffix).
	 * E.g. "Combat" → generates UMyraCombatAttributeSet.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Myra |Definition")
	FName AttributeSetName = "Custom";

	/** All attributes to include in this AttributeSet. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Myra |Definition",
		meta = (TitleProperty = "AttributeName"))
	TArray<FMyraAttributeDefinitionEntry> Attributes;

	/**
	 * If true, the wizard includes standard PostGameplayEffectExecute logic
	 * that handles Damage and Healing meta attributes (if present in Attributes array).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Myra |Definition")
	bool bGenerateDamageHealingLogic = true;

	/**
	 * Extra includes to add to the generated header.
	 * Use this if your attributes depend on project-specific classes.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Myra |Definition|Advanced")
	TArray<FString> ExtraIncludes;

	// UPrimaryDataAsset
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

#if WITH_EDITOR
	/**
	 * Validates attribute names and flags issues in the editor.
	 * Called automatically by the editor when the asset is saved.
	 */
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};
