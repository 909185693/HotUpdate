// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "HotUpdateModule.h"

DEFINE_LOG_CATEGORY(LogHotUpdate)

#define LOCTEXT_NAMESPACE "FHotUpdateModule"

class FHotUpdateModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
#if UE_SERVER
		FPakPlatformFile* PakPlatformFile = (FPakPlatformFile*)FPlatformFileManager::Get().FindPlatformFile(FPakPlatformFile::GetTypeName());
		if (PakPlatformFile != nullptr)
		{
			int32 PakOrder = 100;
			TArray<FString> FoundFiles;
			FString PaksPath = FPaths::Combine(FPaths::ProjectDir(), TEXT("Paks"));
			FPlatformFileManager::Get().GetPlatformFile().FindFiles(FoundFiles, *PaksPath, TEXT("pak"));
			for (const FString& File : FoundFiles)
			{
				++PakOrder;

				bool bSuccessMount = PakPlatformFile->Mount(*File, PakOrder);

				UE_LOG(LogHotUpdate, Log, TEXT("Mount pak file: %s %s!"), *File, bSuccessMount ? TEXT("succeed") : TEXT("failed"));
			}
		}
#endif
	}

	virtual void ShutdownModule() override
	{

	}
};

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FHotUpdateModule, HotUpdate)