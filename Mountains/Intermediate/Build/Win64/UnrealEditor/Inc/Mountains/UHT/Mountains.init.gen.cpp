// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeMountains_init() {}
	static FPackageRegistrationInfo Z_Registration_Info_UPackage__Script_Mountains;
	FORCENOINLINE UPackage* Z_Construct_UPackage__Script_Mountains()
	{
		if (!Z_Registration_Info_UPackage__Script_Mountains.OuterSingleton)
		{
			static const UECodeGen_Private::FPackageParams PackageParams = {
				"/Script/Mountains",
				nullptr,
				0,
				PKG_CompiledIn | 0x00000000,
				0x58E323E6,
				0x7835DE90,
				METADATA_PARAMS(0, nullptr)
			};
			UECodeGen_Private::ConstructUPackage(Z_Registration_Info_UPackage__Script_Mountains.OuterSingleton, PackageParams);
		}
		return Z_Registration_Info_UPackage__Script_Mountains.OuterSingleton;
	}
	static FRegisterCompiledInInfo Z_CompiledInDeferPackage_UPackage__Script_Mountains(Z_Construct_UPackage__Script_Mountains, TEXT("/Script/Mountains"), Z_Registration_Info_UPackage__Script_Mountains, CONSTRUCT_RELOAD_VERSION_INFO(FPackageReloadVersionInfo, 0x58E323E6, 0x7835DE90));
PRAGMA_ENABLE_DEPRECATION_WARNINGS
