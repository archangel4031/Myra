// Copyright Myra . All Rights Reserved.

#include "Character/MyraPawnExtensionComponent.h"
#include "AbilitySystem/MyraAbilitySystemComponent.h"
#include "DataAssets/MyraCharacterData.h"
#include "DataAssets/MyraAbilitySet.h"
#include "Character/MyraCharacter.h"
#include "Character/MyraPlayerState.h"
#include "GameFramework/CharacterMovementComponent.h"

UMyraPawnExtensionComponent::UMyraPawnExtensionComponent()
{
	// This component needs BeginPlay but doesn't need per-frame ticking.
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(false);
}

UMyraPawnExtensionComponent* UMyraPawnExtensionComponent::FindPawnExtensionComponent(const AActor* Actor)
{
	if (!Actor)
	{
		return nullptr;
	}
	return Actor->FindComponentByClass<UMyraPawnExtensionComponent>();
}

void UMyraPawnExtensionComponent::BeginPlay()
{
	Super::BeginPlay();

	// For non-player-controlled pawns (AI), there's no PlayerState and no controller
	// binding to wait for. Mark both as ready immediately.
	const APawn* Pawn = Cast<APawn>(GetOwner());
	if (Pawn && !Pawn->IsPlayerControlled())
	{
		bPlayerStateReady = true;
		bControllerReady  = true;
		// AvatarReady will be set when HandleAvatarSet is called.
	}
}

void UMyraPawnExtensionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

// ------------------------------------------------
//  Readiness Condition Setters
// ------------------------------------------------

void UMyraPawnExtensionComponent::HandlePlayerStateReplicated()
{
	bPlayerStateReady = true;
	CheckPawnReadyToInitialize();
}

void UMyraPawnExtensionComponent::HandleAvatarSet()
{
	bAvatarReady = true;
	CheckPawnReadyToInitialize();
}

void UMyraPawnExtensionComponent::HandleControllerChanged()
{
	bControllerReady = true;
	CheckPawnReadyToInitialize();
}

// ------------------------------------------------
//  Core Logic
// ------------------------------------------------

void UMyraPawnExtensionComponent::CheckPawnReadyToInitialize()
{
	// Already initialized — don't fire twice.
	if (bPawnReadyToInitialize)
	{
		return;
	}

	// All three conditions must be met:
	//   PlayerState: ASC owner is valid (multiplayer) or skipped (AI)
	//   Avatar:      InitAbilityActorInfo has been called (avatar pointer valid)
	//   Controller:  Pawn is possessed (ensures input binding works)
	if (!bPlayerStateReady || !bAvatarReady || !bControllerReady)
	{
		return;
	}

	bPawnReadyToInitialize = true;

	// Apply CharacterData (ability sets, init GE, mesh, etc.) on authority.
	ApplyCharacterData();

	// Broadcast so the Character or any external listener can do final setup.
	OnPawnReadyToInitialize.Broadcast();

	UE_LOG(LogTemp, Log, TEXT("MyraPawnExtensionComponent: %s is ready — Myra initialized."),
		*GetOwner()->GetName());
}

void UMyraPawnExtensionComponent::ApplyCharacterData()
{
	if (!CharacterData)
	{
		return;
	}

	UMyraAbilitySystemComponent* ASC = GetMyraAbilitySystemComponent();
	if (!ASC)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("MyraPawnExtensionComponent: ApplyCharacterData called but no ASC found on '%s'."),
			*GetOwner()->GetName());
		return;
	}

	// Only grant abilities on the server (they replicate to clients automatically).
	if (GetOwner()->HasAuthority())
	{
		// Grant each AbilitySet in the CharacterData.
		for (UMyraAbilitySet* AbilitySet : CharacterData->AbilitySets)
		{
			if (AbilitySet)
			{
				ASC->GrantAbilitySet(AbilitySet, GetOwner());
			}
		}

		// Apply the default attribute init GE to set starting stat values.
		if (CharacterData->DefaultAttributeInitEffect)
		{
			ASC->ApplyEffectToSelf(CharacterData->DefaultAttributeInitEffect, 1.f);
		}
	}

	// Apply mesh/anim data on all clients (cosmetic, no authority needed).
	// Loaded synchronously here; for large projects use async loading (see TODO below).
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (Pawn)
	{
		USkeletalMeshComponent* MeshComp = Pawn->FindComponentByClass<USkeletalMeshComponent>();
		if (MeshComp)
		{
			if (!CharacterData->CharacterMesh.IsNull())
			{
				// TODO: Use async asset loading (UAssetManager::GetStreamableManager().RequestAsyncLoad)
				// for production. Synchronous load is fine for prototyping.
				USkeletalMesh* Mesh = CharacterData->CharacterMesh.LoadSynchronous();
				if (Mesh)
				{
					MeshComp->SetSkeletalMesh(Mesh);
				}
			}

			if (!CharacterData->AnimationBlueprint.IsNull())
			{
				TSubclassOf<UAnimInstance> AnimClass = CharacterData->AnimationBlueprint.LoadSynchronous();
				if (AnimClass)
				{
					MeshComp->SetAnimInstanceClass(AnimClass);
				}
			}
		}

		// Apply movement speeds if set.
		if (ACharacter* Character = Cast<ACharacter>(Pawn))
		{
			UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement();

			// Ensure both the data and the component are valid before assignment
			if (MoveComp && CharacterData && CharacterData->WalkSpeed > 0.f)
			{
				MoveComp->MaxWalkSpeed = CharacterData->WalkSpeed;
			}
		}
	}
}

UMyraAbilitySystemComponent* UMyraPawnExtensionComponent::GetMyraAbilitySystemComponent() const
{
	// Try the Character-based accessor first.
	if (const AMyraCharacter* MyraChar = Cast<AMyraCharacter>(GetOwner()))
	{
		return MyraChar->GetMyraAbilitySystemComponent();
	}

	// Fallback: look on the PlayerState.
	if (const APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		if (const AMyraPlayerState* PS = Pawn->GetPlayerState<AMyraPlayerState>())
		{
			return PS->GetMyraAbilitySystemComponent();
		}
	}

	return nullptr;
}
