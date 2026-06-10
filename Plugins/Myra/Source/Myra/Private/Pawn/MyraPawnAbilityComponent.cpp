// Copyright Myra . All Rights Reserved.

#include "Pawn/MyraPawnAbilityComponent.h"
#include "AbilitySystem/MyraDefaultAttributeSet.h"
#include "Character/MyraPawnExtensionComponent.h"
#include "Character/MyraPlayerState.h"
#include "DataAssets/MyraAbilitySet.h"
#include "GameFramework/Pawn.h"
#include "Pawn/MyraPawnAvatarInterface.h"
#include "Tags/MyraNativeGameplayTags.h"
#include "Logging/MyraLog.h"

UMyraPawnAbilityComponent::UMyraPawnAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(false);
}

UMyraPawnAbilityComponent* UMyraPawnAbilityComponent::FindPawnAbilityComponent(const AActor* Actor)
{
	if (!Actor)
	{
		return nullptr;
	}

	return Actor->FindComponentByClass<UMyraPawnAbilityComponent>();
}

const UMyraDefaultAttributeSet* UMyraPawnAbilityComponent::GetBaseAttributeSet() const
{
	if (ResolvedAbilitySystemComponent)
	{
		return ResolvedAbilitySystemComponent->GetSet<UMyraDefaultAttributeSet>();
	}

	return nullptr;
}

float UMyraPawnAbilityComponent::GetHealth() const
{
	if (const UMyraDefaultAttributeSet* AttributeSet = GetBaseAttributeSet())
	{
		return AttributeSet->GetHealth();
	}

	return 0.f;
}

float UMyraPawnAbilityComponent::GetMaxHealth() const
{
	if (const UMyraDefaultAttributeSet* AttributeSet = GetBaseAttributeSet())
	{
		return AttributeSet->GetMaxHealth();
	}

	return 1.f;
}

float UMyraPawnAbilityComponent::GetHealthPercent() const
{
	const float MaxHealth = GetMaxHealth();
	return MaxHealth > 0.f ? (GetHealth() / MaxHealth) : 0.f;
}

bool UMyraPawnAbilityComponent::IsAlive() const
{
	return GetHealth() > 0.f;
}

bool UMyraPawnAbilityComponent::IsUsingPlayerStateAbilitySystem() const
{
	if (!bUsePlayerStateASC || !ResolvedAbilitySystemComponent)
	{
		return false;
	}

	const APawn* Pawn = GetPawn();
	const AMyraPlayerState* MyraPlayerState = Pawn ? Pawn->GetPlayerState<AMyraPlayerState>() : nullptr;
	return MyraPlayerState && ResolvedAbilitySystemComponent == MyraPlayerState->GetMyraAbilitySystemComponent();
}

void UMyraPawnAbilityComponent::HandlePawnBeginPlay()
{
	if (bUsePlayerStateASC)
	{
		if (UMyraAbilitySystemComponent* OwnedAbilitySystemComponent = GetOwnedAbilitySystemComponent())
		{
			OwnedAbilitySystemComponent->SetIsReplicated(false);
		}
	}

	if (!bUsePlayerStateASC && !bAbilitySystemInitialized)
	{
		InitAbilitySystemOwned();
	}
}

void UMyraPawnAbilityComponent::HandlePawnPossessed(AController* NewController)
{
	if (bUsePlayerStateASC && InitAbilitySystemForPlayerState())
	{
		return;
	}

	InitAbilitySystemOwned();
}

void UMyraPawnAbilityComponent::HandlePawnUnpossessed()
{
	if (IsUsingPlayerStateAbilitySystem())
	{
		ResetResolvedAbilitySystem(true);
	}
}

void UMyraPawnAbilityComponent::HandlePawnPlayerStateReplicated()
{
	if (bUsePlayerStateASC && InitAbilitySystemForPlayerState())
	{
		return;
	}

	if (!bAbilitySystemInitialized)
	{
		InitAbilitySystemOwned();
	}
}

void UMyraPawnAbilityComponent::HandlePawnEndPlay()
{
	ResetResolvedAbilitySystem(true);
}

bool UMyraPawnAbilityComponent::ModifyDamageBeforeApplication(float InDamage, float& OutDamage) const
{
	OutDamage = InDamage;

	if (IMyraPawnAvatarInterface* AvatarInterface = Cast<IMyraPawnAvatarInterface>(GetOwner()))
	{
		return AvatarInterface->ModifyMyraDamageBeforeApplication(InDamage, OutDamage);
	}

	return false;
}

