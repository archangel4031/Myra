// Copyright Myra . All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/MyraBaseAttributeSet.h"
#include "MyraDefaultAttributeSet.generated.h"

/**
 * Built-in default attribute set for Myra.
 *
 * This owns the starter health and damage-related attributes that should exist
 * on every Myra ASC by default.
 */
UCLASS()
class MYRA_API UMyraDefaultAttributeSet : public UMyraBaseAttributeSet
{
	GENERATED_BODY()

public:

	UMyraDefaultAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	/** Current Health. Reaches 0 when the character dies. */
	UPROPERTY(BlueprintReadOnly, Category = "Myra |Attributes|Health",
		ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UMyraDefaultAttributeSet, Health)

	/** Maximum possible Health. Clamped to this in PostGameplayEffectExecute. */
	UPROPERTY(BlueprintReadOnly, Category = "Myra |Attributes|Health",
		ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UMyraDefaultAttributeSet, MaxHealth)

	/** Damage is a transient meta attribute converted into health loss. */
	UPROPERTY(BlueprintReadOnly, Category = "Myra |Attributes|Meta")
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(UMyraDefaultAttributeSet, Damage)

	/** Healing is a transient meta attribute converted into health gain. */
	UPROPERTY(BlueprintReadOnly, Category = "Myra |Attributes|Meta")
	FGameplayAttributeData Healing;
	ATTRIBUTE_ACCESSORS(UMyraDefaultAttributeSet, Healing)

protected:

	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);

	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;
};
