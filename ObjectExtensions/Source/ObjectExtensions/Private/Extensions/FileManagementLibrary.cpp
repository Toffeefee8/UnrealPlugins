// Copyright Phoenix Dawn Development LLC. All Rights Reserved.


#include "Extensions/FileManagementLibrary.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"
#include "UObject/Class.h"
#include "UObject/PropertyPortFlags.h"

TArray<FString> UFileManagementLibrary::GetSavedFiles(FString Subdirectory)
{
	FString SavedDir = GetSavePath() + Subdirectory;
	//UE_LOG(LogTemp,Warning,TEXT("%s"), *SavedDir)
	
	TArray<FString> SaveFiles;
	IFileManager::Get().FindFiles(SaveFiles, *SavedDir, TEXT("*.sav"));

	for (int i = 0; i < SaveFiles.Num(); ++i)
	{
		SaveFiles[i] = FPaths::GetBaseFilename(SaveFiles[i]);
	}
	return SaveFiles;
}

void UFileManagementLibrary::GetFiles(TArray<FString>& FoundFiles, const FString Directory, const FString FileExtension)
{
	IFileManager::Get().FindFiles(FoundFiles, *Directory, *FileExtension);
}

void UFileManagementLibrary::DeleteDirectory(const FString Directory)
{

	if (IFileManager::Get().DirectoryExists(*Directory))
	{
		IFileManager::Get().DeleteDirectory(*Directory,true,true);
	}
}

FString UFileManagementLibrary::GetSavePath(bool Fullpath)
{
	if (Fullpath)
	{
		return FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() + "SaveGames/");
	}
	return FPaths::ProjectSavedDir() + "SaveGames/";
}

FString UFileManagementLibrary::GetUppercaseString(FString String)
{
	FString ReturnString = "";
	for (auto character : String)
	{
		if (FChar::IsUpper(character))
		{
			ReturnString = ReturnString + character;
		}
	}
	return ReturnString;
}