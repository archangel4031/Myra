// Copyright Myra . All Rights Reserved.

#include "Character/MyraPawnExtensionComponent.h"
#include "AbilitySystem/MyraAbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Character/MyraInputComponent.h"
#include "Character/MyraPlayerState.h"
#include "Components/InputComponent.h"
#include "DataAssets/MyraAbilitySet.h"
#include "DataAssets/MyraPawnData.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

UMyraPawnExtensionComponent::UMyraPawnExtensionComponent()
{
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
}

void UMyraPawnExtensionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UEnhancedInputComponent* InputComponent = CachedInputComponent.Get())
	{
		for (const uint32 BindingHandle : BoundInputHandles)
		{
			InputComponent->RemoveBindingByHandle(BindingHandle);
		}
	}

	if (bInputMappingsApplied && PawnData)
	{
		if (APawn* Pawn = Cast<APawn>(GetOwner()))
		{
			if (APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController()))
			{
				if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
				{
					if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
						LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
					{
						for (const FMyraInputMappingContext& MappingEntry : PawnData->DefaultInputMappings)
						{
							if (MappingEntry.MappingContext)
							{
								InputSubsystem->RemoveMappingContext(MappingEntry.MappingContext);
							}
						}
					}
				}
			}
		}
	}

	CachedInputComponent.Reset();
	BoundInputHandles.Reset();

	Super::EndPlay(EndPlayReason);
}

void UMyraPawnExtensionComponent::HandlePlayerStateReplicated()
{
	CheckPawnReadyToInitialize();
	TryInitializePlayerInput();
}

void UMyraPawnExtensionComponent::HandleAvatarSet()
{
	bAvatarReady = true;
	CheckPawnReadyToInitialize();
	TryInitializePlayerInput();
}

void UMyraPawnExtensionComponent::HandleControllerChanged()
{
	TryInitializePlayerInput();
}

void UMyraPawnExtensionComponent::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (CachedInputComponent.Get() != PlayerInputComponent)
	{
		if (UEnhancedInputComponent* ExistingInputComponent = CachedInputComponent.Get())
		{
			for (const uint32 BindingHandle : BoundInputHandles)
			{
				ExistingInputComponent->RemoveBindingByHandle(BindingHandle);
			}
		}

		CachedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
		bAbilityInputBound = false;
		BoundInputHandles.Reset();
	}

	TryInitializePlayerInput();
}

void UMyraPawnExtensionComponent::CheckPawnReadyToInitialize()
{
	if (bPawnReadyToInitialize)
	{
		return;
	}

	if (!bAvatarReady || !GetMyraAbilitySystemComponent())
	{
		return;
	}

	bPawnReadyToInitialize = true;

	ApplyPawnData();
	OnPawnReadyToInitialize.Broadcast();
	TryInitializePlayerInput();

	UE_LOG(LogTemp, Log, TEXT("MyraPawnExtensionComponent: %s is ready — Myra initialized."),
		*GetOwner()->GetName());
}

void UMyraPawnExtensionComponent::ApplyPawnData()
{
	if (!PawnData)
	{
		return;
	}

	UMyraAbilitySystemComponent* ASC = GetMyraAbilitySystemComponent();
	if (!ASC)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("MyraPawnExtensionComponent: ApplyPawnData called but no ASC found on '%s'."),
			*GetOwner()->GetName());
		return;
	}

	if (GetOwner()->HasAuthority())
	{
		for (UMyraAbilitySet* AbilitySet : PawnData->AbilitySets)
		{
			if (AbilitySet)
			{
				ASC->GrantAbilitySet(AbilitySet, GetOwner());
			}
		}

		if (PawnData->DefaultAttributeInitEffect)
		{
			ASC->ApplyInitializationEffectOnce(PawnData->DefaultAttributeInitEffect, 1.f, PawnData);
		}
	}
}

void UMyraPawnExtensionComponent::TryInitializePlayerInput()
{
	if (!bPawnReadyToInitialize || bAbilityInputBound)
	{
		return;
	}

	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn || !Pawn->IsLocallyControlled() || !PawnData)
	{
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController());
	if (!PlayerController)
	{
		return;
	}

	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}

	if (!bInputMappingsApplied)
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
			LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			for (const FMyraInputMappingContext& MappingEntry : PawnData->DefaultInputMappings)
			{
				if (MappingEntry.MappingContext)
				{
					InputSubsystem->AddMappingContext(MappingEntry.MappingContext, MappingEntry.Priority);
				}
			}

			bInputMappingsApplied = true;
		}
	}

	if (!PawnData->InputConfig)
	{
		bAbilityInputBound = true;
		return;
	}

	UEnhancedInputComponent* InputComponent = CachedInputComponent.Get();
	UMyraAbilitySystemComponent* ASC = GetMyraAbilitySystemComponent();
	if (!InputComponent || !ASC)
	{
		return;
	}

	UMyraInputComponent::BindAbilityActions(InputComponent, PawnData->InputConfig, ASC, BoundInputHandles);
	bAbilityInputBound = true;
}

UMyraAbilitySystemComponent* UMyraPawnExtensionComponent::GetMyraAbilitySystemComponent() const
{
	if (const IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(GetOwner()))
	{
		return Cast<UMyraAbilitySystemComponent>(AbilitySystemInterface->GetAbilitySystemComponent());
	}

	if (const APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		if (const AMyraPlayerState* PS = Pawn->GetPlayerState<AMyraPlayerState>())
		{
			return PS->GetMyraAbilitySystemComponent();
		}
	}

	return nullptr;
}
