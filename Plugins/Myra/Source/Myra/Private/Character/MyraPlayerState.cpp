// Copyright Myra . All Rights Reserved.

#include "Character/MyraPlayerState.h"
#include "AbilitySystem/MyraAbilitySystemComponent.h"
#include "AbilitySystem/MyraAttributeSet.h"
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
	AttributeSet = CreateDefaultSubobject<UMyraAttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* AMyraPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AMyraPlayerState::BeginPlay()
{
	Super::BeginPlay();

	// Only the server grants abilities. They replicate to clients automatically.
	if (GetLocalRole() == ROLE_Authority)
	{
		GrantDefaultAbilitySets();
	}
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
