// Copyright Epic Games, Inc. All Rights Reserved.

#include "WorldGenerator.h"
#include "Factories/WorldFactory.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/FileHelper.h"
#include "ImageUtils.h"
#include "Engine/DirectionalLight.h"
#include "Engine/StaticMeshActor.h"
#include "HAL/FileManagerGeneric.h"
#include "AutomatedAssetImportData.h"
#include "AssetToolsModule.h"

#define LOCTEXT_NAMESPACE "FWorldGeneratorModule"

void FWorldGeneratorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FWorldGeneratorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}


UWorld* FWorldGeneratorModule::CreateObjWorld()
{
	FString Name = FString::Printf(TEXT("MapGenerated"));
	FName UniqueName = MakeUniqueObjectName(nullptr, UPackage::StaticClass(), FName(*Name));
	UPackage* Package = CreatePackage(*FString::Printf(TEXT("/Game/GerenatedMaps/%s"), *UniqueName.ToString()));
	UWorldFactory* WorldFactory = NewObject<UWorldFactory>();
	UObject* WorldObject = WorldFactory->FactoryCreateNew(WorldFactory->SupportedClass, Package, UniqueName, EObjectFlags::RF_Standalone | EObjectFlags::RF_Public, nullptr, GWarn);
	UWorld* World = Cast<UWorld>(WorldObject);
	FAssetRegistryModule::AssetCreated(WorldObject);
	World->MarkPackageDirty();

	return World;
}
void FWorldGeneratorModule::GenerateMap(FString InTexturePath)
{
	int32 CubesForRow = 11;
	PixelsNumber = 3;
	MyWorld = CreateObjWorld();
	TArray<uint8> Buffer;
	FFileHelper::LoadFileToArray(Buffer, *InTexturePath);
	UTexture2D* Texture = FImageUtils::ImportBufferAsTexture2D(Buffer);

	//LIGHT
	ADirectionalLight* Light = MyWorld->SpawnActor<ADirectionalLight>();
	Light->SetMobility(EComponentMobility::Movable);

	AStaticMeshActor* Floor = MyWorld->SpawnActor<AStaticMeshActor>();
	UStaticMesh* StaticMesh = LoadObject<UStaticMesh>(nullptr, *FString("StaticMesh'/Engine/BasicShapes/Cube.Cube'"));
	Floor->GetStaticMeshComponent()->SetStaticMesh(StaticMesh);
	Floor->GetStaticMeshComponent()->SetWorldScale3D(FVector(CubesForRow, CubesForRow, 1));
	FVector Location = FVector(0, 0, 0);
	Location.X = ((CubesForRow / 2) * 100);
	Location.Y = ((CubesForRow / 2) * 100);
	Location.Z = -100;
	Floor->SetActorLocation(Location);

	Mul = Texture->GetSizeX() / 32;
	const FColor* ColorData = static_cast<const FColor*>(Texture->PlatformData->Mips[0].BulkData.LockReadOnly());
	for (int32 IndexX = 0; IndexX < Texture->GetSizeX(); IndexX += PixelsNumber * Mul)
	{
		for (int32 IndexY = 0; IndexY < Texture->GetSizeY(); IndexY += PixelsNumber * Mul)
		{
			FColor Pixel = ColorData[IndexY * Texture->GetSizeX() + IndexX];
			SpawnActorOnColor(Pixel, IndexX, IndexY);
		}
	}
	Texture->PlatformData->Mips[0].BulkData.Unlock();
}
bool FWorldGeneratorModule::Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
{
	if (FParse::Command(&Cmd, TEXT("GenerateWorld")))
	{
		FString TexturePath = FParse::Token(Cmd, true);
		//UPluginSettings* PluginSettings = GetMutableDefault<UPluginSetting>();
		//DefaultFloor = LoadObject<UBlueprint>(nullptr, *FString("Blueprint'/Game/Content/Bomberman/DefaultFloor.DefaultFloor'"));
		//DefaultWall = LoadObject<UBlueprint>(nullptr, *FString("Blueprint'/Game/Content/Bomberman/DefaultWall.DefaultWall'"));
		//DestructibleWall = LoadObject<UBlueprint>(nullptr, *FString("Blueprint'/Game/Content/Bomberman/DestructibleWall.DestructibleWall'"));
		//DefaultFloor = LoadObject<UBlueprint>(nullptr, TEXT("C:/Users/luigi/Desktop/UE4 projects/NetworkingProj/Content/Bomberman/DefaultFloor.uasset"));
		//DefaultWall = LoadObject<UBlueprint>(nullptr, TEXT("C:/Users/luigi/Desktop/UE4 projects/NetworkingProj/Content/Bomberman/DefaultWall.uasset"));
		//DestructibleWall = LoadObject<UBlueprint>(nullptr, TEXT("C:/Users/luigi/Desktop/UE4 projects/NetworkingProj/Content/Bomberman/DestructibleWall.uasset"));
		/*DefaultFloor = LoadObject<UBlueprint>(nullptr, TEXT("/Game/Content/Bomberman/DefaultFloor.DefaultFloor_C"));
		DefaultWall = LoadObject<UBlueprint>(nullptr, TEXT("/Game/Content/Bomberman/DefaultWall.DefaultWall_C"));
		DestructibleWall = LoadObject<UBlueprint>(nullptr, TEXT("/Game/Content/Bomberman/DestructibleWall.DestructibleWall_C"));*/

		if (TexturePath == "All")
		{
			const FString& TexturesPath = FString("C:/Users/luigi/Desktop/Maps");
			TArray<FString>ListOfTextures = TArray<FString>();
			FFileManagerGeneric FileManager = FFileManagerGeneric();
			FileManager.FindFilesRecursive(ListOfTextures, *TexturesPath, TEXT("*.png"), true, true);

			for (int32 Index = 0; Index < ListOfTextures.Num(); Index++)
			{
				GenerateMap(*ListOfTextures[Index]);
			}
		}
		else
		{
			GenerateMap(TexturePath);
		}

		UE_LOG(LogTemp, Error, TEXT("loading DONE!"));

		return true;
	}
	return false;
}

