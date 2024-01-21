// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "HotUpdateEditorModule.h"
#include "HotUpdateSettings.h"
#include "HotUpdateSettingsDetailCustomization.h"

#if WITH_EDITOR
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#endif

DEFINE_LOG_CATEGORY(LogHotUpdateEditor)

#define LOCTEXT_NAMESPACE "FHotUpdateEditorModule"

class FHotUpdateEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		// Register the details customizer
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.RegisterCustomClassLayout(UHotUpdateSettings::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FHotUpdateSettingsDetailCustomization::MakeInstance));
		PropertyModule.NotifyCustomizationModuleChanged();

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
		Section->OnModified().BindRaw(this, &FHotUpdateEditorModule::OnSettingsModified);
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
	
IMPLEMENT_MODULE(FHotUpdateEditorModule, HotUpdateEditor)