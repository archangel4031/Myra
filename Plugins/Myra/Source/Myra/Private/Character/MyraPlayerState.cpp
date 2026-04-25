// Copyright Myra . All Rights Reserved.

#include "Character/MyraPlayerState.h"
#include "AbilitySystem/MyraAbilitySystemComponent.h"
#include "AbilitySystem/MyraDefaultAttributeSet.h"
#include "DataAssets/MyraAbilitySet.h"
#include "Net/UnrealNetwork.h"

AMyraPlayerState::AMyraPlayerState()
{
	// PlayerState already ticks and replicates. We just need to make sure the ASC
	// net update frequency is high enough for responsive ability feedback.
	SetNetUpdateFrequency(100.f);

	// Create the ASC. It lives on PlayerState so it persists across respawns.
	AbilitySystemComponent = CreateDefaultSubobject<UMyraAbilitySystemComponent>(
		TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// Create the base attribute set. Subobjects created on the owner are auto-registered
	// with the ASC — no manual registration needed.
	AttributeSet = CreateDefaultSubobject<UMyraDefaultAttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* AMyraPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

float AMyraPlayerState::GetHealth() const
{
	return AttributeSet ? AttributeSet->GetHealth() : 0.f;
}

float AMyraPlayerState::GetMaxHealth() const
{
	return AttributeSet ? AttributeSet->GetMaxHealth() : 0.f;
}

float AMyraPlayerState::GetHealthPercent() const
{
	const float Max = GetMaxHealth();
	return (Max > 0.f) ? (GetHealth() / Max) : 0.f;
}

void AMyraPlayerState::BeginPlay()
{
	Super::BeginPlay();

	// Initialize owner-side ASC state on the server once the PlayerState is live.
	// The avatar may still be null here; the Character will re-init later with itself
	// as the avatar from PossessedBy / OnRep_PlayerState.

	// CLAUDE:
	// Do NOT initialize ASC here. The Character's PossessedBy (server)
	// and OnRep_PlayerState (client) handle InitAbilityActorInfo with a valid avatar.
	// GrantDefaultAbilitySets is called from there via InitAbilitySystemForPlayerState.
	//if (HasAuthority() && AbilitySystemComponent)
	//{
	//	AbilitySystemComponent->InitAbilityActorInfo(this, GetPawn());
	//	GrantDefaultAbilitySets();
	//}
}

void AMyraPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void AMyraPlayerState::OnAbilityActorInfoSet()
{
	// Override in subclasses. Called after the avatar is bound to the ASC.
}

void AMyraPlayerState::GrantDefaultAbilitySets()
{
	for (UMyraAbilitySet* AbilitySet : DefaultAbilitySets)
	{
		if (AbilitySet)
		{
			AbilitySystemComponent->GrantAbilitySet(AbilitySet, this);
		}
	}
}
