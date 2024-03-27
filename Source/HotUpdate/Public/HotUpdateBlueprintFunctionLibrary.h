// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Http.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "HotUpdateBlueprintFunctionLibrary.generated.h"


/**
 * 游戏版本管理
 */
UCLASS()
class HOTUPDATE_API UGameVersionManager : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = GameVersion)
	static int32 GetGameMajorVersion();

	UFUNCTION(BlueprintCallable, Category = GameVersion)
	static int32 GetGameMinorVersion();

	UFUNCTION(BlueprintCallable, Category = GameVersion)
	static int32 GetGamePatchVersion();

	UFUNCTION(BlueprintCallable, Category = GameVersion)
	static int32 GetGameBuildVersion();

	UFUNCTION(BlueprintCallable, Category = GameVersion)
	static int32 GetCurrentPatchVersion();

	UFUNCTION(BlueprintCallable, Category = GameVersion)
	static FString GetDisplayGameVersion();

	UFUNCTION(BlueprintCallable, Category = GameVersion)
	static FString GetDisplayCurrentVersion();

	UFUNCTION(BlueprintCallable, Category = GameVersion)
	static FString GetReleaseChannel();

	UFUNCTION(BlueprintCallable, Category = GameVersion, Meta = (WorldContext = "WorldContextObject"))
	static void StartGame(UObject* WorldContextObject);
};

/**
 * 更新检查状态
 */
UENUM(BlueprintType)
enum class ECheckStatus : uint8
{
	NeedUpdate,
	LowVersion,
	ConnectError,
	DataError,
	SystemError,
	Normal
};

/**
 * 更新文件
 */
USTRUCT(BlueprintType)
struct FUpdateFile
{
	GENERATED_USTRUCT_BODY()
	
	FUpdateFile()
		: URL(TEXT(""))
		, PathFile(TEXT(""))
		, MD5(TEXT(""))
		, Size(0)
		, Version(0)
		, Patch(0)
	{

	}

	UPROPERTY(BlueprintReadOnly, Category = Update)
	FString URL;

	UPROPERTY(BlueprintReadOnly, Category = Update)
	FString PathFile;

	UPROPERTY(BlueprintReadOnly, Category = Update)
	FString MD5;

	UPROPERTY(BlueprintReadOnly, Category = Update)
	int32 Size;

	UPROPERTY(BlueprintReadOnly, Category = Update)
	int32 Version;

	UPROPERTY(BlueprintReadOnly, Category = Update)
	int32 Patch;
};

/**
 * 更新内容
 */
USTRUCT(BlueprintType)
struct FUpdateContent 
{
	GENERATED_USTRUCT_BODY()
	
	FUpdateContent()
		: Version(0)
		, TotalBytes(0)
	{

	}

	UPROPERTY(BlueprintReadOnly, Category = Update)
	int32 Version;

	UPROPERTY(BlueprintReadOnly, Category = Update)
	int32 TotalBytes;

	UPROPERTY(BlueprintReadOnly, Category = Update)
	TArray<FUpdateFile> Files;
};

/**
 * 请求更新
 */
USTRUCT(BlueprintType)
struct FRequestUpdate
{
	GENERATED_USTRUCT_BODY()

	FRequestUpdate()
		: Version(0)
		, Channel(TEXT(""))
		, DeviceId(TEXT(""))
	{

	}

	UPROPERTY(BlueprintReadOnly, Category = Update)
	int32 Version;

	UPROPERTY(BlueprintReadOnly, Category = Update)
	FString Channel;

	UPROPERTY(BlueprintReadOnly, Category = Update)
	FString DeviceId;
};

USTRUCT(BlueprintType)
struct FResposeUpdate
{
	GENERATED_USTRUCT_BODY()

	FResposeUpdate()
		: Code(0)
		, Message(TEXT(""))
	{

	}

	UPROPERTY(BlueprintReadOnly, Category = Update)
	int32 Code;

	UPROPERTY(BlueprintReadOnly, Category = Update)
	FString Message;

	UPROPERTY(BlueprintReadOnly, Category = Update)
	TArray<FUpdateFile> Data;
};

/* 
* 检查更新
*/
UCLASS(BlueprintType)
class HOTUPDATE_API UCheckUpdateProxy : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	virtual void Activate() override;

	virtual void BeginDestroy() override;

	virtual void SetReadyToDestroy() override;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FHotUpdateCompleted, ECheckStatus, CheckStatus, int32, ErrorCode, const FUpdateContent&, UpdateContent);
	UPROPERTY(BlueprintAssignable) FHotUpdateCompleted Completed;

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "CheckUpdate", Category = "HotUpdate"))
	static UCheckUpdateProxy* CheckUpdate();

	// 删除过期的Pak文件
	virtual void DeleteExpiredPaks(const FResposeUpdate& ResposeUpdate);

	virtual void OnCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);

private:
	bool bWasCompleted;
};

/**
 * 下载状态
 */
UENUM(BlueprintType)
enum class EDownloadStatus : uint8
{
	Processing,
	ConnectError,
	SaveFailed,
	Successful
};

/*
* 下载更新内容
*/
UCLASS(BlueprintType)
class HOTUPDATE_API UDownloadContentProxy : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	virtual void Activate() override;

	virtual void BeginDestroy() override;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FDownloadContentProgressChanged, EDownloadStatus, DownloadStatus, int32, BytesDownload, int32, TotalBytes);
	UPROPERTY(BlueprintAssignable) FDownloadContentProgressChanged ProgressChanged;

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "DownloadContent", Category = "HotUpdate"))
	static UDownloadContentProxy* DownloadContent(const FUpdateContent& Content);

	virtual void StartupDownloader();

	virtual void OnCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);

	virtual void OnProgress(FHttpRequestPtr Request, int32 BytesSent, int32 BytesReceived);

private:
	int32 DownloadIndex;
	int32 BytesDownload;
	int32 LastBytesReceived;

	FUpdateFile CurrentUpdateFile;

	FUpdateContent UpdateContent;

#if ENGINE_MAJOR_VERSION > 4 || ENGINE_MINOR_VERSION > 24
	TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> HttpRequestPtr;
#else
	TSharedPtr<IHttpRequest> HttpRequestPtr;
#endif
};