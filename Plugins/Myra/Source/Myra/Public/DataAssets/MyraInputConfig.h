// Copyright Myra . All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "MyraInputConfig.generated.h"

class UInputAction;

/**
 * Minimal input action mapping used by Myra.
 * One Enhanced Input action maps to one Gameplay Tag.
 */
USTRUCT(BlueprintType)
struct FMyraInputAction
{
	GENERATED_BODY()

	/** Enhanced Input action asset to bind. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Myra |Input")
	TObjectPtr<UInputAction> InputAction = nullptr;

	/** Gameplay Tag forwarded to native code or the Ability System. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Myra |Input",
		meta = (Categories = "Input"))
	FGameplayTag InputTag;
};

/**
 * Data asset containing native input mappings and ability input mappings.
 * This mirrors Lyra's split between native gameplay input and GAS-triggered input,
 * while keeping the asset small for beginners.
 */
UCLASS(BlueprintType, Const, meta = (DisplayName = "Myra Input Config"))
class MYRA_API UMyraInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:

	/** Input actions that should be handled by pawn or controller code. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Myra |Input",
		meta = (TitleProperty = "InputAction"))
	TArray<FMyraInputAction> NativeInputActions;

	/** Input actions that should forward their tag to the Ability System. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Myra |Input",
		meta = (TitleProperty = "InputAction"))
	TArray<FMyraInputAction> AbilityInputActions;

	/** Find a native input action by tag. */
	const UInputAction* FindNativeInputActionForTag(
		const FGameplayTag& InputTag,
		bool bLogIfNotFound = false) const;

	/** Find an ability input action by tag. */
	const UInputAction* FindAbilityInputActionForTag(
		const FGameplayTag& InputTag,
		bool bLogIfNotFound = false) const;

private:

	const UInputAction* FindInputActionForTag(
		const TArray<FMyraInputAction>& InputActions,
		const FGameplayTag& InputTag,
		bool bLogIfNotFound) const;
};
