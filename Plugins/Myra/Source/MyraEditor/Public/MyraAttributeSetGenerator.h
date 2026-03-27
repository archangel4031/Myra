// Copyright Myra . All Rights Reserved.
#pragma once

#include "CoreMinimal.h"

class UMyraAttributeDefinition;

/**
 * FMyraGenerationResult
 * Returned by the generator to report what happened.
 */
struct FMyraGenerationResult
{
	bool bSuccess = false;

	/** Absolute path to the generated .h file (empty if failed). */
	FString GeneratedHeaderPath;

	/** Absolute path to the generated .cpp file (empty if failed). */
	FString GeneratedSourcePath;

	/** Human-readable messages (errors, warnings, info). */
	TArray<FString> Messages;

	void AddMessage(const FString& Msg) { Messages.Add(Msg); }
	void AddError  (const FString& Msg) { Messages.Add(TEXT("[ERROR] ") + Msg); }
};

/**
 * FMyraAttributeSetGenerator
 *
 * Takes a UMyraAttributeDefinition data asset and produces a pair of
 * .h / .cpp files for a new UAttributeSet subclass.
 *
 * KEY DESIGN RULE: This generator ONLY creates NEW files.
 *   It will refuse to overwrite an existing file unless bOverwrite is true.
 *   It NEVER modifies any existing source files.
 *
 * OUTPUT EXAMPLE (from a definition named "Combat" with attributes Strength, Armor):
 *
 *   MyraCombatAttributeSet.h
 *   ├─ ATTRIBUTE_ACCESSORS for Strength, MaxStrength, Armor, MaxArmor
 *   ├─ UPROPERTY + ReplicatedUsing for each (unless meta)
 *   └─ OnRep declarations
 *
 *   MyraCombatAttributeSet.cpp
 *   ├─ GetLifetimeReplicatedProps (DOREPLIFETIME_CONDITION_NOTIFY for each)
 *   ├─ PreAttributeChange with clamp logic
 *   ├─ PostGameplayEffectExecute with meta attribute handling
 *   └─ OnRep implementations with GAMEPLAYATTRIBUTE_REPNOTIFY
 */
class MYRAEDITOR_API FMyraAttributeSetGenerator
{
public:

	/**
	 * Generate the .h and .cpp files for the given attribute definition.
	 *
	 * Generated headers are written to:
	 *   Plugins/Myra/Source/Myra/Public/AbilitySystem/CustomAttributeSets
	 *
	 * Generated sources are written to:
	 *   Plugins/Myra/Source/Myra/Private/AbilitySystem/CustomAttributeSets
	 *
	 * @param Definition The asset describing attributes to generate.
	 * @param bOverwrite If true, existing files with the same name are overwritten.
	 *                   Defaults to false (safe mode).
	 * @return           Result struct with file paths and error/warning messages.
	 */
	static FMyraGenerationResult GenerateAttributeSet(
		const UMyraAttributeDefinition* Definition,
		bool bOverwrite = false);

private:

	/** Resolves the fixed public/private output directories inside the Myra plugin module. */
	static bool ResolveOutputDirectories(
		FString& PublicOutputDirectory,
		FString& PrivateOutputDirectory,
		FMyraGenerationResult& Result);

	/** Generates the full content of the .h file as a string. */
	static FString BuildHeaderContent(const UMyraAttributeDefinition* Definition,
		const FString& ClassName);

	/** Generates the full content of the .cpp file as a string. */
	static FString BuildSourceContent(const UMyraAttributeDefinition* Definition,
		const FString& ClassName, const FString& HeaderRelPath);

	/** Writes text to a file, creating directories as needed. */
	static bool WriteFile(const FString& FilePath, const FString& Content,
		bool bOverwrite, FMyraGenerationResult& Result);

	/** Validates a definition before generating. Adds errors to Result if invalid. */
	static bool ValidateDefinition(const UMyraAttributeDefinition* Definition,
		FMyraGenerationResult& Result);
};