void UMyraPawnAbilityComponent::Respawn()
{
	UMyraAbilitySystemComponent* ASC = GetMyraAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	// Remove the Dead tag. 
	// Because HandleDeathTag listens for NewOrRemoved, it will fire again here.
	// Check inside HandleDeathTag if the tag was ADDED or REMOVED to split Death/Revive logic.
	ASC->RemoveLooseGameplayTag(MyraGameplayTags::Myra_State_Dead);

	// Apply the Respawn Gameplay Effect
	if (RespawnGameplayEffect)
	{
		// Create the context (who is causing this effect? In this case, the pawn itself)
		FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
		ContextHandle.AddInstigator(GetOwner(), GetOwner());

		// Generate the spec handle using the class, a level (1.0f), and the context
		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(RespawnGameplayEffect, 1.0f, ContextHandle);

		if (SpecHandle.IsValid())
		{
			// Apply it to ourselves
			ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
	else
	{
		UE_LOG(LogMyra, Warning, TEXT("MyraPawnAbilityComponent: RespawnGameplayEffect is not assigned!"));
	}
}

bool UMyraPawnAbilityComponent::InitAbilitySystemForPlayerState()
{
	APawn* Pawn = GetPawn();
	AMyraPlayerState* MyraPlayerState = Pawn ? Pawn->GetPlayerState<AMyraPlayerState>() : nullptr;
	if (!MyraPlayerState)
	{
		return false;
	}

	UMyraAbilitySystemComponent* NewResolvedAbilitySystemComponent = MyraPlayerState->GetMyraAbilitySystemComponent();
	if (!NewResolvedAbilitySystemComponent)
	{
		return false;
	}

	if (ResolvedAbilitySystemComponent != NewResolvedAbilitySystemComponent)
	{
		ResetResolvedAbilitySystem(true);
		ResolvedAbilitySystemComponent = NewResolvedAbilitySystemComponent;
	}

	ResolvedAbilitySystemComponent->InitAbilityActorInfo(MyraPlayerState, Pawn);

	if (Pawn && Pawn->HasAuthority())
	{
		MyraPlayerState->GrantDefaultAbilitySets();
	}

	if (UMyraPawnExtensionComponent* PawnExtensionComponent = GetPawnExtensionComponent())
	{
		PawnExtensionComponent->HandlePlayerStateReplicated();
		PawnExtensionComponent->HandleAvatarSet();
	}

	FinalizeAbilitySystemInitialization();
	return true;
}

void UMyraPawnAbilityComponent::InitAbilitySystemOwned()
{
	UMyraAbilitySystemComponent* OwnedAbilitySystemComponent = GetOwnedAbilitySystemComponent();
	APawn* Pawn = GetPawn();
	if (!OwnedAbilitySystemComponent || !Pawn)
	{
		return;
	}

	if (ResolvedAbilitySystemComponent != OwnedAbilitySystemComponent)
	{
		ResetResolvedAbilitySystem(true);
		ResolvedAbilitySystemComponent = OwnedAbilitySystemComponent;
	}

	ResolvedAbilitySystemComponent->InitAbilityActorInfo(Pawn, Pawn);

	if (Pawn->HasAuthority())
	{
		for (UMyraAbilitySet* AbilitySet : DefaultAbilitySets)
		{
			if (AbilitySet)
			{
				ResolvedAbilitySystemComponent->GrantAbilitySet(AbilitySet, Pawn);
			}
		}
	}

	if (UMyraPawnExtensionComponent* PawnExtensionComponent = GetPawnExtensionComponent())
	{
		PawnExtensionComponent->HandleAvatarSet();
	}

	FinalizeAbilitySystemInitialization();
}

void UMyraPawnAbilityComponent::FinalizeAbilitySystemInitialization()
{
	if (!ResolvedAbilitySystemComponent || bAbilitySystemInitialized)
	{
		return;
	}

	bAbilitySystemInitialized = true;

	if (APawn* Pawn = GetPawn(); Pawn && Pawn->HasAuthority())
	{
		ApplyDefaultAttributeInitEffect();
	}

	BindAttributeChangeCallbacks();

	if (IMyraPawnAvatarInterface* AvatarInterface = Cast<IMyraPawnAvatarInterface>(GetOwner()))
	{
		AvatarInterface->HandleMyraAbilitySystemInitialized();
	}
}

void UMyraPawnAbilityComponent::ApplyDefaultAttributeInitEffect()
{
	if (!IsUsingPlayerStateAbilitySystem() && DefaultAttributeInitEffect && ResolvedAbilitySystemComponent)
	{
		ResolvedAbilitySystemComponent->ApplyInitializationEffectOnce(DefaultAttributeInitEffect, 1.f, this);
	}
}

void UMyraPawnAbilityComponent::BindAttributeChangeCallbacks()
{
	if (!ResolvedAbilitySystemComponent)
	{
		return;
	}

	ResolvedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		UMyraDefaultAttributeSet::GetHealthAttribute())
		.RemoveAll(this);
	ResolvedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		UMyraDefaultAttributeSet::GetHealthAttribute())
		.AddUObject(this, &UMyraPawnAbilityComponent::HandleHealthChanged);

	ResolvedAbilitySystemComponent->RegisterGameplayTagEvent(
		MyraGameplayTags::Myra_State_Dead,
		EGameplayTagEventType::NewOrRemoved)
		.RemoveAll(this);
	ResolvedAbilitySystemComponent->RegisterGameplayTagEvent(
		MyraGameplayTags::Myra_State_Dead,
		EGameplayTagEventType::NewOrRemoved)
		.AddUObject(this, &UMyraPawnAbilityComponent::HandleDeathTag);

	ResolvedAbilitySystemComponent->OnGameplayEffectAttributeExecuted.RemoveDynamic(
		this, &UMyraPawnAbilityComponent::HandleGameplayEffectExecuted);
	ResolvedAbilitySystemComponent->OnGameplayEffectAttributeExecuted.AddDynamic(
		this, &UMyraPawnAbilityComponent::HandleGameplayEffectExecuted);
}

