// Copyright Myra . All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/MyraAbilitySystemComponent.h"
#include "Components/ActorComponent.h"
#include "MyraPawnAbilityComponent.generated.h"

class AController;
class APawn;
class UGameplayEffect;
class UMyraAbilitySet;
class UMyraDefaultAttributeSet;
class UMyraPawnExtensionComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FMyraOnPawnHealthChanged, APawn*, Pawn, float, OldValue, float, NewValue);

/**
 * Shared GAS bootstrap and state component for Myra pawn avatars.
 * Both AMyraPawn and AMyraCharacter use this so multiplayer and owned-ASC
 * initialization stays in one place.
 */
UCLASS(ClassGroup = "Myra", meta = (BlueprintSpawnableComponent),
	DisplayName = "Myra Pawn Ability Component")
class MYRA_API UMyraPawnAbilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UMyraPawnAbilityComponent();

	/** Convenience getter — finds this component on any actor. Returns null if not present. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Myra|Pawn",
		meta = (DefaultToSelf = "Actor"))
	static UMyraPawnAbilityComponent* FindPawnAbilityComponent(const AActor* Actor);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Myra|Pawn")
	UMyraAbilitySystemComponent* GetMyraAbilitySystemComponent() const { return ResolvedAbilitySystemComponent; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Myra|Attributes")
	const UMyraDefaultAttributeSet* GetBaseAttributeSet() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Myra|Attributes")
	float GetHealth() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Myra|Attributes")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Myra|Attributes")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Myra|State")
	bool IsAlive() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Myra|State")
	bool IsAbilitySystemInitialized() const { return bAbilitySystemInitialized; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Myra|State")
	bool IsUsingPlayerStateAbilitySystem() const;

	UFUNCTION(BlueprintCallable, Category = "Myra|Pawn")
	void HandlePawnBeginPlay();

	UFUNCTION(BlueprintCallable, Category = "Myra|Pawn")
	void HandlePawnPossessed(AController* NewController);

	UFUNCTION(BlueprintCallable, Category = "Myra|Pawn")
	void HandlePawnUnpossessed();

	UFUNCTION(BlueprintCallable, Category = "Myra|Pawn")
	void HandlePawnPlayerStateReplicated();

	UFUNCTION(BlueprintCallable, Category = "Myra|Pawn")
	void HandlePawnEndPlay();

	bool ModifyDamageBeforeApplication(float InDamage, float& OutDamage) const;

	/**
	 * If true (default), the active ASC is taken from AMyraPlayerState.
	 * Otherwise the pawn uses its own Ability System Component.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Myra|Configuration")
	bool bUsePlayerStateASC = true;

	/**
	 * AbilitySets to grant when this pawn initializes its own ASC.
	 * Ignored when PlayerState ASC is active.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Myra|Configuration",
		meta = (EditCondition = "!bUsePlayerStateASC"))
	TArray<TObjectPtr<UMyraAbilitySet>> DefaultAbilitySets;

	/**
	 * Gameplay Effect applied on init to set starting attribute values when this pawn
	 * owns the active ASC. Use only one attribute initialization path per pawn.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Myra|Configuration")
	TSubclassOf<UGameplayEffect> DefaultAttributeInitEffect;

	// The Gameplay Effect applied upon respawn (e.g., restores Health/Mana to max)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Myra|Death")
	TSubclassOf<class UGameplayEffect> RespawnGameplayEffect;

	// Call this to revive the pawn
	UFUNCTION(BlueprintCallable, Category = "Myra|Death")
	virtual void Respawn();

private:

	bool InitAbilitySystemForPlayerState();
	void InitAbilitySystemOwned();
	void FinalizeAbilitySystemInitialization();
	void ApplyDefaultAttributeInitEffect();
	void BindAttributeChangeCallbacks();
	void ClearAbilitySystemCallbacks();
	void ResetResolvedAbilitySystem(bool bNotifyAvatar);

	APawn* GetPawn() const;
	UMyraAbilitySystemComponent* GetOwnedAbilitySystemComponent() const;
	UMyraPawnExtensionComponent* GetPawnExtensionComponent() const;

	void HandleHealthChanged(const FOnAttributeChangeData& ChangeData);

	void HandleDeathTag(const FGameplayTag GameplayTag, int32 NewCount);

	UFUNCTION()
	void HandleGameplayEffectExecuted(const FMyraGEExecutedInfo& Info);

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Myra|Components",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMyraAbilitySystemComponent> ResolvedAbilitySystemComponent;

	bool bAbilitySystemInitialized = false;
};
