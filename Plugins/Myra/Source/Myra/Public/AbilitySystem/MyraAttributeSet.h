// Copyright Myra . All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/MyraDefaultAttributeSet.h"
#include "MyraAttributeSet.generated.h"

/**
 * Backwards-compatible wrapper. New code should use UMyraDefaultAttributeSet.
 */
UCLASS()
class MYRA_API UMyraAttributeSet : public UMyraDefaultAttributeSet
{
	GENERATED_BODY()

public:

	UMyraAttributeSet();
};
