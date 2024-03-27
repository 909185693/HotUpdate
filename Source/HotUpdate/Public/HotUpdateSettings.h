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
UCLASS(Config = Game, DefaultConfig, Meta = (DisplayName = "HotUpdate"))
class HOTUPDATE_API UHotUpdateSettings : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(Config, EditAnywhere, Category = Build)
	bool bEnable;

	UPROPERTY(Config, EditAnywhere, Category = Build)
	FString ServerAddress;

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

	UPROPERTY(Config, EditAnywhere, Category = Build)
	FString GameDefaultMap;

	UPROPERTY(Config, EditAnywhere, Category = Build)
	FString LocalMapOptions;

#if WITH_EDITORONLY_DATA
	UPROPERTY(Config, EditAnywhere, Category = Build)
	FName ReleaseChannel;

	UPROPERTY(Config, EditAnywhere, Category = Build)
	TArray<FName> CustomChannels;
#endif
};
