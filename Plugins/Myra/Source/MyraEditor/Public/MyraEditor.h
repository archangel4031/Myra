// Copyright Myra . All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;

/**
 * FMyraEditorModule
 *
 * Editor-only module. Responsible for:
 *   - Registering the "Myra " Tools menu entry and toolbar button.
 *   - Registering custom asset type actions for MyraAbilitySet, MyraAttributeDefinition, etc.
 *   - Hosting the Attribute Set Wizard window.
 */
class FMyraEditorModule : public IModuleInterface
{
public:

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:

	/** Registers entries under Tools → Myra  in the editor menu bar. */
	void RegisterMenuExtensions();

	/** Unregisters menus and toolbar items on shutdown. */
	void UnregisterMenuExtensions();

	/** Registers custom asset categories and type actions for right-click → Create. */
	void RegisterAssetTypeActions();
	void UnregisterAssetTypeActions();

	/** Opens the Attribute Set Wizard window. Called when the menu item is clicked. */
	void OpenAttributeSetWizard();

	/** Registered asset type actions (kept so we can unregister them on shutdown). */
	TArray<TSharedRef<class IAssetTypeActions>> RegisteredAssetTypeActions;

	/** Handle for the menu extension so we can remove it later. */
	TSharedPtr<class FExtender> MenuExtender;
};
