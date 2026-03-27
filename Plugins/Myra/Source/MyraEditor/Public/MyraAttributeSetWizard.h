// Copyright Myra . All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class UMyraAttributeDefinition;
class SWindow;
class STextBlock;
struct FAssetData;

/**
 * FMyraAttributeSetWizard
 *
 * Static entry point that opens the Attribute Set Wizard as a modal dialog.
 * Call FMyraAttributeSetWizard::OpenWizard() from the menu entry callback.
 */
class FMyraAttributeSetWizard
{
public:
	static void OpenWizard();
};

/**
 * SMyraAttributeSetWizard
 *
 * The Slate widget that lives inside the wizard window.
 * Displays:
 *   - A MyraAttributeDefinition asset picker (to select the definition to generate from)
 *   - Fixed output location info for the Myra module
 *   - An "Overwrite existing" checkbox
 *   - A Generate button
 *   - A result log text area
 */
class SMyraAttributeSetWizard : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SMyraAttributeSetWizard) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:

	// ------------------------------------------------
	//  UI Actions
	// ------------------------------------------------

	FReply OnGenerateClicked();

	void OnDefinitionAssetSelected(const FAssetData& AssetData);

	// ------------------------------------------------
	//  Helpers
	// ------------------------------------------------

	/** Appends a line to the result log text. */
	void AppendLog(const FString& Message, bool bIsError = false);

	/** Clears the result log. */
	void ClearLog();

	// ------------------------------------------------
	//  State
	// ------------------------------------------------

	/** Currently selected attribute definition asset. */
	TObjectPtr<UMyraAttributeDefinition> SelectedDefinition = nullptr;

	/** Overwrite existing files checkbox state. */
	bool bOverwriteExisting = false;

	/** Log display widget. */
	TSharedPtr<STextBlock> LogText;

	/** The parent window (so we can close it after success). */
	TWeakPtr<SWindow> ParentWindow;

	/** Accumulated log string. */
	FString LogString;
};
