// Copyright Myra . All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "DataAssets/MyraInputConfig.h"
#include "EnhancedInputComponent.h"
#include "GameplayTagContainer.h"
#include "MyraInputComponent.generated.h"

class UMyraAbilitySystemComponent;
class UInputAction;

/**
 * UMyraInputComponent
 *
 * Lightweight helper around UEnhancedInputComponent for Myra.
 * It can bind ability actions from a UMyraInputConfig automatically, and it
 * also exposes a small helper for binding native input tags to pawn code.
 *
 * Projects can use this as their default input component class, but the static
 * helpers also work with a regular UEnhancedInputComponent to keep setup simple.
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

	/**
	 * Binds all ability input actions from the config to the supplied ASC.
	 * Each action forwards its tag on Started / Completed / Canceled.
	 */
	static void BindAbilityActions(
		UEnhancedInputComponent* InputComponent,
		const UMyraInputConfig* InputConfig,
		UMyraAbilitySystemComponent* ASC,
		TArray<uint32>& OutBindHandles);

	template <class UserClass, typename FuncType>
	static bool BindNativeAction(
		UEnhancedInputComponent* InputComponent,
		const UMyraInputConfig* InputConfig,
		const FGameplayTag& InputTag,
		ETriggerEvent TriggerEvent,
		UserClass* Object,
		FuncType Func,
		bool bLogIfNotFound = true);
};

template <class UserClass, typename FuncType>
bool UMyraInputComponent::BindNativeAction(
	UEnhancedInputComponent* InputComponent,
	const UMyraInputConfig* InputConfig,
	const FGameplayTag& InputTag,
	ETriggerEvent TriggerEvent,
	UserClass* Object,
	FuncType Func,
	bool bLogIfNotFound)
{
	if (!InputComponent || !InputConfig || !Object || !InputTag.IsValid())
	{
		return false;
	}

	if (const UInputAction* InputAction = InputConfig->FindNativeInputActionForTag(InputTag, bLogIfNotFound))
	{
		InputComponent->BindAction(InputAction, TriggerEvent, Object, Func);
		return true;
	}

	return false;
}