//void FWorldGeneratorModule::SpawnActorOnColor(FColor InColor, int32 InX, int32 InY)
//{
//	FVector Offset = FVector(100, 100, 0);
//
//	if (InColor.R == 255)
//	{
//		AStaticMeshActor* Wall = MyWorld->SpawnActor<AStaticMeshActor>(DestructibleWall->GeneratedClass);
//		Wall->SetActorLocation(FVector(InX, InY, 0) * Offset);
//	}
//	else if (InColor.G == 255)
//	{
//		AStaticMeshActor* Wall = MyWorld->SpawnActor<AStaticMeshActor>(DefaultWall->GeneratedClass);
//		Wall->SetActorLocation(FVector(InX, InY, 0) * Offset);
//	}
//}
void FWorldGeneratorModule::SpawnActorOnColor(FColor InColor, int32 InX, int32 InY)
{
	InX /= PixelsNumber * Mul;
	InY /= PixelsNumber * Mul;
	UStaticMesh* StaticMesh = LoadObject<UStaticMesh>(nullptr, *FString("StaticMesh'/Engine/BasicShapes/Cube.Cube'"));
	UStaticMesh* StaticMeshRed = LoadObject<UStaticMesh>(nullptr, *FString("StaticMesh'/Engine/BasicShapes/CubeRed.CubeRed'"));
	float FOffset = StaticMesh->GetBounds().GetBox().GetSize().X;
	FVector Offset = FVector(FOffset, FOffset, 0);
	AStaticMeshActor* MyActor = MyWorld->SpawnActor<AStaticMeshActor>();

	if (InColor.R == 255)
	{
		MyActor->SetActorLocation(FVector(InX, InY, 0) * Offset);
		MyActor->GetStaticMeshComponent()->SetStaticMesh(StaticMeshRed);
	}
	else if (InColor.G == 255)
	{
		MyActor->SetActorLocation(FVector(InX, InY, 0) * Offset);
		MyActor->GetStaticMeshComponent()->SetStaticMesh(StaticMesh);
	}
}

TArray<UObject*> FWorldGeneratorModule::ImportAsset(const TArray<FString>& Files)
{
	UAutomatedAssetImportData* NewTexture = NewObject<UAutomatedAssetImportData>();
	NewTexture->DestinationPath = TEXT("/Game/Assets/Textures");
	NewTexture->Filenames = Files;
	NewTexture->bReplaceExisting = true;

	FAssetToolsModule& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");
	TArray<UObject*>ImportedAssets = AssetTools.Get().ImportAssetsAutomated(NewTexture);

	for (UObject* Obj : ImportedAssets)
	{
		UPackage* Package = Obj->GetPackage();
		FString FileName = FPackageName::LongPackageNameToFilename(Package->GetPathName(), FPackageName::GetAssetPackageExtension());

		UPackage::SavePackage(Package, Obj, RF_Public | RF_Standalone, *FileName);
		FAssetRegistryModule::AssetCreated(Obj);
	}

	return ImportedAssets;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FWorldGeneratorModule, WorldGenerator)