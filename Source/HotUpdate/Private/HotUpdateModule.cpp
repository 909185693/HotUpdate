// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "HotUpdateModule.h"
#include "HotUpdateSettings.h"

#if WITH_EDITOR
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#endif

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

		RegisterSettings();
	}

	virtual void ShutdownModule() override
	{
		UnregisterSettings();
	}

	void RegisterSettings()
	{
#if WITH_EDITOR
		ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
		if (!SettingsModule)
			return;

		const auto Section = SettingsModule->RegisterSettings("Project", "Plugins", "HotUpdate",
			LOCTEXT("HotUpdateEditorSettingsName", "HotUpdate"),
			LOCTEXT("HotUpdateEditorSettingsDescription", "HotUpdate Settings"),
			GetMutableDefault<UHotUpdateSettings>());
		Section->OnModified().BindRaw(this, &FHotUpdateModule::OnSettingsModified);
#endif
	}

	void UnregisterSettings()
	{
#if WITH_EDITOR
		ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
		if (SettingsModule)
			SettingsModule->UnregisterSettings("Project", "Plugins", "HotUpdate");
#endif
	}

	bool OnSettingsModified()
	{
		return true;
	}
};

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FHotUpdateModule, HotUpdate)