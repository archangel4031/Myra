// Copyright Myra . All Rights Reserved.

#include "AbilitySystem/MyraDefaultAttributeSet.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"
#include "Tags/MyraNativeGameplayTags.h"
#include "AbilitySystem/MyraAbilitySystemComponent.h"

UMyraDefaultAttributeSet::UMyraDefaultAttributeSet()
{
	InitHealth(100.f);
	InitMaxHealth(100.f);
	InitDamage(0.f);
	InitHealing(0.f);
}

void UMyraDefaultAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UMyraDefaultAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMyraDefaultAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
}

void UMyraDefaultAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	ClampAttribute(Attribute, NewValue);
}

void UMyraDefaultAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	if (!ASC)
	{
		Super::PostGameplayEffectExecute(Data);
		return;
	}

	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		const float LocalDamage = GetDamage();
		SetDamage(0.f);

		if (LocalDamage > 0.f)
		{
			// Give Blueprint a chance to route damage before it reaches Health.
			// The default implementation is a passthrough (returns LocalDamage unchanged).
			// A Blueprint subclass of UMyraAbilitySystemComponent can override
			// ModifyDamageBeforeApplication to absorb some or all damage into Shield first
			// and return only the remainder to be applied here.
			float FinalDamage = LocalDamage;
			if (UMyraAbilitySystemComponent* MyraASC = Cast<UMyraAbilitySystemComponent>(ASC))
			{
				FinalDamage = MyraASC->ModifyDamageBeforeApplication(LocalDamage);
			}

			if (FinalDamage > 0.f)
			{
				const float NewHealth = FMath::Clamp(GetHealth() - FinalDamage, 0.f, GetMaxHealth());
				SetHealth(NewHealth);

				if (NewHealth <= 0.f)
				{
					FGameplayEventData EventData;
					EventData.EventMagnitude = FinalDamage;
					UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
						ASC->GetAvatarActor(),
						MyraGameplayTags::Myra_GameEvent_Death,
						EventData);
				}
			}
		}
	}

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

	SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));

	// Run the shared Myra broadcast logic after the final post-processed values are set.
	Super::PostGameplayEffectExecute(Data);
}

void UMyraDefaultAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMyraDefaultAttributeSet, Health, OldValue);
}

void UMyraDefaultAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMyraDefaultAttributeSet, MaxHealth, OldValue);
}

void UMyraDefaultAttributeSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.f);
	}
}