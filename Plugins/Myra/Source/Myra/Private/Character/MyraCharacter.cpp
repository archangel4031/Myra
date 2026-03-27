// Copyright Myra . All Rights Reserved.

#include "Character/MyraCharacter.h"
#include "AbilitySystem/MyraAbilitySystemComponent.h"
#include "AbilitySystem/MyraAttributeSet.h"
#include "Character/MyraPlayerState.h"
#include "DataAssets/MyraAbilitySet.h"
#include "Tags/MyraNativeGameplayTags.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayTagContainer.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayEffect.h"

AMyraCharacter::AMyraCharacter()
{
	// When using PlayerState ASC, we don't create components here.
	// When bUsePlayerStateASC = false, we create them in the constructor.
	// However, we defer reading bUsePlayerStateASC to BeginPlay/PossessedBy
	// because CDO defaults may not reflect the final subclass value here.

	// AI / single-player characters always create their own ASC.
	// This is created unconditionally but only used when bUsePlayerStateASC = false.
	OwnedAbilitySystemComponent = CreateDefaultSubobject<UMyraAbilitySystemComponent>(
		TEXT("OwnedAbilitySystemComponent"));
	OwnedAbilitySystemComponent->SetIsReplicated(true);

	OwnedAttributeSet = CreateDefaultSubobject<UMyraAttributeSet>(TEXT("OwnedAttributeSet"));
}

// ------------------------------------------------
//  IAbilitySystemInterface
// ------------------------------------------------

UAbilitySystemComponent* AMyraCharacter::GetAbilitySystemComponent() const
{
	return GetMyraAbilitySystemComponent();
}

UMyraAbilitySystemComponent* AMyraCharacter::GetMyraAbilitySystemComponent() const
{
	return AbilitySystemComponentRef;
}

const UMyraAttributeSet* AMyraCharacter::GetBaseAttributeSet() const
{
	if (AbilitySystemComponentRef)
	{
		return AbilitySystemComponentRef->GetSet<UMyraAttributeSet>();
	}
	return nullptr;
}

// ------------------------------------------------
//  IGameplayTagAssetInterface
// ------------------------------------------------

void AMyraCharacter::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	if (AbilitySystemComponentRef)
	{
		AbilitySystemComponentRef->GetOwnedGameplayTags(TagContainer);
	}
}

// ------------------------------------------------
//  Initialization — SERVER
// ------------------------------------------------

void AMyraCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (bUsePlayerStateASC)
	{
		InitAbilitySystemForPlayerState();
	}
	else
	{
		InitAbilitySystemOwned();
	}
}

// ------------------------------------------------
//  Initialization — CLIENT
// ------------------------------------------------

void AMyraCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (bUsePlayerStateASC)
	{
		InitAbilitySystemForPlayerState();
	}
	// Owned ASC path: already initialized on BeginPlay
}

void AMyraCharacter::BeginPlay()
{
	Super::BeginPlay();

	// AI characters and single-player pawns without a PlayerController
	// never call PossessedBy on the server via a player. Init here.
	if (!bUsePlayerStateASC && !bAbilitySystemInitialized)
	{
		InitAbilitySystemOwned();
	}
}

// ------------------------------------------------
//  ASC Initialization — PlayerState path
// ------------------------------------------------

void AMyraCharacter::InitAbilitySystemForPlayerState()
{
	AMyraPlayerState* MyraPlayerState = GetPlayerState<AMyraPlayerState>();
	if (!MyraPlayerState)
	{
		return;
	}

	AbilitySystemComponentRef = MyraPlayerState->GetMyraAbilitySystemComponent();
	if (!AbilitySystemComponentRef)
	{
		return;
	}

	// Tell the ASC which actor is its "avatar" (the physical Character)
	// and which actor owns it (the PlayerState).
	AbilitySystemComponentRef->InitAbilityActorInfo(MyraPlayerState, this);

	OnAbilitySystemInitialized();
}

// ------------------------------------------------
//  ASC Initialization — Owned (AI / single-player) path
// ------------------------------------------------

void AMyraCharacter::InitAbilitySystemOwned()
{
	AbilitySystemComponentRef = OwnedAbilitySystemComponent;

	// For owned ASC, owner and avatar are both the Character.
	AbilitySystemComponentRef->InitAbilityActorInfo(this, this);

	// Grant ability sets on the server only.
	if (GetLocalRole() == ROLE_Authority)
	{
		for (UMyraAbilitySet* AbilitySet : DefaultAbilitySets)
		{
			if (AbilitySet)
			{
				AbilitySystemComponentRef->GrantAbilitySet(AbilitySet, this);
			}
		}
	}

	OnAbilitySystemInitialized();
}

