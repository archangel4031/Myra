// Copyright Myra . All Rights Reserved.

#include "Character/MyraCharacter.h"
#include "AbilitySystem/MyraAbilitySystemComponent.h"
#include "AbilitySystem/MyraDefaultAttributeSet.h"
#include "Character/MyraPawnExtensionComponent.h"
#include "Character/MyraPlayerState.h"
#include "DataAssets/MyraAbilitySet.h"
#include "Tags/MyraNativeGameplayTags.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Components/InputComponent.h"
#include "GameplayTagContainer.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayEffect.h"

AMyraCharacter::AMyraCharacter()
{
	// Keep an owned ASC on the Character for the beginner-friendly path and as a fallback
	// when PlayerState setup is intentionally skipped or unavailable.
	OwnedAbilitySystemComponent = CreateDefaultSubobject<UMyraAbilitySystemComponent>(
		TEXT("OwnedAbilitySystemComponent"));
	
	// Now this should happen in Begin Play
	//OwnedAbilitySystemComponent->SetIsReplicated(true);

	OwnedAttributeSet = CreateDefaultSubobject<UMyraDefaultAttributeSet>(TEXT("OwnedAttributeSet"));

	// Pawn Extension Component is always created, but it will only do something if the ASC is initialized and tells it to react.
	PawnExtensionComponent = CreateDefaultSubobject<UMyraPawnExtensionComponent>(TEXT("PawnExtensionComponent"));
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
	return ResolvedAbilitySystemComponent;
}

const UMyraDefaultAttributeSet* AMyraCharacter::GetBaseAttributeSet() const
{
	if (ResolvedAbilitySystemComponent)
	{
		return ResolvedAbilitySystemComponent->GetSet<UMyraDefaultAttributeSet>();
	}
	return nullptr;
}

// ------------------------------------------------
//  IGameplayTagAssetInterface
// ------------------------------------------------

void AMyraCharacter::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	if (ResolvedAbilitySystemComponent)
	{
		ResolvedAbilitySystemComponent->GetOwnedGameplayTags(TagContainer);
	}
}

// ------------------------------------------------
//  Initialization — SERVER
// ------------------------------------------------

void AMyraCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (bUsePlayerStateASC && InitAbilitySystemForPlayerState())
	{
		return;
	}

	InitAbilitySystemOwned();
}

void AMyraCharacter::UnPossessed()
{
	if (IsUsingPlayerStateAbilitySystem())
	{
		HandlePlayerStateAbilitySystemRemoved();
	}

	Super::UnPossessed();
}

// ------------------------------------------------
//  Initialization — CLIENT
// ------------------------------------------------

void AMyraCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (bUsePlayerStateASC && InitAbilitySystemForPlayerState())
	{
		return;
	}

	// Fallback to the Character ASC when PlayerState setup is unavailable.
	if (!bAbilitySystemInitialized)
	{
		InitAbilitySystemOwned();
	}
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

void AMyraCharacter::BeginPlay()
{
	Super::BeginPlay();

	// CLAUDE: For PlayerState ASC, replication is handled by the PlayerState. The Character's owned ASC should not replicate to avoid conflicts.
	if (bUsePlayerStateASC)
	{
		OwnedAbilitySystemComponent->SetIsReplicated(false);
	}

	// AI characters and single-player pawns without a PlayerController
	// never call PossessedBy on the server via a player. Init here.
	if (!bUsePlayerStateASC && !bAbilitySystemInitialized)
	{
		InitAbilitySystemOwned();
	}
}

void AMyraCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsUsingPlayerStateAbilitySystem())
	{
		HandlePlayerStateAbilitySystemRemoved();
	}

	Super::EndPlay(EndPlayReason);
}

// ------------------------------------------------
//  ASC Initialization — PlayerState path
// ------------------------------------------------

bool AMyraCharacter::InitAbilitySystemForPlayerState()
{
	AMyraPlayerState* MyraPlayerState = GetPlayerState<AMyraPlayerState>();
	if (!MyraPlayerState)
	{
		return false;
	}

	ResolvedAbilitySystemComponent = MyraPlayerState->GetMyraAbilitySystemComponent();
	if (!ResolvedAbilitySystemComponent)
	{
		return false;
	}

	// Tell the ASC which actor is its "avatar" (the physical Character)
	// and which actor owns it (the PlayerState).
	ResolvedAbilitySystemComponent->InitAbilityActorInfo(MyraPlayerState, this);

	// Grant PlayerState-level ability sets now that the avatar is valid
	if (GetLocalRole() == ROLE_Authority)
	{
		MyraPlayerState->GrantDefaultAbilitySets();
	}

	if (PawnExtensionComponent)
	{
		PawnExtensionComponent->HandlePlayerStateReplicated();
		PawnExtensionComponent->HandleAvatarSet();
	}

	OnAbilitySystemInitialized();
	return true;
}

// ------------------------------------------------
//  ASC Initialization — Owned (AI / single-player) path
// ------------------------------------------------