void UMyraPawnAbilityComponent::ClearAbilitySystemCallbacks()
{
	if (!ResolvedAbilitySystemComponent)
	{
		return;
	}

	ResolvedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		UMyraDefaultAttributeSet::GetHealthAttribute())
		.RemoveAll(this);
	ResolvedAbilitySystemComponent->RegisterGameplayTagEvent(
		MyraGameplayTags::Myra_State_Dead,
		EGameplayTagEventType::NewOrRemoved)
		.RemoveAll(this);
	ResolvedAbilitySystemComponent->OnGameplayEffectAttributeExecuted.RemoveDynamic(
		this, &UMyraPawnAbilityComponent::HandleGameplayEffectExecuted);
}

void UMyraPawnAbilityComponent::ResetResolvedAbilitySystem(bool bNotifyAvatar)
{
	ClearAbilitySystemCallbacks();

	if (bNotifyAvatar)
	{
		if (UMyraPawnExtensionComponent* PawnExtensionComponent = GetPawnExtensionComponent())
		{
			PawnExtensionComponent->HandlePawnUninitialized();
		}
	}

	ResolvedAbilitySystemComponent = nullptr;
	bAbilitySystemInitialized = false;

	if (bNotifyAvatar)
	{
		if (IMyraPawnAvatarInterface* AvatarInterface = Cast<IMyraPawnAvatarInterface>(GetOwner()))
		{
			AvatarInterface->HandleMyraAbilitySystemUninitialized();
		}
	}
}

APawn* UMyraPawnAbilityComponent::GetPawn() const
{
	return Cast<APawn>(GetOwner());
}

UMyraAbilitySystemComponent* UMyraPawnAbilityComponent::GetOwnedAbilitySystemComponent() const
{
	return GetOwner() ? GetOwner()->FindComponentByClass<UMyraAbilitySystemComponent>() : nullptr;
}

UMyraPawnExtensionComponent* UMyraPawnAbilityComponent::GetPawnExtensionComponent() const
{
	return UMyraPawnExtensionComponent::FindPawnExtensionComponent(GetOwner());
}

void UMyraPawnAbilityComponent::HandleHealthChanged(const FOnAttributeChangeData& ChangeData)
{
	if (IMyraPawnAvatarInterface* AvatarInterface = Cast<IMyraPawnAvatarInterface>(GetOwner()))
	{
		AvatarInterface->HandleMyraHealthChanged(ChangeData.OldValue, ChangeData.NewValue);
	}
}

void UMyraPawnAbilityComponent::HandleDeathTag(const FGameplayTag GameplayTag, int32 NewCount)
{
	if (NewCount <= 0 || IsAlive())
	{
		return;
	}

	if (IsUsingPlayerStateAbilitySystem())
	{
		if (UMyraPawnExtensionComponent* PawnExtensionComponent = GetPawnExtensionComponent())
		{
			PawnExtensionComponent->HandlePawnUninitialized();
		}
	}

	if (IMyraPawnAvatarInterface* AvatarInterface = Cast<IMyraPawnAvatarInterface>(GetOwner()))
	{
		AvatarInterface->HandleMyraDeath(nullptr);
	}
}

void UMyraPawnAbilityComponent::HandleGameplayEffectExecuted(const FMyraGEExecutedInfo& Info)
{
	if (IMyraPawnAvatarInterface* AvatarInterface = Cast<IMyraPawnAvatarInterface>(GetOwner()))
	{
		AvatarInterface->HandleMyraGameplayEffectExecuted(Info);
	}
}
