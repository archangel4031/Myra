// Copyright Myra . All Rights Reserved.

#include "DataAssets/MyraAttributeDefinition.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

FPrimaryAssetId UMyraAttributeDefinition::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(FPrimaryAssetType("MyraAttributeDefinition"), GetFName());
}

#if WITH_EDITOR
EDataValidationResult UMyraAttributeDefinition::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	// Validate the AttributeSet name
	const FString NameStr = AttributeSetName.ToString();
	if (NameStr.IsEmpty())
	{
		Context.AddError(NSLOCTEXT("Myra", "EmptyAttributeSetName",
			"AttributeSetName must not be empty."));
		Result = EDataValidationResult::Invalid;
	}
	else
	{
		// Must start with a letter
		if (!FChar::IsAlpha(NameStr[0]))
		{
			Context.AddError(FText::Format(
				NSLOCTEXT("Myra", "InvalidAttributeSetNameStart",
					"AttributeSetName '{0}' must start with a letter."),
				FText::FromString(NameStr)));
			Result = EDataValidationResult::Invalid;
		}

		// Must be alphanumeric only
		for (TCHAR Char : NameStr)
		{
			if (!FChar::IsAlnum(Char) && Char != '_')
			{
				Context.AddError(FText::Format(
					NSLOCTEXT("Myra", "InvalidAttributeSetNameChars",
						"AttributeSetName '{0}' must only contain letters, numbers, and underscores."),
					FText::FromString(NameStr)));
				Result = EDataValidationResult::Invalid;
				break;
			}
		}
	}

	// Validate individual attributes
	TSet<FName> SeenNames;
	for (int32 i = 0; i < Attributes.Num(); ++i)
	{
		const FMyraAttributeDefinitionEntry& Entry = Attributes[i];
		const FString EntryName = Entry.AttributeName.ToString();

		if (EntryName.IsEmpty())
		{
			Context.AddError(FText::Format(
				NSLOCTEXT("Myra", "EmptyAttributeName",
					"Attribute at index {0} has an empty name."),
				FText::AsNumber(i)));
			Result = EDataValidationResult::Invalid;
			continue;
		}

		if (!FChar::IsAlpha(EntryName[0]))
		{
			Context.AddError(FText::Format(
				NSLOCTEXT("Myra", "InvalidAttributeNameStart",
					"Attribute '{0}' must start with a letter."),
				FText::FromName(Entry.AttributeName)));
			Result = EDataValidationResult::Invalid;
		}

		for (TCHAR Char : EntryName)
		{
			if (!FChar::IsAlnum(Char) && Char != '_')
			{
				Context.AddError(FText::Format(
					NSLOCTEXT("Myra", "InvalidAttributeNameChars",
						"Attribute '{0}' must only contain letters, numbers, and underscores."),
					FText::FromName(Entry.AttributeName)));
				Result = EDataValidationResult::Invalid;
				break;
			}
		}

		if (SeenNames.Contains(Entry.AttributeName))
		{
			Context.AddError(FText::Format(
				NSLOCTEXT("Myra", "DuplicateAttributeName",
					"Duplicate attribute name '{0}'."),
				FText::FromName(Entry.AttributeName)));
			Result = EDataValidationResult::Invalid;
		}
		SeenNames.Add(Entry.AttributeName);
	}

	return Result == EDataValidationResult::NotValidated
		? EDataValidationResult::Valid
		: Result;
}
#endif