void AMyraCharacter::InitAbilitySystemOwned()
{
	ResolvedAbilitySystemComponent = OwnedAbilitySystemComponent;

	// For owned ASC, owner and avatar are both the Character.
	ResolvedAbilitySystemComponent->InitAbilityActorInfo(this, this);

	// Grant ability sets on the server only.
	if (GetLocalRole() == ROLE_Authority)
	{
		for (UMyraAbilitySet* AbilitySet : DefaultAbilitySets)
		{
			if (AbilitySet)
			{
				ResolvedAbilitySystemComponent->GrantAbilitySet(AbilitySet, this);
			}
		}
	}

	if (PawnExtensionComponent)
	{
		PawnExtensionComponent->HandleAvatarSet();
	}

	OnAbilitySystemInitialized();
}

void AMyraCharacter::HandlePlayerStateAbilitySystemRemoved()
{
	if (PawnExtensionComponent)
	{
		PawnExtensionComponent->HandlePawnUninitialized();
	}

	ResolvedAbilitySystemComponent = nullptr;
	bAbilitySystemInitialized = false;
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

	// Apply startup GE to set initial attribute values.
	if (GetLocalRole() == ROLE_Authority)
	{
		ApplyDefaultAttributeInitEffect();
	}

	// Bind to attribute change delegates for Blueprint events.
	BindAttributeChangeCallbacks();
}

void AMyraCharacter::ApplyDefaultAttributeInitEffect()
{
	if (!IsUsingPlayerStateAbilitySystem() && DefaultAttributeInitEffect && ResolvedAbilitySystemComponent)
	{
		ResolvedAbilitySystemComponent->ApplyInitializationEffectOnce(DefaultAttributeInitEffect, 1.f, this);
	}
}

void AMyraCharacter::BindAttributeChangeCallbacks()
{
	if (!ResolvedAbilitySystemComponent)
	{
		return;
	}

	// Bind to Health changes to broadcast our Blueprint delegate and handle death.
	ResolvedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		UMyraDefaultAttributeSet::GetHealthAttribute())
		.RemoveAll(this);
	ResolvedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		UMyraDefaultAttributeSet::GetHealthAttribute())
		.AddUObject(this, &AMyraCharacter::HandleHealthChanged);

	// Listen for the Death gameplay tag being added (set by the attribute set).
	ResolvedAbilitySystemComponent->RegisterGameplayTagEvent(
		MyraGameplayTags::Myra_State_Dead,
		EGameplayTagEventType::NewOrRemoved)
		.RemoveAll(this);
	ResolvedAbilitySystemComponent->RegisterGameplayTagEvent(
		MyraGameplayTags::Myra_State_Dead,
		EGameplayTagEventType::NewOrRemoved)
		.AddUObject(this, &AMyraCharacter::HandleDeathTag);

	// Remove any previous binding first to avoid double-binding on re-init.
	ResolvedAbilitySystemComponent->OnGameplayEffectAttributeExecuted.RemoveDynamic(
		this, &AMyraCharacter::HandleGameplayEffectExecuted);
	ResolvedAbilitySystemComponent->OnGameplayEffectAttributeExecuted.AddDynamic(
		this, &AMyraCharacter::HandleGameplayEffectExecuted);
}

bool AMyraCharacter::IsUsingPlayerStateAbilitySystem() const
{
	if (!bUsePlayerStateASC || !ResolvedAbilitySystemComponent)
	{
		return false;
	}

	const AMyraPlayerState* MyraPlayerState = GetPlayerState<AMyraPlayerState>();
	return MyraPlayerState && ResolvedAbilitySystemComponent == MyraPlayerState->GetMyraAbilitySystemComponent();
}

// ------------------------------------------------
//  Attribute Helpers
// ------------------------------------------------

float AMyraCharacter::GetHealth() const
{
	if (const UMyraDefaultAttributeSet* AS = GetBaseAttributeSet())
	{
		return AS->GetHealth();
	}
	return 0.f;
}

float AMyraCharacter::GetMaxHealth() const
{
	if (const UMyraDefaultAttributeSet* AS = GetBaseAttributeSet())
	{
		return AS->GetMaxHealth();
	}
	return 1.f;
}

float AMyraCharacter::GetHealthPercent() const
{
	const float Max = GetMaxHealth();
	return (Max > 0.f) ? (GetHealth() / Max) : 0.f;
}

bool AMyraCharacter::ModifyDamageBeforeApplication_Implementation(float InDamage, float& OutDamage)
{
	// Returning false means "not handled here", so PlayerState-owned ASCs can still
	// fall back to the PlayerState override when the Character does not customize routing.
	OutDamage = InDamage;
	return false;
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

	if (IsUsingPlayerStateAbilitySystem())
	{
		if (PawnExtensionComponent)
		{
			PawnExtensionComponent->HandlePawnUninitialized();
		}
	}

	// Default behaviour: disable collision and stop all movement.
	// Override in your subclass to play death animations, ragdoll, etc.
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
}


void AMyraCharacter::HandleGameplayEffectExecuted(const FMyraGEExecutedInfo& Info)
{
	// Route the ASC delegate into the Blueprint-overridable NativeEvent.
	OnGameplayEffectExecuted(Info);
}

void AMyraCharacter::OnGameplayEffectExecuted_Implementation(const FMyraGEExecutedInfo& Info)
{
	// Default implementation is intentionally empty.
	// Override this event in your Character Blueprint to react to GE executions.
}
