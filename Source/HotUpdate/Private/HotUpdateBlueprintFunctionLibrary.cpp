// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "HotUpdateBlueprintFunctionLibrary.h"
#include "HotUpdateModule.h"
#include "HotUpdate/Public/Version.h"
#include "HotUpdate/Public/Channel.h"
#include "JsonObjectConverter.h"
#include "IPlatformFilePak.h"
#include "Misc/Paths.h"
#include "Misc/SecureHash.h"
#include "Misc/FileHelper.h"
#include "Misc/CoreDelegates.h"
#if ENGINE_MAJOR_VERSION >= 5
#include "HAL/PlatformFileManager.h"
#else
#include "HAL/PlatformFilemanager.h"
#endif
#include "Runtime/Launch/Resources/Version.h"


#define VERSION_FILE_PATH FPaths::Combine(*FPaths::ProjectSavedDir(), TEXT("Version.ini"))

static bool bWasMounted = false;

void MountPak()
{
	if (bWasMounted)
	{
		return;
	}
	bWasMounted = true;

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

			bool bSuccessUnmount = PakPlatformFile->Unmount(*File);

			UE_LOG(LogHotUpdate, Log, TEXT("Unmount pak file: %s %s!"), *File, bSuccessUnmount ? TEXT("succeed") : TEXT("failed"));

			bool bSuccessMount = PakPlatformFile->Mount(*File, PakOrder);

			UE_LOG(LogHotUpdate, Log, TEXT("Mount pak file: %s %s!"), *File, bSuccessMount ? TEXT("succeed") : TEXT("failed"));
		}
	}
}

/**
 * 版本管理
 */
UGameVersionManager::UGameVersionManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

int32 UGameVersionManager::GetGameMajorVersion()
{
	return GAME_MAJOR_VERSION;
}

int32 UGameVersionManager::GetGameMinorVersion()
{
	return GAME_MINOR_VERSION;
}

int32 UGameVersionManager::GetGamePatchVersion()
{
	return GAME_PATCH_VERSION;
}

int32 UGameVersionManager::GetGameBuildVersion()
{
	return GAME_BUILD_VERSION;
}

int32 UGameVersionManager::GetCurrentPatchVersion()
{
	FString PatchVersion = TEXT("0");
	FFileHelper::LoadFileToString(PatchVersion, *VERSION_FILE_PATH);

	return FCString::Atoi(*PatchVersion);
}

FString UGameVersionManager::GetDisplayGameVersion()
{
	return FString::Printf(TEXT("%d.%d.%d.%d"), GAME_MAJOR_VERSION, GAME_MINOR_VERSION, GAME_PATCH_VERSION, GAME_BUILD_VERSION);
}

FString UGameVersionManager::GetDisplayCurrentVersion()
{
	return FString::Printf(TEXT("%d.%d.%d.%d"), GAME_MAJOR_VERSION, GAME_MINOR_VERSION, FMath::Max(GAME_PATCH_VERSION, GetCurrentPatchVersion()), GAME_BUILD_VERSION);
}

FString UGameVersionManager::GetReleaseChannel()
{
	return RELEASE_CHANNEL;
}

/*
* 检查更新
*/
void UCheckUpdateProxy::Activate()
{
	bool bEnable = false;
	if (!GConfig->GetBool(TEXT("HotUpdate"), TEXT("bEnable"), bEnable, GEngineIni))
	{
		SetReadyToDestroy();
		return;
	}

	FString ServerAddress;
	if (!GConfig->GetString(TEXT("HotUpdate"), TEXT("ServerAddress"), ServerAddress, GEngineIni))
	{
		UE_LOG(LogHotUpdate, Warning, TEXT("Hot update enabled, but ServerAddress is null."));
		SetReadyToDestroy();
		return;
	}

	FRequestUpdate RequestUpdate;
	RequestUpdate.Version = GAME_BUILD_VERSION;
	RequestUpdate.Channel = RELEASE_CHANNEL;
#if UE_EDITOR
	RequestUpdate.DeviceId = TEXT("Development Editor");
#else
	RequestUpdate.DeviceId = FPlatformMisc::GetDeviceId();
#endif

	FString DataContent = TEXT("{}");
	FJsonObjectConverter::UStructToJsonObjectString(RequestUpdate, DataContent, 0, 0);

	UE_LOG(LogHotUpdate, Log, TEXT("Get updates: %s!"), *DataContent);

#if ENGINE_MAJOR_VERSION > 4 || ENGINE_MINOR_VERSION > 24
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpReuest = FHttpModule::Get().CreateRequest();
#else
	TSharedRef<IHttpRequest> HttpReuest = FHttpModule::Get().CreateRequest();
#endif
	HttpReuest->SetURL(ServerAddress);
	HttpReuest->SetVerb(TEXT("POST"));
	HttpReuest->SetHeader(TEXT("Content-Type"), TEXT("application/json; charset=utf-8"));
	HttpReuest->SetTimeout(10.f);
	HttpReuest->OnProcessRequestComplete().BindUObject(this, &UCheckUpdateProxy::OnCompleted);
	HttpReuest->SetContentAsString(DataContent);
	HttpReuest->ProcessRequest();
}

