// Copyright Myra. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"

/**
 * MyraLog
 *
 * Custom log category for the Myra plugin.
 * Include this header anywhere you want to use LogMyra instead of LogMyra.
 *
 * Usage:
 *   #include "MyraLog.h"
 *
 *   UE_LOG(LogMyra, Log,     TEXT("Info message"));
 *   UE_LOG(LogMyra, Warning, TEXT("Warning message"));
 *   UE_LOG(LogMyra, Error,   TEXT("Error message"));
 *   UE_LOG(LogMyra, Verbose, TEXT("Verbose message"));
 */
MYRA_API DECLARE_LOG_CATEGORY_EXTERN(LogMyra, Log, All);
