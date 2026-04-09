// Copyright Myra . All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "MyraAttributeSet.generated.h"

/**
 * Convenience macro that generates the standard boilerplate for each Myra attribute:
 *   - UPROPERTY declaration
 *   - Getter (GetHealth)
 *   - Setter (SetHealth)
 *   - Initializer (InitHealth)
 *   - Static attribute accessor (GetHealthAttribute)
 *
 * This is the same macro Epic uses in their own attribute sets.
 */
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * UMyraAttributeSet
 *
 * The base attribute set for Myra. It only defines health by default.
 * Add project-specific attributes in your own UAttributeSet subclasses.
 *
 * HOW ATTRIBUTES WORK:
 *   - Each attribute has a "base" value and a "current" value.
 *   - Max attributes (MaxHealth etc.) act as clamp targets in PostGameplayEffectExecute.
 *   - All changes go through Gameplay Effects — never modify attributes directly.
 */
UCLASS()
class MYRA_API UMyraAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:

	UMyraAttributeSet();

	//~ Begin UAttributeSet Interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	/**
	 * Called after ANY attribute value changes on this set (base or current).
	 * We use this to broadcast the ASC's OnAttributeChanged delegate so UI can
	 * react without needing per-attribute delegate subscriptions.
	 * PostAttributeChange is correctly a virtual on UAttributeSet, not on UAbilitySystemComponent.
	 */
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	//~ End UAttributeSet Interface

	// ------------------------------------------------
	//  Health
	// ------------------------------------------------

	/** Current Health. Reaches 0 when the character dies. */
	UPROPERTY(BlueprintReadOnly, Category = "Myra |Attributes|Health",
		ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UMyraAttributeSet, Health)

	/** Maximum possible Health. Clamped to this in PostGameplayEffectExecute. */
	UPROPERTY(BlueprintReadOnly, Category = "Myra |Attributes|Health",
		ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UMyraAttributeSet, MaxHealth)

	// ------------------------------------------------
	//  Damage / Healing (Meta Attributes)
	// ------------------------------------------------

	/**
	 * Damage is a "meta" attribute — it is never replicated and only exists transiently
	 * during effect execution. An incoming hit applies Damage as an Instant GE, and
	 * PostGameplayEffectExecute converts it into a Health reduction.
	 *
	 * This pattern lets you intercept, modify, or block damage in one clean place.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Myra |Attributes|Meta")
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(UMyraAttributeSet, Damage)

	/** Same as Damage but adds to Health instead of subtracting. */
	UPROPERTY(BlueprintReadOnly, Category = "Myra |Attributes|Meta")
	FGameplayAttributeData Healing;
	ATTRIBUTE_ACCESSORS(UMyraAttributeSet, Healing)

protected:

	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);

	/**
	 * Clamp a current attribute to [0, MaxAttribute].
	 * Called from PreAttributeChange.
	 */
	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;
};
