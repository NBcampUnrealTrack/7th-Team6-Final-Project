// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "PartTimeBeatGameMode.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

void EmptyLinkFunctionForGeneratedCodePartTimeBeatGameMode() {}

// ********** Begin Cross Module References ********************************************************
ENGINE_API UClass* Z_Construct_UClass_AGameModeBase();
PARTTIMEBEAT_API UClass* Z_Construct_UClass_APartTimeBeatGameMode();
PARTTIMEBEAT_API UClass* Z_Construct_UClass_APartTimeBeatGameMode_NoRegister();
UPackage* Z_Construct_UPackage__Script_PartTimeBeat();
// ********** End Cross Module References **********************************************************

// ********** Begin Class APartTimeBeatGameMode ****************************************************
void APartTimeBeatGameMode::StaticRegisterNativesAPartTimeBeatGameMode()
{
}
FClassRegistrationInfo Z_Registration_Info_UClass_APartTimeBeatGameMode;
UClass* APartTimeBeatGameMode::GetPrivateStaticClass()
{
	using TClass = APartTimeBeatGameMode;
	if (!Z_Registration_Info_UClass_APartTimeBeatGameMode.InnerSingleton)
	{
		GetPrivateStaticClassBody(
			StaticPackage(),
			TEXT("PartTimeBeatGameMode"),
			Z_Registration_Info_UClass_APartTimeBeatGameMode.InnerSingleton,
			StaticRegisterNativesAPartTimeBeatGameMode,
			sizeof(TClass),
			alignof(TClass),
			TClass::StaticClassFlags,
			TClass::StaticClassCastFlags(),
			TClass::StaticConfigName(),
			(UClass::ClassConstructorType)InternalConstructor<TClass>,
			(UClass::ClassVTableHelperCtorCallerType)InternalVTableHelperCtorCaller<TClass>,
			UOBJECT_CPPCLASS_STATICFUNCTIONS_FORCLASS(TClass),
			&TClass::Super::StaticClass,
			&TClass::WithinClass::StaticClass
		);
	}
	return Z_Registration_Info_UClass_APartTimeBeatGameMode.InnerSingleton;
}
UClass* Z_Construct_UClass_APartTimeBeatGameMode_NoRegister()
{
	return APartTimeBeatGameMode::GetPrivateStaticClass();
}
struct Z_Construct_UClass_APartTimeBeatGameMode_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n *  Simple GameMode for a third person game\n */" },
#endif
		{ "HideCategories", "Info Rendering MovementReplication Replication Actor Input Movement Collision Rendering HLOD WorldPartition DataLayers Transformation" },
		{ "IncludePath", "PartTimeBeatGameMode.h" },
		{ "ModuleRelativePath", "PartTimeBeatGameMode.h" },
		{ "ShowCategories", "Input|MouseInput Input|TouchInput" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Simple GameMode for a third person game" },
#endif
	};
#endif // WITH_METADATA
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<APartTimeBeatGameMode>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
UObject* (*const Z_Construct_UClass_APartTimeBeatGameMode_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_AGameModeBase,
	(UObject* (*)())Z_Construct_UPackage__Script_PartTimeBeat,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_APartTimeBeatGameMode_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_APartTimeBeatGameMode_Statics::ClassParams = {
	&APartTimeBeatGameMode::StaticClass,
	"Game",
	&StaticCppClassTypeInfo,
	DependentSingletons,
	nullptr,
	nullptr,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	0,
	0,
	0,
	0x008003ADu,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_APartTimeBeatGameMode_Statics::Class_MetaDataParams), Z_Construct_UClass_APartTimeBeatGameMode_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_APartTimeBeatGameMode()
{
	if (!Z_Registration_Info_UClass_APartTimeBeatGameMode.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_APartTimeBeatGameMode.OuterSingleton, Z_Construct_UClass_APartTimeBeatGameMode_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_APartTimeBeatGameMode.OuterSingleton;
}
DEFINE_VTABLE_PTR_HELPER_CTOR(APartTimeBeatGameMode);
APartTimeBeatGameMode::~APartTimeBeatGameMode() {}
// ********** End Class APartTimeBeatGameMode ******************************************************

// ********** Begin Registration *******************************************************************
struct Z_CompiledInDeferFile_FID_Users_dignw_Documents_GitHub_7th_Team6_Final_Project_PartTimeBeat_Source_PartTimeBeat_PartTimeBeatGameMode_h__Script_PartTimeBeat_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_APartTimeBeatGameMode, APartTimeBeatGameMode::StaticClass, TEXT("APartTimeBeatGameMode"), &Z_Registration_Info_UClass_APartTimeBeatGameMode, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(APartTimeBeatGameMode), 3393485702U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_dignw_Documents_GitHub_7th_Team6_Final_Project_PartTimeBeat_Source_PartTimeBeat_PartTimeBeatGameMode_h__Script_PartTimeBeat_4279459634(TEXT("/Script/PartTimeBeat"),
	Z_CompiledInDeferFile_FID_Users_dignw_Documents_GitHub_7th_Team6_Final_Project_PartTimeBeat_Source_PartTimeBeat_PartTimeBeatGameMode_h__Script_PartTimeBeat_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_dignw_Documents_GitHub_7th_Team6_Final_Project_PartTimeBeat_Source_PartTimeBeat_PartTimeBeatGameMode_h__Script_PartTimeBeat_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// ********** End Registration *********************************************************************

PRAGMA_ENABLE_DEPRECATION_WARNINGS
