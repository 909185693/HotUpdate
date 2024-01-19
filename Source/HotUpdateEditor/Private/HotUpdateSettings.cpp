// Copyright 2022 CQUnreal. All Rights Reserved.


#include "HotUpdateSettings.h"


void UHotUpdateSettings::SetReleaseChannel(const FName NewReleaseChannel)
{
	UE_LOG(LogTemp, Log, TEXT("SetChannel: %s"), *NewReleaseChannel.ToString());
}