// ------------------------------------------------
//  Post-initialization
// ------------------------------------------------

void AMyraCharacter::OnAbilitySystemInitialized()
{
	if (bAbilitySystemInitialized)
	{
		return;
	}
	bAbilitySystemInitialized = true;

	// Apply startup GE to set initial attribute values (Health=100, Mana=50, etc.)
	if (GetLocalRole() == ROLE_Authority)
	{
		ApplyDefaultAttributeInitEffect();
	}

	// Bind to attribute change delegates for Blueprint events.
	BindAttributeChangeCallbacks();
}

void AMyraCharacter::ApplyDefaultAttributeInitEffect()
{
	if (DefaultAttributeInitEffect && AbilitySystemComponentRef)
	{
		AbilitySystemComponentRef->ApplyEffectToSelf(DefaultAttributeInitEffect, 1.f);
	}
}

void AMyraCharacter::BindAttributeChangeCallbacks()
{
	if (!AbilitySystemComponentRef)
	{
		return;
	}

	// Bind to Health changes to broadcast our Blueprint delegate and handle death.
	AbilitySystemComponentRef->GetGameplayAttributeValueChangeDelegate(
		UMyraAttributeSet::GetHealthAttribute())
		.AddUObject(this, &AMyraCharacter::HandleHealthChanged);

	// Listen for the Death gameplay tag being added (set by the attribute set).
	AbilitySystemComponentRef->RegisterGameplayTagEvent(
		MyraGameplayTags::State_Dead,
		EGameplayTagEventType::NewOrRemoved)
		.AddUObject(this, &AMyraCharacter::HandleDeathTag);
}

// ------------------------------------------------
//  Attribute Helpers
// ------------------------------------------------

float AMyraCharacter::GetHealth() const
{
	if (const UMyraAttributeSet* AS = GetBaseAttributeSet())
	{
		return AS->GetHealth();
	}
	return 0.f;
}

float AMyraCharacter::GetMaxHealth() const
{
	if (const UMyraAttributeSet* AS = GetBaseAttributeSet())
	{
		return AS->GetMaxHealth();
	}
	return 1.f;
}

float AMyraCharacter::GetMana() const
{
	if (const UMyraAttributeSet* AS = GetBaseAttributeSet())
	{
		return AS->GetMana();
	}
	return 0.f;
}

float AMyraCharacter::GetMaxMana() const
{
	if (const UMyraAttributeSet* AS = GetBaseAttributeSet())
	{
		return AS->GetMaxMana();
	}
	return 1.f;
}

float AMyraCharacter::GetStamina() const
{
	if (const UMyraAttributeSet* AS = GetBaseAttributeSet())
	{
		return AS->GetStamina();
	}
	return 0.f;
}

float AMyraCharacter::GetMaxStamina() const
{
	if (const UMyraAttributeSet* AS = GetBaseAttributeSet())
	{
		return AS->GetMaxStamina();
	}
	return 1.f;
}

float AMyraCharacter::GetHealthPercent() const
{
	const float Max = GetMaxHealth();
	return (Max > 0.f) ? (GetHealth() / Max) : 0.f;
}

// ------------------------------------------------
//  State Queries
// ------------------------------------------------

bool AMyraCharacter::IsAlive() const
{
	return GetHealth() > 0.f;
}

// ------------------------------------------------
//  Internal Callbacks
// ------------------------------------------------

void AMyraCharacter::HandleHealthChanged(const FOnAttributeChangeData& ChangeData)
{
	OnHealthChanged.Broadcast(this, ChangeData.OldValue, ChangeData.NewValue);
}

void AMyraCharacter::HandleDeathTag(const FGameplayTag GameplayTag, int32 NewCount)
{
	if (NewCount > 0 && IsAlive() == false)
	{
		// Who caused the death? Pull from the last damage GE instigator.
		AActor* Killer = nullptr; // Could be retrieved from GE context if stored.
		OnDeath(Killer);
	}
}

void AMyraCharacter::OnDeath_Implementation(AActor* Killer)
{
	OnDeathEvent.Broadcast(this, Killer);

	// Default behaviour: disable collision and stop all movement.
	// Override in your subclass to play death animations, ragdoll, etc.
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
}
