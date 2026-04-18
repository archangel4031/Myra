// Copyright Myra . All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MyraPawnExtensionComponent.generated.h"

class UEnhancedInputComponent;
class UInputComponent;
class UMyraAbilitySystemComponent;
class UMyraPawnData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMyraOnPawnReadyToInitialize);

/**
 * UMyraPawnExtensionComponent
 *
 * Lightweight Lyra-style pawn extension component.
 * It applies PawnData once the ASC avatar is ready, and it binds Enhanced Input
 * ability actions automatically once the local input component becomes available.
 */
UCLASS(ClassGroup = "Myra", meta = (BlueprintSpawnableComponent),
	DisplayName = "Myra Pawn Extension Component")
class MYRA_API UMyraPawnExtensionComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UMyraPawnExtensionComponent();

	/** Convenience getter — finds this component on any actor. Returns null if not present. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Myra |Pawn",
		meta = (DefaultToSelf = "Actor"))
	static UMyraPawnExtensionComponent* FindPawnExtensionComponent(const AActor* Actor);

	/** Generic pawn setup asset used to initialize this pawn. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Myra |Configuration")
	TObjectPtr<UMyraPawnData> PawnData;

	/** Call this when PlayerState replication becomes available on PlayerState-ASC pawns. */
	UFUNCTION(BlueprintCallable, Category = "Myra |Pawn")
	void HandlePlayerStateReplicated();

	/** Call this after InitAbilityActorInfo has assigned the pawn avatar on the ASC. */
	UFUNCTION(BlueprintCallable, Category = "Myra |Pawn")
	void HandleAvatarSet();

	/** Call this when controller ownership changes for a locally controlled pawn. */
	UFUNCTION(BlueprintCallable, Category = "Myra |Pawn")
	void HandleControllerChanged();

	/** Call this from SetupPlayerInputComponent so Myra can auto-bind ability input. */
	UFUNCTION(BlueprintCallable, Category = "Myra |Pawn")
	void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent);

	/** Returns true once PawnData has been applied and the pawn is ready. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Myra |Pawn")
	bool IsReadyToInitialize() const { return bPawnReadyToInitialize; }

	/** Returns the typed ASC from the owning pawn or its PlayerState. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Myra |Pawn")
	UMyraAbilitySystemComponent* GetMyraAbilitySystemComponent() const;

	UPROPERTY(BlueprintAssignable, Category = "Myra |Pawn")
	FMyraOnPawnReadyToInitialize OnPawnReadyToInitialize;

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	void CheckPawnReadyToInitialize();
	void ApplyPawnData();
	void TryInitializePlayerInput();

	bool bPawnReadyToInitialize = false;
	bool bAvatarReady = false;
	bool bInputMappingsApplied = false;
	bool bAbilityInputBound = false;

	TWeakObjectPtr<UEnhancedInputComponent> CachedInputComponent;
	TArray<uint32> BoundInputHandles;
};
