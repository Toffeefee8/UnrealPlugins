#include "SaveSystemEditor.h"

#include "Kismet/GameplayStatics.h"
#include "SaveSystem/Public/SaveSubSystem.h"
#include "SaveSystem/Public/SaveObjects/SoloSaveGame.h"

#define LOCTEXT_NAMESPACE "FSaveSystemEditorModule"

const FName FSaveSystemEditorModule::SaveEditorTabName = "Save Editor";

void FSaveSystemEditorModule::StartupModule()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		SaveEditorTabName,
		FOnSpawnTab::CreateRaw(this, &FSaveSystemEditorModule::OnSpawnSaveEditorTab)
	)
	.SetDisplayName(FText::FromName(SaveEditorTabName))
	.SetMenuType(ETabSpawnerMenuType::Enabled);
}

void FSaveSystemEditorModule::ShutdownModule()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(SaveEditorTabName);
}

TArray<FName> FSaveSystemEditorModule::GetAllOptions() const
{
	TArray<FName> AllOptions {};
	
	const FString SaveDir = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("SaveGames/"));
	
	TArray<FString> FoundFiles;
	IFileManager::Get().FindFilesRecursive(
		FoundFiles,
		*SaveDir,
		TEXT("*.sav"),
		/*Files=*/true,
		/*Directories=*/false
	);
	
	for (const FString& FilePath : FoundFiles)
	{
		FString RelativePath = FilePath;
		FPaths::MakePathRelativeTo(RelativePath, *SaveDir);
		RelativePath = FPaths::ChangeExtension(RelativePath, TEXT(""));

		AllOptions.Add(FName(*RelativePath));
	}

	return AllOptions;
}

void FSaveSystemEditorModule::RefreshAllData()
{
	FoundSavePaths = GetAllOptions();
	SetupModifiableSave();
}

bool FSaveSystemEditorModule::SetupModifiableSave()
{
	TObjectPtr<USaveGame> LoadedObject = UGameplayStatics::LoadGameFromSlot(CurrentSelectedPath.ToString(), 0);
	DetailsView->SetObject(LoadedObject);
	CurrentSelectedObject = LoadedObject;
	return LoadedObject != nullptr;
}

TSharedRef<SWidget> FSaveSystemEditorModule::OnGenerateWidget(FName Selected)
{
	return SNew(STextBlock)
	   .Text(FText::FromName(Selected));
}

void FSaveSystemEditorModule::OnSaveSelectionChanged(FName NewValue, ESelectInfo::Type SelectInfo)
{
	CurrentSelectedPath = NewValue;
	SetupModifiableSave();
}

TSharedRef<class SDockTab> FSaveSystemEditorModule::OnSpawnSaveEditorTab(const class FSpawnTabArgs& SpawnTabArgs)
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.bForceHiddenPropertyVisibility = true;
	DetailsViewArgs.bHideSelectionTip = true;
	DetailsViewArgs.bLockable = false;
	DetailsViewArgs.bUpdatesFromSelection = false;
	DetailsViewArgs.bShowPropertyMatrixButton = false;
	DetailsViewArgs.ViewIdentifier = "SaveEditor";
	
	DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	CurrentSelectedPath = FName(USaveSubSystem::GetSoloSaveName());
	
	RefreshAllData();
	
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SVerticalBox)
			
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.FillWidth(1)
				[
					SNew(SComboBox<FName>)
					.OptionsSource(&FoundSavePaths)
					.OnGenerateWidget_Raw(this, &FSaveSystemEditorModule::OnGenerateWidget)
					.OnSelectionChanged_Raw(this, &FSaveSystemEditorModule::OnSaveSelectionChanged)
					[
						SNew(STextBlock)
						.Text_Lambda([this]()
						{
							return CurrentSelectedObject.IsValid()
								? FText::FromName(CurrentSelectedPath)
								: FText::FromString("---");
						})
					]
				]

				+SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SSpacer)
				]
				
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(FText::FromString("Refresh"))
					.ButtonColorAndOpacity(FColor(59, 20, 26))
					.OnClicked_Lambda([this]()
					{
						RefreshAllData();
						return FReply::Handled();
					})
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(FText::FromString("Save"))
					.ButtonColorAndOpacity(FColor(22, 48, 29))
					.OnClicked_Lambda([this]()
					{
						if (CurrentSelectedObject.IsValid())
							UGameplayStatics::SaveGameToSlot(CurrentSelectedObject.Get(), CurrentSelectedPath.ToString(), 0);
						return FReply::Handled();
					})
				]
			]
			
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				DetailsView.ToSharedRef()
			]
		];
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FSaveSystemEditorModule, SaveSystemEditor)