void UCheckUpdateProxy::BeginDestroy()
{
	if (!bWasCompleted)
	{
		Completed.Broadcast(ECheckStatus::SystemError, -(int32)ECheckStatus::SystemError, FUpdateContent());
	}

	Super::BeginDestroy();
}

void UCheckUpdateProxy::SetReadyToDestroy()
{
	bWasCompleted = true;

	Super::SetReadyToDestroy();
}

UCheckUpdateProxy* UCheckUpdateProxy::CheckUpdate()
{
	UCheckUpdateProxy* NewEvent = NewObject<UCheckUpdateProxy>();
	NewEvent->bWasCompleted = false;

	return NewEvent;
}

void UCheckUpdateProxy::DeleteExpiredPaks(const FResposeUpdate& ResposeUpdate)
{
	FString PaksPath = FPaths::Combine(*FPaths::ProjectDir(), TEXT("Paks"));

	TArray<FString> FileNames;
	IFileManager::Get().FindFiles(FileNames, *PaksPath);

	for (const FString& FileName : FileNames)
	{
		bool bShouldDelete = true;
		for (const FUpdateFile& UpdateFile : ResposeUpdate.Data)
		{
			if (UpdateFile.PathFile.Contains(FileName))
			{
				const FString PathFile = FPaths::Combine(*FPaths::ProjectDir(), *UpdateFile.PathFile);
				const FMD5Hash MD5Hash = FMD5Hash::HashFile(*PathFile);
				const FString MD5 = LexToString(MD5Hash);
				if (MD5 == UpdateFile.MD5)
				{
					bShouldDelete = false;
					break;
				}
			}
		}

		if (bShouldDelete)
		{
			FString DeleteFileName = FPaths::Combine(*PaksPath, *FileName);

			IFileManager::Get().Delete(*DeleteFileName);

			UE_LOG(LogHotUpdate, Log, TEXT("Delete expired pak: %s!"), *DeleteFileName);
		}
	}
}

void UCheckUpdateProxy::OnCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	ECheckStatus CheckStatus = ECheckStatus::Normal;

	FUpdateContent UpdateContent;
	
	int32 ErrorCode = 0;

	if (bConnectedSuccessfully && Response.IsValid())
	{
		const FString Content = Response->GetContentAsString();

		UE_LOG(LogHotUpdate, Log, TEXT("Updates Content: %s!"), *Content);
		
		FResposeUpdate ResposeUpdate;
		bool bResult = FJsonObjectConverter::JsonObjectStringToUStruct<FResposeUpdate>(Content, &ResposeUpdate, 0, 0);
		if (bResult)
		{
			ErrorCode = ResposeUpdate.Code;

			if (ResposeUpdate.Code == 0)
			{
				// 删除过期的Pak文件
				DeleteExpiredPaks(ResposeUpdate);

				const int32 BuildVersion = UGameVersionManager::GetGameBuildVersion();
				UpdateContent.TotalBytes = 0;
				for (const FUpdateFile& UpdateFile : ResposeUpdate.Data)
				{
					if (UpdateFile.Patch == 0)
					{
						CheckStatus = ECheckStatus::LowVersion;

						UpdateContent.Version = 0;
						UpdateContent.TotalBytes = UpdateFile.Size;
						UpdateContent.Files.Empty();
						UpdateContent.Files.Add(UpdateFile);

						break;
					}

					if (UpdateContent.Version < UpdateFile.Patch)
					{
						UpdateContent.Version = UpdateFile.Patch;
					}

					const FString PathFile = FPaths::Combine(*FPaths::ProjectDir(), *UpdateFile.PathFile);
					const FMD5Hash MD5Hash = FMD5Hash::HashFile(*PathFile);
					const FString MD5 = LexToString(MD5Hash);
					if (MD5 != UpdateFile.MD5)
					{
						UpdateContent.TotalBytes += UpdateFile.Size;
						UpdateContent.Files.Add(UpdateFile);

						CheckStatus = ECheckStatus::NeedUpdate;

						UE_LOG(LogHotUpdate, Log, TEXT("NeedUpdateFiles MD5-Source[%s] Target[%s] URL[%s] PathFile[%s] ByteSize[%d]"), *MD5, *UpdateFile.MD5, *UpdateFile.URL, *UpdateFile.PathFile, UpdateFile.Size);
					}
				}

				// 写入版本文件
				const FString VersionData = FString::Printf(TEXT("%d"), UpdateContent.Version);

				FFileHelper::SaveStringToFile(VersionData, *VERSION_FILE_PATH);
			}
			else
			{
				ErrorCode = -(int32)ECheckStatus::SystemError;

				CheckStatus = ECheckStatus::SystemError;
			}
		}
		else
		{
			ErrorCode = -(int32)ECheckStatus::DataError;

			CheckStatus = ECheckStatus::DataError;
		}
	}
	else
	{
		ErrorCode = -(int32)ECheckStatus::ConnectError;

		CheckStatus = ECheckStatus::ConnectError;
	}

	if (CheckStatus == ECheckStatus::Normal)
	{
		MountPak();
	}

	Completed.Broadcast(CheckStatus, ErrorCode, UpdateContent);

	SetReadyToDestroy();
}


