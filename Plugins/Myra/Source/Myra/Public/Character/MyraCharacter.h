// Copyright Myra . All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagAssetInterface.h"
#include "Pawn/MyraPawnAbilityComponent.h"
#include "Pawn/MyraPawnAvatarInterface.h"
#include "MyraCharacter.generated.h"

class UInputComponent;
class UMyraDefaultAttributeSet;
class UMyraPawnExtensionComponent;

/**
 * Shared Myra character base.
 * Character-specific behavior lives here, while generic GAS initialization is
 * handled by UMyraPawnAbilityComponent so it can also be reused by AMyraPawn.
 */
UCLASS(Abstract)
class MYRA_API AMyraCharacter : public ACharacter,
	public IAbilitySystemInterface,
	public IGameplayTagAssetInterface,
	public IMyraPawnAvatarInterface
{
	GENERATED_BODY()

public:

	AMyraCharacter();

	//~ Begin IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~ End IAbilitySystemInterface

	//~ Begin IGameplayTagAssetInterface
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	//~ End IGameplayTagAssetInterface

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Myra|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMyraAbilitySystemComponent> OwnedAbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Myra|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMyraPawnAbilityComponent> PawnAbilityComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Myra|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMyraPawnExtensionComponent> PawnExtensionComponent;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Myra")
	virtual UMyraPawnAbilityComponent* GetMyraPawnAbilityComponent() const override { return PawnAbilityComponent; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Myra")
	virtual UMyraPawnExtensionComponent* GetMyraPawnExtensionComponent() const override { return PawnExtensionComponent; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Myra")
	virtual UMyraAbilitySystemComponent* GetMyraAbilitySystemComponent() const override;

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
	bool IsDead() const { return !IsAlive(); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Myra|State")
	bool IsUsingPlayerStateAbilitySystem() const;

	UFUNCTION(BlueprintNativeEvent, Category = "Myra|Damage")
	bool ModifyDamageBeforeApplication(float InDamage, float& OutDamage);
	virtual bool ModifyDamageBeforeApplication_Implementation(float InDamage, float& OutDamage);

	UPROPERTY(BlueprintAssignable, Category = "Myra|Events")
	FMyraOnPawnHealthChanged OnHealthChanged;

	UFUNCTION(BlueprintNativeEvent, Category = "Myra|Events")
	void OnGameplayEffectExecuted(const FMyraGEExecutedInfo& Info);
	virtual void OnGameplayEffectExecuted_Implementation(const FMyraGEExecutedInfo& Info);

protected:

	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	virtual void OnRep_PlayerState() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	virtual void OnAbilitySystemInitialized();

	UFUNCTION(BlueprintNativeEvent, Category = "Myra|Events")
	void OnDeath(APawn* DeadPawn);
	virtual void OnDeath_Implementation(APawn* DeadPawn);

	//~ Begin IMyraPawnAvatarInterface
	virtual void HandleMyraAbilitySystemInitialized() override;
	virtual void HandleMyraAbilitySystemUninitialized() override;
	virtual void HandleMyraHealthChanged(float OldValue, float NewValue) override;
	virtual bool ModifyMyraDamageBeforeApplication(float InDamage, float& OutDamage) override;
	virtual void HandleMyraDeath(APawn* DeadPawn) override;
	virtual void HandleMyraGameplayEffectExecuted(const FMyraGEExecutedInfo& Info) override;
	//~ End IMyraPawnAvatarInterface
};
