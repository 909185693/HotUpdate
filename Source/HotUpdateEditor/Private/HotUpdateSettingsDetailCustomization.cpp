// Copyright Epic Games, Inc. All Rights Reserved.

#include "HotUpdateSettingsDetailCustomization.h"
#include "HotUpdateSettings.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Interfaces/IPluginManager.h"

//////////////////////////////////////////////////////////////////////////
// FHotUpdateSettingsDetailCustomization

TSharedRef<IDetailCustomization> FHotUpdateSettingsDetailCustomization::MakeInstance()
{
	return MakeShareable(new FHotUpdateSettingsDetailCustomization);
}

void FHotUpdateSettingsDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	ReleaseChannelProperty = DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(UHotUpdateSettings, ReleaseChannel));
	if (IDetailPropertyRow* DetailPropertyRow = DetailLayout.EditDefaultProperty(ReleaseChannelProperty))
	{
		DetailPropertyRow->CustomWidget().IsEnabled(true)
			.NameContent()
			[
				ReleaseChannelProperty->CreatePropertyNameWidget()
			]
			.ValueContent()
			[
				SNew(SComboButton)
					.OnGetMenuContent(this, &FHotUpdateSettingsDetailCustomization::GetReleaseChannelContent)
					.ContentPadding(FMargin(2.0f, 2.0f))
					.IsEnabled(true)
					.ButtonContent()
					[
						SNew(STextBlock)
							.Text(this, &FHotUpdateSettingsDetailCustomization::GetCurrentReleaseChannel)
							.Font(IDetailLayoutBuilder::GetDetailFont())
					]
			];
	}

	ReleaseChannelProperty->SetValue(GetReleaseChannel());
}

TSharedRef<SWidget> FHotUpdateSettingsDetailCustomization::GetReleaseChannelContent()
{
	FMenuBuilder MenuBuilder(true, NULL);

	if (const UHotUpdateSettings* HotUpdateSettings = GetDefault<UHotUpdateSettings>())
	{
		for (int32 Index = 0; Index < HotUpdateSettings->CustomChannels.Num(); Index++)
		{
			FUIAction ItemAction(FExecuteAction::CreateSP(this, &FHotUpdateSettingsDetailCustomization::OnReleaseChannelChanged, Index));
			MenuBuilder.AddMenuEntry(FText::FromName(HotUpdateSettings->CustomChannels[Index]), TAttribute<FText>(), FSlateIcon(), ItemAction);
		}
	}

	return MenuBuilder.MakeWidget();
}

FText FHotUpdateSettingsDetailCustomization::GetCurrentReleaseChannel() const
{
	if (ReleaseChannelProperty.IsValid())
	{
		FText OutText;
		ReleaseChannelProperty->GetValueAsDisplayText(OutText);
		return OutText;
	}

	return FText::FromString(TEXT(""));
}

void FHotUpdateSettingsDetailCustomization::OnReleaseChannelChanged(int32 Index)
{
	if (const UHotUpdateSettings* HotUpdateSettings = GetDefault<UHotUpdateSettings>())
	{
		FName ReleaseChannel = HotUpdateSettings->CustomChannels.IsValidIndex(Index) ? HotUpdateSettings->CustomChannels[Index] : TEXT("");
		if (ReleaseChannelProperty.IsValid())
		{
			ReleaseChannelProperty->SetValue(ReleaseChannel);
		}

		SetReleaseChannel(ReleaseChannel);
	}
}

FName FHotUpdateSettingsDetailCustomization::GetReleaseChannel() const
{
	TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("HotUpdate"));
	if (Plugin.IsValid())
	{
		FString ChannelFilename = FPaths::Combine(*Plugin->GetBaseDir(), TEXT("Source/HotUpdate/Public/Channel.h"));
		
		TArray<FString> ChannelContext;
		FFileHelper::LoadFileToStringArray(ChannelContext, *ChannelFilename);
		for (const FString& ChannelData : ChannelContext)
		{
			if (ChannelData.Contains(TEXT("#define RELEASE_CHANNEL")))
			{
				return FName(ChannelData.RightChop(31).RightChop(2));
			}
		}
	}

	return TEXT("");
}

void FHotUpdateSettingsDetailCustomization::SetReleaseChannel(const FName& NewReleaseChannel)
{
	TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("HotUpdate"));
	if (Plugin.IsValid())
	{
		FString ChannelFilename = FPaths::Combine(*Plugin->GetBaseDir(), TEXT("Source/HotUpdate/Public/Channel.h"));
		
		TArray<FString> ChannelContext;
		FFileHelper::LoadFileToStringArray(ChannelContext, *ChannelFilename);
		for (int32 Index = 0; Index < ChannelContext.Num(); Index++)
		{
			FString& ChannelData = ChannelContext[Index];
			if (ChannelData.Contains(TEXT("#define RELEASE_CHANNEL")))
			{
				ChannelData = FString::Printf(TEXT("#define RELEASE_CHANNEL	TEXT(\"%s\")"), *NewReleaseChannel.ToString());
				FFileHelper::SaveStringArrayToFile(ChannelContext, *ChannelFilename);
				break;
			}
		}
	}
}