/*
* 下载更新内容
*/
void UDownloadContentProxy::Activate()
{
	StartupDownloader();
}

void UDownloadContentProxy::BeginDestroy()
{
	if (HttpRequestPtr.IsValid())
	{
		HttpRequestPtr->CancelRequest();
	}

	Super::BeginDestroy();
}

UDownloadContentProxy* UDownloadContentProxy::DownloadContent(const FUpdateContent& Content)
{
	UDownloadContentProxy* NewEvent = NewObject<UDownloadContentProxy>();
	NewEvent->LastBytesReceived = 0;
	NewEvent->BytesDownload = 0;
	NewEvent->DownloadIndex = 0;
	NewEvent->UpdateContent = Content;

	return NewEvent;
}

void UDownloadContentProxy::StartupDownloader()
{
	if (UpdateContent.Files.IsValidIndex(DownloadIndex))
	{
		LastBytesReceived = 0;

		CurrentUpdateFile = UpdateContent.Files[DownloadIndex++];

#if ENGINE_MAJOR_VERSION > 4 || ENGINE_MINOR_VERSION > 24
		TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpReuest = FHttpModule::Get().CreateRequest();
#else
		TSharedRef<IHttpRequest> HttpReuest = FHttpModule::Get().CreateRequest();
#endif
		HttpReuest->SetURL(CurrentUpdateFile.URL);
		HttpReuest->SetVerb(TEXT("GET"));
		HttpReuest->SetHeader(TEXT("Content-Type"), TEXT("application/x-www-form-urlencoded"));
		HttpReuest->SetTimeout(60.f);
		HttpReuest->OnProcessRequestComplete().BindUObject(this, &UDownloadContentProxy::OnCompleted);
		HttpReuest->OnRequestProgress().BindUObject(this, &UDownloadContentProxy::OnProgress);
		HttpReuest->ProcessRequest();

		HttpRequestPtr = HttpReuest;
	}
	else
	{
		// 挂载补丁文件
		MountPak();

		ProgressChanged.Broadcast(EDownloadStatus::Successful, BytesDownload, UpdateContent.TotalBytes);

		SetReadyToDestroy();
	}
}

void UDownloadContentProxy::OnCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	if (bConnectedSuccessfully && EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		FString SavedPath = FPaths::Combine(FPaths::ProjectDir(), CurrentUpdateFile.PathFile);
		bool bSaved = FFileHelper::SaveArrayToFile(Response->GetContent(), *SavedPath);
		if (bSaved)
		{
			StartupDownloader();
		}
		else
		{
			ProgressChanged.Broadcast(EDownloadStatus::SaveFailed, BytesDownload, UpdateContent.TotalBytes);
		}
	}
	else
	{
		ProgressChanged.Broadcast(EDownloadStatus::ConnectError, BytesDownload, UpdateContent.TotalBytes);

		SetReadyToDestroy();
	}

	HttpRequestPtr.Reset();
	HttpRequestPtr = nullptr;
}

void UDownloadContentProxy::OnProgress(FHttpRequestPtr Request, int32 BytesSent, int32 BytesReceived)
{
	BytesDownload += (BytesReceived - LastBytesReceived);

	LastBytesReceived = BytesReceived;

	ProgressChanged.Broadcast(EDownloadStatus::Processing, BytesDownload, UpdateContent.TotalBytes);
}
