// Copyright Myra . All Rights Reserved.

#include "AbilitySystem/MyraAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Tags/MyraNativeGameplayTags.h"
#include "AbilitySystem/MyraAbilitySystemComponent.h"

UMyraAttributeSet::UMyraAttributeSet()
{
	// Default attribute values — these are overridden by startup Gameplay Effects
	// (see UMyraAbilitySet and GE_Init* blueprints).
	InitHealth(100.f);
	InitMaxHealth(100.f);
	InitDamage(0.f);
	InitHealing(0.f);
}

void UMyraAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// REPNOTIFY_Always ensures OnRep fires even when the value doesn't change
	// (e.g. clamped at max). Required so prediction corrections work correctly.
	DOREPLIFETIME_CONDITION_NOTIFY(UMyraAttributeSet, Health,     COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMyraAttributeSet, MaxHealth,  COND_None, REPNOTIFY_Always);
	// Damage and Healing are meta attributes — intentionally NOT replicated.
}

// ------------------------------------------------
//  PreAttributeChange — called BEFORE the attribute value changes.
//  Use this to CLAMP values coming in. Do NOT trigger game logic here.
// ------------------------------------------------
void UMyraAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	ClampAttribute(Attribute, NewValue);
}

// ------------------------------------------------
//  PostGameplayEffectExecute — called AFTER an instant GE has been applied.
//  Use this to respond to changes (death, empty mana, etc.)
//  and to handle meta attributes like Damage and Healing.
// ------------------------------------------------
void UMyraAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	// --- Handle incoming Damage ---
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		// Grab and zero out the transient Damage value
		const float LocalDamage = GetDamage();
		SetDamage(0.f);

		if (LocalDamage > 0.f)
		{
			const float NewHealth = FMath::Clamp(GetHealth() - LocalDamage, 0.f, GetMaxHealth());
			SetHealth(NewHealth);

			// TODO: If NewHealth == 0, notify the character to begin dying.
			// We broadcast a tag event so anything (C++ or Blueprint) can listen:
			if (NewHealth <= 0.f)
			{
				FGameplayEventData EventData;
				EventData.EventMagnitude = LocalDamage;
				UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
					ASC->GetAvatarActor(),
					MyraGameplayTags::Myra_GameEvent_Death,
					EventData);
			}
		}
	}

	// --- Handle incoming Healing ---
	if (Data.EvaluatedData.Attribute == GetHealingAttribute())
	{
		const float LocalHealing = GetHealing();
		SetHealing(0.f);

		if (LocalHealing > 0.f)
		{
			const float NewHealth = FMath::Clamp(GetHealth() + LocalHealing, 0.f, GetMaxHealth());
			SetHealth(NewHealth);
		}
	}

	// Final clamp for Health after any change.
	SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
}

// ------------------------------------------------
//  OnRep functions — required for client-side prediction corrections
// ------------------------------------------------

void UMyraAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMyraAttributeSet, Health, OldValue);
}

void UMyraAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMyraAttributeSet, MaxHealth, OldValue);
}

// ------------------------------------------------
//  PostAttributeChange
//  Called after every attribute value change on this set.
//  We cast the owning ASC to UGASAbilitySystemComponent and call
//  NotifyAttributeChanged so the ASC's Blueprint delegate fires.
// ------------------------------------------------

void UMyraAttributeSet::PostAttributeChange(
	const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	UMyraAbilitySystemComponent* MYRAASC = Cast<UMyraAbilitySystemComponent>(
		GetOwningAbilitySystemComponent());

	if (MYRAASC)
	{
		MYRAASC->NotifyAttributeChanged(Attribute, OldValue, NewValue);
	}
}

// ------------------------------------------------
//  Helpers
// ------------------------------------------------

void UMyraAttributeSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.f); // MaxHealth must always be at least 1
	}
}
