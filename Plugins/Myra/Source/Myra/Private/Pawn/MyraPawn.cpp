// Copyright Myra . All Rights Reserved.

#include "Pawn/MyraPawn.h"
#include "AbilitySystem/MyraDefaultAttributeSet.h"
#include "Character/MyraPawnExtensionComponent.h"
#include "Components/InputComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/PawnMovementComponent.h"

AMyraPawn::AMyraPawn()
{
	OwnedAbilitySystemComponent = CreateDefaultSubobject<UMyraAbilitySystemComponent>(TEXT("OwnedAbilitySystemComponent"));
	OwnedAbilitySystemComponent->SetIsReplicated(true);
	PawnAbilityComponent = CreateDefaultSubobject<UMyraPawnAbilityComponent>(TEXT("PawnAbilityComponent"));
	PawnExtensionComponent = CreateDefaultSubobject<UMyraPawnExtensionComponent>(TEXT("PawnExtensionComponent"));
}

UAbilitySystemComponent* AMyraPawn::GetAbilitySystemComponent() const
{
	return GetMyraAbilitySystemComponent();
}

void AMyraPawn::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	if (const UMyraAbilitySystemComponent* AbilitySystemComponent = GetMyraAbilitySystemComponent())
	{
		AbilitySystemComponent->GetOwnedGameplayTags(TagContainer);
	}
}

UMyraAbilitySystemComponent* AMyraPawn::GetMyraAbilitySystemComponent() const
{
	return PawnAbilityComponent ? PawnAbilityComponent->GetMyraAbilitySystemComponent() : nullptr;
}

const UMyraDefaultAttributeSet* AMyraPawn::GetBaseAttributeSet() const
{
	return PawnAbilityComponent ? PawnAbilityComponent->GetBaseAttributeSet() : nullptr;
}

float AMyraPawn::GetHealth() const
{
	return PawnAbilityComponent ? PawnAbilityComponent->GetHealth() : 0.f;
}

float AMyraPawn::GetMaxHealth() const
{
	return PawnAbilityComponent ? PawnAbilityComponent->GetMaxHealth() : 1.f;
}

float AMyraPawn::GetHealthPercent() const
{
	return PawnAbilityComponent ? PawnAbilityComponent->GetHealthPercent() : 0.f;
}

bool AMyraPawn::IsAlive() const
{
	return PawnAbilityComponent ? PawnAbilityComponent->IsAlive() : false;
}

bool AMyraPawn::IsUsingPlayerStateAbilitySystem() const
{
	return PawnAbilityComponent ? PawnAbilityComponent->IsUsingPlayerStateAbilitySystem() : false;
}

bool AMyraPawn::ModifyDamageBeforeApplication_Implementation(float InDamage, float& OutDamage)
{
	OutDamage = InDamage;
	return false;
}

void AMyraPawn::OnGameplayEffectExecuted_Implementation(const FMyraGEExecutedInfo& Info)
{
}

void AMyraPawn::BeginPlay()
{
	Super::BeginPlay();

	if (PawnAbilityComponent)
	{
		PawnAbilityComponent->HandlePawnBeginPlay();
	}
}

void AMyraPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (PawnAbilityComponent)
	{
		PawnAbilityComponent->HandlePawnPossessed(NewController);
	}
}

void AMyraPawn::UnPossessed()
{
	if (PawnAbilityComponent)
	{
		PawnAbilityComponent->HandlePawnUnpossessed();
	}

	Super::UnPossessed();
}

void AMyraPawn::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (PawnAbilityComponent)
	{
		PawnAbilityComponent->HandlePawnPlayerStateReplicated();
	}
}

void AMyraPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (PawnAbilityComponent)
	{
		PawnAbilityComponent->HandlePawnEndPlay();
	}

	Super::EndPlay(EndPlayReason);
}

void AMyraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (PawnExtensionComponent)
	{
		PawnExtensionComponent->HandleControllerChanged();
		PawnExtensionComponent->SetupPlayerInputComponent(PlayerInputComponent);
	}
}

void AMyraPawn::OnAbilitySystemInitialized()
{
}

void AMyraPawn::OnDeath_Implementation(AActor* Killer)
{
	if (UPrimitiveComponent* RootPrimitiveComponent = Cast<UPrimitiveComponent>(GetRootComponent()))
	{
		RootPrimitiveComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (UPawnMovementComponent* PawnMovementComponent = GetMovementComponent())
	{
		PawnMovementComponent->StopMovementImmediately();
		PawnMovementComponent->Deactivate();
	}
}

void AMyraPawn::HandleMyraAbilitySystemInitialized()
{
	OnAbilitySystemInitialized();
}

void AMyraPawn::HandleMyraAbilitySystemUninitialized()
{
}

void AMyraPawn::HandleMyraHealthChanged(float OldValue, float NewValue)
{
	OnHealthChanged.Broadcast(this, OldValue, NewValue);
}

bool AMyraPawn::ModifyMyraDamageBeforeApplication(float InDamage, float& OutDamage)
{
	return ModifyDamageBeforeApplication(InDamage, OutDamage);
}

void AMyraPawn::HandleMyraDeath(AActor* Killer)
{
	OnDeathEvent.Broadcast(this, Killer);
	OnDeath(Killer);
}

void AMyraPawn::HandleMyraGameplayEffectExecuted(const FMyraGEExecutedInfo& Info)
{
	OnGameplayEffectExecuted(Info);
}
