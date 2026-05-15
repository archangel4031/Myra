// Copyright Myra . All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagAssetInterface.h"
#include "Pawn/MyraPawnAbilityComponent.h"
#include "Pawn/MyraPawnAvatarInterface.h"
#include "MyraPawn.generated.h"

class UInputComponent;
class UMyraDefaultAttributeSet;
class UMyraPawnExtensionComponent;

/**
 * Generic Myra pawn base for non-character player avatars such as vehicles,
 * drones, turrets, and other controllable actors.
 */
UCLASS(Blueprintable)
class MYRA_API AMyraPawn : public APawn,
	public IAbilitySystemInterface,
	public IGameplayTagAssetInterface,
	public IMyraPawnAvatarInterface
{
	GENERATED_BODY()

public:

	AMyraPawn();

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
	FMyraOnPawnDeath OnDeathEvent;

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
	void OnDeath(AActor* Killer);
	virtual void OnDeath_Implementation(AActor* Killer);

	//~ Begin IMyraPawnAvatarInterface
	virtual void HandleMyraAbilitySystemInitialized() override;
	virtual void HandleMyraAbilitySystemUninitialized() override;
	virtual void HandleMyraHealthChanged(float OldValue, float NewValue) override;
	virtual bool ModifyMyraDamageBeforeApplication(float InDamage, float& OutDamage) override;
	virtual void HandleMyraDeath(AActor* Killer) override;
	virtual void HandleMyraGameplayEffectExecuted(const FMyraGEExecutedInfo& Info) override;
	//~ End IMyraPawnAvatarInterface
};
