// Copyright Myra . All Rights Reserved.

#include "DataAssets/MyraAbilitySet.h"
#include "AbilitySystem/MyraAbilitySystemComponent.h"
#include "AbilitySystem/MyraGameplayAbility.h"
#include "AttributeSet.h"
#include "GameplayEffect.h"

UMyraAbilitySet::UMyraAbilitySet()
{
}

void UMyraAbilitySet::GiveToAbilitySystem(
	UMyraAbilitySystemComponent* ASC,
	UObject* SourceObject,
	TArray<FGameplayAbilitySpecHandle>& OutAbilityHandles,
	TArray<FActiveGameplayEffectHandle>& OutEffectHandles) const
{
	if (!ASC)
	{
		UE_LOG(LogTemp, Error, TEXT("Myra: GiveToAbilitySystem called with null ASC on AbilitySet '%s'."), *GetName());
		return;
	}

	// Only the server grants abilities. They replicate automatically.
	check(ASC->IsOwnerActorAuthoritative());

	// ------------------------------------------
	//  1. Grant Attribute Sets
	//     Do this FIRST so they exist before abilities try to read them.
	// ------------------------------------------
	for (const FMyraAbilitySet_AttributeSet& Entry : GrantedAttributeSets)
	{
		if (!IsValid(Entry.AttributeSetClass))
		{
			UE_LOG(LogTemp, Warning, TEXT("Myra: AbilitySet '%s' has an invalid AttributeSet entry."), *GetName());
			continue;
		}

		// Don't add a duplicate — check whether this class is already present on the ASC.
		bool bAlreadyPresent = false;
		for (const UAttributeSet* ExistingSet : ASC->GetSpawnedAttributes())
		{
			if (ExistingSet && ExistingSet->GetClass() == Entry.AttributeSetClass)
			{
				bAlreadyPresent = true;
				break;
			}
		}
		if (bAlreadyPresent)
		{
			continue;
		}

		UAttributeSet* NewSet = NewObject<UAttributeSet>(ASC->GetOwner(), Entry.AttributeSetClass);
		ASC->AddAttributeSetSubobject(NewSet);
	}

	// ------------------------------------------
	//  2. Grant Gameplay Abilities
	// ------------------------------------------
	for (const FMyraAbilitySet_GameplayAbility& Entry : GrantedGameplayAbilities)
	{
		if (!IsValid(Entry.AbilityClass))
		{
			UE_LOG(LogTemp, Warning, TEXT("Myra: AbilitySet '%s' has an invalid Ability entry."), *GetName());
			continue;
		}

		UMyraGameplayAbility* AbilityCDO = Entry.AbilityClass->GetDefaultObject<UMyraGameplayAbility>();

		FGameplayAbilitySpec AbilitySpec(AbilityCDO, Entry.AbilityLevel);
		AbilitySpec.SourceObject = SourceObject;

		// Store the input tag in the DynamicAbilityTags so the input component
		// can bind to it via BindAbilityActivationToInputTag.
		if (Entry.InputTag.IsValid())
		{
			// Use the new getter to access the tag container
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(Entry.InputTag);
		}

		FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(AbilitySpec);
		OutAbilityHandles.Add(Handle);
	}

	// ------------------------------------------
	//  3. Apply startup Gameplay Effects
	// ------------------------------------------
	for (const FMyraAbilitySet_GameplayEffect& Entry : GrantedGameplayEffects)
	{
		if (!IsValid(Entry.GameplayEffectClass))
		{
			UE_LOG(LogTemp, Warning, TEXT("Myra: AbilitySet '%s' has an invalid GameplayEffect entry."), *GetName());
			continue;
		}

		FActiveGameplayEffectHandle Handle = ASC->ApplyEffectToSelf(Entry.GameplayEffectClass, Entry.EffectLevel);
		if (Handle.IsValid())
		{
			OutEffectHandles.Add(Handle);
		}
	}
}

FPrimaryAssetId UMyraAbilitySet::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(FPrimaryAssetType("MyraAbilitySet"), GetFName());
}
