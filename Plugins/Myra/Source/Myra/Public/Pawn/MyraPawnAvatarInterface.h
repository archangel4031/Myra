// Copyright Myra . All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MyraPawnAvatarInterface.generated.h"

class UMyraAbilitySystemComponent;
class UMyraPawnAbilityComponent;
class UMyraPawnExtensionComponent;
struct FMyraGEExecutedInfo;

UINTERFACE(BlueprintType)
class MYRA_API UMyraPawnAvatarInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Native interface implemented by Myra pawn avatars.
 * Shared systems use this to notify the owning pawn/character about GAS events
 * without depending on a specific actor class.
 */
class MYRA_API IMyraPawnAvatarInterface
{
	GENERATED_BODY()

public:

	virtual UMyraAbilitySystemComponent* GetMyraAbilitySystemComponent() const = 0;
	virtual UMyraPawnAbilityComponent* GetMyraPawnAbilityComponent() const = 0;
	virtual UMyraPawnExtensionComponent* GetMyraPawnExtensionComponent() const = 0;

	virtual void HandleMyraAbilitySystemInitialized() = 0;
	virtual void HandleMyraAbilitySystemUninitialized() = 0;
	virtual void HandleMyraHealthChanged(float OldValue, float NewValue) = 0;
	virtual bool ModifyMyraDamageBeforeApplication(float InDamage, float& OutDamage) = 0;
	virtual void HandleMyraDeath(AActor* Killer) = 0;
	virtual void HandleMyraGameplayEffectExecuted(const FMyraGEExecutedInfo& Info) = 0;
};
