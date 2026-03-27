// Copyright Myra . All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "GameplayTagContainer.h"
#include "MyraInputComponent.generated.h"

class UMyraAbilitySystemComponent;

/**
 * UMyraInputComponent
 *
 * Minimal subclass of UEnhancedInputComponent.
 * Input-to-ability binding is handled in Blueprints via the
 * TryActivateAbilitiesByTag node on the ASC.
 *
 * This class exists so you can set it as the DefaultInputComponentClass
 * in Project Settings and extend it in Blueprints if needed.
 */
UCLASS(ClassGroup = "Myra", meta = (BlueprintSpawnableComponent))
class MYRA_API UMyraInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:

	/**
	 * Tries to activate all abilities on the ASC that match the given tag.
	 * Call this from your Blueprint input event (On Input Action → Started).
	 */
	UFUNCTION(BlueprintCallable, Category = "Myra |Input")
	static void ActivateAbilityByTag(UMyraAbilitySystemComponent* ASC, FGameplayTag InputTag);

	/**
	 * Signals input-released to any active abilities matching the tag.
	 * Call this from your Blueprint input event (On Input Action → Completed).
	 */
	UFUNCTION(BlueprintCallable, Category = "Myra |Input")
	static void ReleaseAbilityByTag(UMyraAbilitySystemComponent* ASC, FGameplayTag InputTag);
};
