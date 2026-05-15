// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "Mountains/Public/ProceduralTerrain.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeProceduralTerrain() {}

// Begin Cross Module References
ENGINE_API UClass* Z_Construct_UClass_AActor();
MOUNTAINS_API UClass* Z_Construct_UClass_AProceduralTerrain();
MOUNTAINS_API UClass* Z_Construct_UClass_AProceduralTerrain_NoRegister();
PROCEDURALMESHCOMPONENT_API UClass* Z_Construct_UClass_UProceduralMeshComponent_NoRegister();
UPackage* Z_Construct_UPackage__Script_Mountains();
// End Cross Module References

// Begin Class AProceduralTerrain
void AProceduralTerrain::StaticRegisterNativesAProceduralTerrain()
{
}
IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(AProceduralTerrain);
UClass* Z_Construct_UClass_AProceduralTerrain_NoRegister()
{
	return AProceduralTerrain::StaticClass();
}
struct Z_Construct_UClass_AProceduralTerrain_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
		{ "IncludePath", "ProceduralTerrain.h" },
		{ "ModuleRelativePath", "Public/ProceduralTerrain.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_CustomMesh_MetaData[] = {
		{ "Category", "Terrain" },
		{ "EditInline", "true" },
		{ "ModuleRelativePath", "Public/ProceduralTerrain.h" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FObjectPropertyParams NewProp_CustomMesh;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<AProceduralTerrain>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UClass_AProceduralTerrain_Statics::NewProp_CustomMesh = { "CustomMesh", nullptr, (EPropertyFlags)0x00200800000a001d, UECodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AProceduralTerrain, CustomMesh), Z_Construct_UClass_UProceduralMeshComponent_NoRegister, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_CustomMesh_MetaData), NewProp_CustomMesh_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_AProceduralTerrain_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AProceduralTerrain_Statics::NewProp_CustomMesh,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_AProceduralTerrain_Statics::PropPointers) < 2048);
UObject* (*const Z_Construct_UClass_AProceduralTerrain_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_AActor,
	(UObject* (*)())Z_Construct_UPackage__Script_Mountains,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_AProceduralTerrain_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_AProceduralTerrain_Statics::ClassParams = {
	&AProceduralTerrain::StaticClass,
	"Engine",
	&StaticCppClassTypeInfo,
	DependentSingletons,
	nullptr,
	Z_Construct_UClass_AProceduralTerrain_Statics::PropPointers,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	0,
	UE_ARRAY_COUNT(Z_Construct_UClass_AProceduralTerrain_Statics::PropPointers),
	0,
	0x009000A4u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_AProceduralTerrain_Statics::Class_MetaDataParams), Z_Construct_UClass_AProceduralTerrain_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_AProceduralTerrain()
{
	if (!Z_Registration_Info_UClass_AProceduralTerrain.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_AProceduralTerrain.OuterSingleton, Z_Construct_UClass_AProceduralTerrain_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_AProceduralTerrain.OuterSingleton;
}
template<> MOUNTAINS_API UClass* StaticClass<AProceduralTerrain>()
{
	return AProceduralTerrain::StaticClass();
}
DEFINE_VTABLE_PTR_HELPER_CTOR(AProceduralTerrain);
AProceduralTerrain::~AProceduralTerrain() {}
// End Class AProceduralTerrain

// Begin Registration
struct Z_CompiledInDeferFile_FID_Users_maxse_Documents_Unreal_Projects_Mountains_Source_Mountains_Public_ProceduralTerrain_h_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_AProceduralTerrain, AProceduralTerrain::StaticClass, TEXT("AProceduralTerrain"), &Z_Registration_Info_UClass_AProceduralTerrain, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(AProceduralTerrain), 1296511723U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_maxse_Documents_Unreal_Projects_Mountains_Source_Mountains_Public_ProceduralTerrain_h_2368075671(TEXT("/Script/Mountains"),
	Z_CompiledInDeferFile_FID_Users_maxse_Documents_Unreal_Projects_Mountains_Source_Mountains_Public_ProceduralTerrain_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_maxse_Documents_Unreal_Projects_Mountains_Source_Mountains_Public_ProceduralTerrain_h_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// End Registration
PRAGMA_ENABLE_DEPRECATION_WARNINGS
