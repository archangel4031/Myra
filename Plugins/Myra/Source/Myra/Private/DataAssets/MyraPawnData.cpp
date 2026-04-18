// Copyright Myra . All Rights Reserved.

#include "DataAssets/MyraPawnData.h"

FPrimaryAssetId UMyraPawnData::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(FPrimaryAssetType("MyraPawnData"), GetFName());
}
