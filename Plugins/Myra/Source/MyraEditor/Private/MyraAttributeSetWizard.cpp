// Copyright Myra . All Rights Reserved.

#include "MyraAttributeSetWizard.h"
#include "MyraAttributeSetGenerator.h"
#include "DataAssets/MyraAttributeDefinition.h"

#include "Widgets/SWindow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "PropertyCustomizationHelpers.h"
#include "Framework/Application/SlateApplication.h"

#define LOCTEXT_NAMESPACE "MyraAttributeSetWizard"

namespace MyraAttributeSetWizard
{
	const TCHAR* PublicOutputPath = TEXT("Plugins/Myra/Source/Myra/Public/AbilitySystem/CustomAttributeSets");
	const TCHAR* PrivateOutputPath = TEXT("Plugins/Myra/Source/Myra/Private/AbilitySystem/CustomAttributeSets");
}

// ============================================================
//  FMyraAttributeSetWizard — static launcher
// ============================================================

void FMyraAttributeSetWizard::OpenWizard()
{
	TSharedRef<SMyraAttributeSetWizard> WizardContent = SNew(SMyraAttributeSetWizard);

	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(LOCTEXT("WizardTitle", "Myra  — Attribute Set Wizard"))
		.SizingRule(ESizingRule::FixedSize)
		.ClientSize(FVector2D(600.f, 520.f))
		.SupportsMinimize(false)
		.SupportsMaximize(false)
		[
			WizardContent
		];

	// Give the widget a back-reference so it can close the window on success.
	// (Can't set before SWindow is created, so done here.)

	FSlateApplication::Get().AddModalWindow(Window, nullptr, /*bSlowTaskWindow=*/false);
}

// ============================================================
//  SMyraAttributeSetWizard — widget construction
// ============================================================

void SMyraAttributeSetWizard::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SBox).Padding(16.f)
		[
			SNew(SVerticalBox)

			// ── Title ────────────────────────────────────────────
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Instructions",
					"Select a Myra Attribute Definition asset, then click Generate. A new C++ AttributeSet class will be written to the Myra plugin.\n"
					"Header output: Plugins/Myra/Source/Myra/Public/AbilitySystem/CustomAttributeSets\n"
					"Source output: Plugins/Myra/Source/Myra/Private/AbilitySystem/CustomAttributeSets\n"
					"Recompile the project after generation to use the new class."))
				.AutoWrapText(true)
			]

			+ SVerticalBox::Slot().AutoHeight().Padding(0, 4)
			[ SNew(SSeparator) ]

			// ── Asset Picker ─────────────────────────────────────
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 6)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().VAlign(VAlign_Center).AutoWidth().Padding(0, 0, 8, 0)
				[ SNew(STextBlock).Text(LOCTEXT("DefinitionLabel", "Attribute Definition:")) ]

				+ SHorizontalBox::Slot().FillWidth(1.f)
				[
					// Object property entry box for selecting a UMyraAttributeDefinition asset
					SNew(SObjectPropertyEntryBox)
					.AllowedClass(UMyraAttributeDefinition::StaticClass())
					.OnObjectChanged(this, &SMyraAttributeSetWizard::OnDefinitionAssetSelected)
					.AllowClear(true)
				]
			]

			// ── Overwrite Checkbox ────────────────────────────────
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 6)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 8, 0)
				[
					SNew(SCheckBox)
					.IsChecked_Lambda([this]() { return bOverwriteExisting ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
					.OnCheckStateChanged_Lambda([this](ECheckBoxState State)
					{
						bOverwriteExisting = (State == ECheckBoxState::Checked);
					})
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[ SNew(STextBlock).Text(LOCTEXT("OverwriteLabel", "Overwrite existing files")) ]
			]

			+ SVerticalBox::Slot().AutoHeight().Padding(0, 4)
			[ SNew(SSeparator) ]

			// ── Generate Button ───────────────────────────────────
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right).Padding(0, 8)
			[
				SNew(SButton)
				.Text(LOCTEXT("GenerateBtn", "Generate Attribute Set"))
				.ButtonColorAndOpacity(FLinearColor(0.1f, 0.5f, 0.2f))
				.OnClicked(this, &SMyraAttributeSetWizard::OnGenerateClicked)
			]

			// ── Log Output ────────────────────────────────────────
			+ SVerticalBox::Slot().FillHeight(1.f).Padding(0, 4)
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					SAssignNew(LogText, STextBlock)
					.Text(LOCTEXT("LogPlaceholder", "Generation log will appear here..."))
					.AutoWrapText(true)
					.ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f))
				]
			]
		]
	];
}

// ============================================================
//  Actions
// ============================================================

FReply SMyraAttributeSetWizard::OnGenerateClicked()
{
	ClearLog();

	if (!SelectedDefinition)
	{
		AppendLog(TEXT("Please select a Myra Attribute Definition asset first."), /*bError=*/true);
		return FReply::Handled();
	}

	AppendLog(FString::Printf(TEXT("Generating attribute set for: %s"), *SelectedDefinition->AttributeSetName.ToString()));
	AppendLog(FString::Printf(TEXT("Header output: %s"), MyraAttributeSetWizard::PublicOutputPath));
	AppendLog(FString::Printf(TEXT("Source output: %s"), MyraAttributeSetWizard::PrivateOutputPath));

	FMyraGenerationResult GenResult = FMyraAttributeSetGenerator::GenerateAttributeSet(SelectedDefinition, bOverwriteExisting);

	for (const FString& Msg : GenResult.Messages)
	{
		AppendLog(Msg, Msg.StartsWith(TEXT("[ERROR]")));
	}

	if (GenResult.bSuccess)
	{
		AppendLog(TEXT(""));
		AppendLog(TEXT("SUCCESS — files written to disk."));
		AppendLog(TEXT("Next step: build the Editor target once so UnrealHeaderTool registers the new UCLASS."));
		AppendLog(TEXT("Live Coding usually will not discover brand-new reflected headers/files."));
		AppendLog(TEXT("Recommended: close the editor, build Development Editor in Visual Studio or Rider, then reopen."));
		AppendLog(TEXT("After that, the new AttributeSet class will appear in class pickers and data assets."));
	}

	return FReply::Handled();
}

void SMyraAttributeSetWizard::OnDefinitionAssetSelected(const FAssetData& AssetData)
{
	SelectedDefinition = Cast<UMyraAttributeDefinition>(AssetData.GetAsset());
	if (SelectedDefinition)
	{
		AppendLog(FString::Printf(TEXT("Selected definition: %s (AttributeSetName: '%s', %d attributes)"),
			*AssetData.AssetName.ToString(),
			*SelectedDefinition->AttributeSetName.ToString(),
			SelectedDefinition->Attributes.Num()));
	}
}

// ============================================================
//  Helpers
// ============================================================

void SMyraAttributeSetWizard::AppendLog(const FString& Message, bool bIsError)
{
	const FString Prefix = bIsError ? TEXT("[✗] ") : TEXT("[✓] ");
	LogString += Prefix + Message + TEXT("\n");
	if (LogText.IsValid())
	{
		LogText->SetText(FText::FromString(LogString));
		LogText->SetColorAndOpacity(bIsError ? FLinearColor::Red : FLinearColor(0.8f, 0.9f, 0.8f));
	}
}

void SMyraAttributeSetWizard::ClearLog()
{
	LogString.Empty();
	if (LogText.IsValid())
	{
		LogText->SetText(FText::GetEmpty());
	}
}

#undef LOCTEXT_NAMESPACE
