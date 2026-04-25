// Copyright Myra . All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "MyraBaseAttributeSet.generated.h"

/**
 * Convenience macro that generates the standard boilerplate for each Myra attribute:
 *   - UPROPERTY declaration
 *   - Getter (GetHealth)
 *   - Setter (SetHealth)
 *   - Initializer (InitHealth)
 *   - Static attribute accessor (GetHealthAttribute)
 */
#ifndef ATTRIBUTE_ACCESSORS
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)
#endif

/**
 * Shared Myra attribute-set behavior with no gameplay attributes of its own.
 *
 * Subclass this for both the built-in default set and generated project sets so
 * future shared changes do not need to be copied into every generated file.
 */
UCLASS(Abstract)
class MYRA_API UMyraBaseAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:

	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

protected:

	void BroadcastAttributeChanged(const FGameplayAttribute& Attribute, float OldValue, float NewValue) const;
	void BroadcastGameplayEffectExecuted(const FGameplayEffectModCallbackData& Data) const;
};
