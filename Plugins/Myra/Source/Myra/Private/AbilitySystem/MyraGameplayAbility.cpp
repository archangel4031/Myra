// Copyright Myra . All Rights Reserved.

#include "AbilitySystem/MyraGameplayAbility.h"
#include "AbilitySystem/MyraAbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/Pawn.h"
#include "Pawn/MyraPawnAbilityComponent.h"

UMyraGameplayAbility::UMyraGameplayAbility()
{
	// By default abilities only activate if the actor is locally controlled.
	// Change to EGameplayAbilityNetExecutionPolicy::ServerOnly for server-authoritative abilities.
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	// Instanced abilities support per-activation state (timers, montage tracking, etc.).
	// InstancedPerActor = one instance per actor. InstancedPerExecution = one per activation.
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

// ------------------------------------------------
//  OnAvatarSet — called when the avatar is assigned to the ASC.
//  This is the right place to handle OnGranted / OnSpawn activation policies.
// ------------------------------------------------

void UMyraGameplayAbility::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);

	if (ActivationPolicy == EMyraAbilityActivationPolicy::OnGranted
		|| ActivationPolicy == EMyraAbilityActivationPolicy::OnSpawn)
	{
		ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle, true);
	}
}

// ------------------------------------------------
//  CanActivateAbility — extend the base check.
// ------------------------------------------------

bool UMyraGameplayAbility::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	if (const UMyraPawnAbilityComponent* PawnAbilityComponent =
		UMyraPawnAbilityComponent::FindPawnAbilityComponent(ActorInfo->AvatarActor.Get()))
	{
		if (!PawnAbilityComponent->IsAlive())
		{
			return false;
		}
	}

	return true;
}

// ------------------------------------------------
//  Typed Accessors
// ------------------------------------------------

APawn* UMyraGameplayAbility::GetMyraPawn() const
{
	return Cast<APawn>(GetAvatarActorFromActorInfo());
}

UMyraAbilitySystemComponent* UMyraGameplayAbility::GetMyraAbilitySystemComponent() const
{
	return Cast<UMyraAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
}

UMyraPawnAbilityComponent* UMyraGameplayAbility::GetMyraPawnAbilityComponent() const
{
	return UMyraPawnAbilityComponent::FindPawnAbilityComponent(GetAvatarActorFromActorInfo());
}

bool UMyraGameplayAbility::HasAbilityTag(FGameplayTag AbilityTag) const
{
	return AbilityTag.IsValid() && GetAssetTags().HasTag(AbilityTag);
}

// ------------------------------------------------
//  Cost & Cooldown
// ------------------------------------------------

float UMyraGameplayAbility::GetAbilityCostAmount() const
{
	UGameplayEffect* CostEffect = GetCostGameplayEffect();
	if (!CostEffect)
	{
		return 0.f;
	}

	for (const FGameplayModifierInfo& Modifier : CostEffect->Modifiers)
	{
		float Cost = 0.f;
		Modifier.ModifierMagnitude.GetStaticMagnitudeIfPossible(GetAbilityLevel(), Cost);
		if (Cost != 0.f)
		{
			return FMath::Abs(Cost);
		}
	}
	return 0.f;
}

float UMyraGameplayAbility::GetAbilityCooldownDuration() const
{
	UGameplayEffect* CDEffect = GetCooldownGameplayEffect();
	if (!CDEffect)
	{
		return 0.f;
	}

	float Duration = 0.f;
	CDEffect->DurationMagnitude.GetStaticMagnitudeIfPossible(GetAbilityLevel(), Duration);
	return Duration;
}

float UMyraGameplayAbility::GetAbilityCooldownTimeRemaining() const
{
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	if (!ActorInfo)
	{
		return 0.f;
	}

	const FGameplayTagContainer* CooldownTags = GetCooldownTags();
	if (!CooldownTags || CooldownTags->IsEmpty())
	{
		return 0.f;
	}

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (!ASC)
	{
		return 0.f;
	}

	FGameplayEffectQuery const Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(*CooldownTags);
	TArray<float> Durations = ASC->GetActiveEffectsTimeRemaining(Query);
	if (Durations.Num() > 0)
	{
		Durations.Sort();
		return Durations.Last(); // Return the longest remaining cooldown
	}
	return 0.f;
}

