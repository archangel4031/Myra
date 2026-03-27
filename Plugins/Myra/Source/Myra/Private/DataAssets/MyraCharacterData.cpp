// Copyright Myra . All Rights Reserved.

#include "DataAssets/MyraCharacterData.h"

FPrimaryAssetId UMyraCharacterData::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(FPrimaryAssetType("MyraCharacterData"), GetFName());
}
