// Copyright Myra . All Rights Reserved.

#include "MyraEditor.h"
#include "MyraAttributeSetWizard.h"
#include "ToolMenus.h"
#include "IAssetTools.h"
#include "AssetToolsModule.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FMyraEditorModule"

void FMyraEditorModule::StartupModule()
{
	// Wait until the engine is fully loaded before touching menus.
	if (UToolMenus::IsToolMenuUIEnabled())
	{
		RegisterMenuExtensions();
	}
	else
	{
		UToolMenus::RegisterStartupCallback(
			FSimpleMulticastDelegate::FDelegate::CreateRaw(
				this, &FMyraEditorModule::RegisterMenuExtensions));
	}

	RegisterAssetTypeActions();
}

void FMyraEditorModule::ShutdownModule()
{
	UnregisterMenuExtensions();
	UnregisterAssetTypeActions();
}

// ------------------------------------------------
//  Menu Registration
// ------------------------------------------------

void FMyraEditorModule::RegisterMenuExtensions()
{
	UToolMenu* ToolsMenu = UToolMenus::Get()->ExtendMenu("MainFrame.MainMenu.Tools");
	if (!ToolsMenu)
	{
		return;
	}

	FToolMenuSection& Section = ToolsMenu->FindOrAddSection(
		"Myra",
		LOCTEXT("MyraSectionLabel", "Myra "));

	Section.AddMenuEntry(
		"OpenAttributeSetWizard",
		LOCTEXT("AttributeSetWizardLabel",   "Attribute Set Wizard..."),
		LOCTEXT("AttributeSetWizardTooltip", "Generate a new C++ Attribute Set class from a MyraAttributeDefinition data asset."),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Edit"),
		FUIAction(FExecuteAction::CreateRaw(this, &FMyraEditorModule::OpenAttributeSetWizard))
	);
}

void FMyraEditorModule::UnregisterMenuExtensions()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);
}

// ------------------------------------------------
//  Asset Type Actions
// ------------------------------------------------

void FMyraEditorModule::RegisterAssetTypeActions()
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>(
		"AssetTools").Get();

	// Register actions — currently just registers the classes so they appear in
	// right-click → Myra  category. Add FAssetTypeActions_MyraAbilitySet etc.
	// in separate files for full thumbnail/color customization.
}

void FMyraEditorModule::UnregisterAssetTypeActions()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>(
			"AssetTools").Get();

		for (const TSharedRef<IAssetTypeActions>& Action : RegisteredAssetTypeActions)
		{
			AssetTools.UnregisterAssetTypeActions(Action);
		}
	}
	RegisteredAssetTypeActions.Empty();
}

// ------------------------------------------------
//  Wizard Launch
// ------------------------------------------------

void FMyraEditorModule::OpenAttributeSetWizard()
{
	FMyraAttributeSetWizard::OpenWizard();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMyraEditorModule, MyraEditor)