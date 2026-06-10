// Copyright Myra . All Rights Reserved.

#include "Character/MyraCharacter.h"
#include "AbilitySystem/MyraDefaultAttributeSet.h"
#include "Character/MyraPawnExtensionComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AMyraCharacter::AMyraCharacter()
{
	OwnedAbilitySystemComponent = CreateDefaultSubobject<UMyraAbilitySystemComponent>(TEXT("OwnedAbilitySystemComponent"));
	OwnedAbilitySystemComponent->SetIsReplicated(true);
	PawnAbilityComponent = CreateDefaultSubobject<UMyraPawnAbilityComponent>(TEXT("PawnAbilityComponent"));
	PawnExtensionComponent = CreateDefaultSubobject<UMyraPawnExtensionComponent>(TEXT("PawnExtensionComponent"));
}

UAbilitySystemComponent* AMyraCharacter::GetAbilitySystemComponent() const
{
	return GetMyraAbilitySystemComponent();
}

void AMyraCharacter::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	if (const UMyraAbilitySystemComponent* AbilitySystemComponent = GetMyraAbilitySystemComponent())
	{
		AbilitySystemComponent->GetOwnedGameplayTags(TagContainer);
	}
}

UMyraAbilitySystemComponent* AMyraCharacter::GetMyraAbilitySystemComponent() const
{
	return PawnAbilityComponent ? PawnAbilityComponent->GetMyraAbilitySystemComponent() : nullptr;
}

const UMyraDefaultAttributeSet* AMyraCharacter::GetBaseAttributeSet() const
{
	return PawnAbilityComponent ? PawnAbilityComponent->GetBaseAttributeSet() : nullptr;
}

float AMyraCharacter::GetHealth() const
{
	return PawnAbilityComponent ? PawnAbilityComponent->GetHealth() : 0.f;
}

float AMyraCharacter::GetMaxHealth() const
{
	return PawnAbilityComponent ? PawnAbilityComponent->GetMaxHealth() : 1.f;
}

float AMyraCharacter::GetHealthPercent() const
{
	return PawnAbilityComponent ? PawnAbilityComponent->GetHealthPercent() : 0.f;
}

bool AMyraCharacter::IsAlive() const
{
	return PawnAbilityComponent ? PawnAbilityComponent->IsAlive() : false;
}

bool AMyraCharacter::IsUsingPlayerStateAbilitySystem() const
{
	return PawnAbilityComponent ? PawnAbilityComponent->IsUsingPlayerStateAbilitySystem() : false;
}

bool AMyraCharacter::ModifyDamageBeforeApplication_Implementation(float InDamage, float& OutDamage)
{
	OutDamage = InDamage;
	return false;
}

void AMyraCharacter::OnGameplayEffectExecuted_Implementation(const FMyraGEExecutedInfo& Info)
{
}

void AMyraCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (PawnAbilityComponent)
	{
		PawnAbilityComponent->HandlePawnBeginPlay();
	}
}

void AMyraCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (PawnAbilityComponent)
	{
		PawnAbilityComponent->HandlePawnPossessed(NewController);
	}
}

void AMyraCharacter::UnPossessed()
{
	if (PawnAbilityComponent)
	{
		PawnAbilityComponent->HandlePawnUnpossessed();
	}

	Super::UnPossessed();
}

void AMyraCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (PawnAbilityComponent)
	{
		PawnAbilityComponent->HandlePawnPlayerStateReplicated();
	}
}

void AMyraCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (PawnAbilityComponent)
	{
		PawnAbilityComponent->HandlePawnEndPlay();
	}

	Super::EndPlay(EndPlayReason);
}

void AMyraCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (PawnExtensionComponent)
	{
		PawnExtensionComponent->HandleControllerChanged();
		PawnExtensionComponent->SetupPlayerInputComponent(PlayerInputComponent);
	}
}

void AMyraCharacter::OnAbilitySystemInitialized()
{
}

void AMyraCharacter::OnDeath_Implementation(APawn* DeadPawn)
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
}

void AMyraCharacter::HandleMyraAbilitySystemInitialized()
{
	OnAbilitySystemInitialized();
}

void AMyraCharacter::HandleMyraAbilitySystemUninitialized()
{
}

void AMyraCharacter::HandleMyraHealthChanged(float OldValue, float NewValue)
{
	OnHealthChanged.Broadcast(this, OldValue, NewValue);
}

bool AMyraCharacter::ModifyMyraDamageBeforeApplication(float InDamage, float& OutDamage)
{
	return ModifyDamageBeforeApplication(InDamage, OutDamage);
}

void AMyraCharacter::HandleMyraDeath(APawn* DeadPawn)
{
	OnDeath(DeadPawn);
}

void AMyraCharacter::HandleMyraGameplayEffectExecuted(const FMyraGEExecutedInfo& Info)
{
	OnGameplayEffectExecuted(Info);
}
