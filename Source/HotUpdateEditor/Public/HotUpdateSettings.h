// Copyright 2022 CQUnreal. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "HotUpdateSettings.generated.h"


UENUM(BlueprintType)
enum class EVersionController : uint8
{
	None,
	Git,
	Perforce,
	Subversion
};

/**
 * UHotUpdateSettings
 */
UCLASS(Config = HotUpdate, DefaultConfig, Meta = (DisplayName = "HotUpdate"))
class HOTUPDATEEDITOR_API UHotUpdateSettings : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(Config, EditAnywhere, Category = Build)
	int32 MajorVersion;

	UPROPERTY(Config, EditAnywhere, Category = Build)
	int32 MinorVersion;
	
	UPROPERTY(Config, EditAnywhere, Category = Build)
	EVersionController VersionController;

	UPROPERTY(Config, EditAnywhere, Category = Build, meta = (EditConditionHides, EditCondition = "VersionController==EVersionController::Perforce"))
	FString P4Server;

	UPROPERTY(Config, EditAnywhere, Category = Build, meta = (EditConditionHides, EditCondition = "VersionController==EVersionController::Perforce" ))
	FString P4Username;

	UPROPERTY(Config, EditAnywhere, Category = Build, meta = (EditConditionHides, EditCondition = "VersionController==EVersionController::Perforce"))
	FString P4Password;

	UPROPERTY(Config, EditAnywhere, Category = Build, meta = (EditConditionHides, EditCondition = "VersionController==EVersionController::Perforce"))
	FString P4Workspaces;

#if WITH_EDITORONLY_DATA
	UPROPERTY(Config, EditAnywhere, Category = Build, BlueprintSetter = SetReleaseChannel)
	FName ReleaseChannel;

	UPROPERTY(Config, EditAnywhere, Category = Build)
	TArray<FName> CustomChannels;
#endif

protected:

	UFUNCTION(BlueprintSetter)
	void SetReleaseChannel(const FName NewReleaseChannel);
};
