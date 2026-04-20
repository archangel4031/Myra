// Copyright Myra . All Rights Reserved.

#include "Character/MyraPawnExtensionComponent.h"
#include "AbilitySystem/MyraAbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Character/MyraInputComponent.h"
#include "Character/MyraPlayerState.h"
#include "Character/MyraCharacter.h"
#include "Components/InputComponent.h"
#include "DataAssets/MyraAbilitySet.h"
#include "DataAssets/MyraPawnData.h"
#include "AttributeSet.h"
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
	HandlePawnUninitialized();

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

void UMyraPawnExtensionComponent::HandlePawnUninitialized()
{
	RemovePawnData();
	RemoveInputBindings();
	RemoveInputMappings();

	bPawnReadyToInitialize = false;
	bAvatarReady = false;
	bInputMappingsApplied = false;
	bAbilityInputBound = false;

	CachedInputComponent.Reset();
	BoundInputHandles.Reset();
}

void UMyraPawnExtensionComponent::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (CachedInputComponent.Get() != PlayerInputComponent)
	{
		RemoveInputBindings();

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
}

void UMyraPawnExtensionComponent::ApplyPawnData()
{
	if (!PawnData || bPawnDataApplied)
	{
		return;
	}

	UMyraAbilitySystemComponent* ASC = GetMyraAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	if (GetOwner()->HasAuthority())
	{
		for (UMyraAbilitySet* AbilitySet : PawnData->AbilitySets)
		{
			if (AbilitySet)
			{
				AbilitySet->GiveToAbilitySystem(
					ASC,
					GetOwner(),
					AppliedPawnDataAbilityHandles,
					AppliedPawnDataEffectHandles,
					AppliedPawnDataAttributeSetHandles);
			}
		}

		if (PawnData->DefaultAttributeInitEffect)
		{
			const FActiveGameplayEffectHandle InitEffectHandle =
				ASC->ApplyInitializationEffectOnce(PawnData->DefaultAttributeInitEffect, 1.f, PawnData);
			if (InitEffectHandle.IsValid())
			{
				AppliedPawnDataEffectHandles.Add(InitEffectHandle);
			}
		}

		bPawnDataApplied = true;
	}
}

void UMyraPawnExtensionComponent::RemovePawnData()
{
	if (!bPawnDataApplied)
	{
		return;
	}

	UMyraAbilitySystemComponent* ASC = GetMyraAbilitySystemComponent();
	if (ASC && GetOwner()->HasAuthority())
	{
		for (const FGameplayAbilitySpecHandle& AbilityHandle : AppliedPawnDataAbilityHandles)
		{
			if (AbilityHandle.IsValid())
			{
				ASC->ClearAbility(AbilityHandle);
			}
		}

		for (const FActiveGameplayEffectHandle& EffectHandle : AppliedPawnDataEffectHandles)
		{
			ASC->RemoveTrackedGameplayEffect(EffectHandle);
		}

		for (const TWeakObjectPtr<UAttributeSet>& AttributeSetHandle : AppliedPawnDataAttributeSetHandles)
		{
			if (UAttributeSet* AttributeSet = AttributeSetHandle.Get())
			{
				ASC->RemoveSpawnedAttribute(AttributeSet);
			}
		}
	}

	AppliedPawnDataAbilityHandles.Reset();
	AppliedPawnDataEffectHandles.Reset();
	AppliedPawnDataAttributeSetHandles.Reset();
	bPawnDataApplied = false;
}

void UMyraPawnExtensionComponent::RemoveInputBindings()
{
	if (UEnhancedInputComponent* InputComponent = CachedInputComponent.Get())
	{
		for (const uint32 BindingHandle : BoundInputHandles)
		{
			InputComponent->RemoveBindingByHandle(BindingHandle);
		}
	}
}

void UMyraPawnExtensionComponent::RemoveInputMappings()
{
	if (!bInputMappingsApplied || !PawnData)
	{
		return;
	}

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
	//CLADUE: 
	if (const APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		// Only check the PlayerState ASC if the Character actually wants to use it
		if (const AMyraCharacter* MyraChar = Cast<AMyraCharacter>(Pawn))
		{
			if (MyraChar->bUsePlayerStateASC)
			{
				if (const AMyraPlayerState* PS = Pawn->GetPlayerState<AMyraPlayerState>())
				{
					return PS->GetMyraAbilitySystemComponent();
				}
			}
		}
	}

	// Fall back to whatever the pawn itself exposes via the interface
	if (const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(GetOwner()))
	{
		return Cast<UMyraAbilitySystemComponent>(ASI->GetAbilitySystemComponent());
	}

	return nullptr;

	// Replaced by above code
	//if (const APawn* Pawn = Cast<APawn>(GetOwner()))
	//{
	//	if (const AMyraPlayerState* PS = Pawn->GetPlayerState<AMyraPlayerState>())
	//	{
	//		return PS->GetMyraAbilitySystemComponent();
	//	}
	//}

	//if (const IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(GetOwner()))
	//{
	//	if (UMyraAbilitySystemComponent* ASC =
	//		Cast<UMyraAbilitySystemComponent>(AbilitySystemInterface->GetAbilitySystemComponent()))
	//	{
	//		return ASC;
	//	}
	//}	

	//return nullptr;
}
