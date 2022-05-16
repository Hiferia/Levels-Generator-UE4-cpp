// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FWorldGeneratorModule : public IModuleInterface, public FSelfRegisteringExec
{
public:

	UWorld* MyWorld;
	UBlueprint* DefaultFloor;
	UBlueprint* DefaultWall;
	UBlueprint* DestructibleWall;
	int32 Mul;
	int32 PixelsNumber;
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	UWorld* CreateObjWorld();
	void GenerateMap(FString InTexturePath);
	bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar);
	void SpawnActorOnColor(FColor InColor, int32 InX, int32 InY);
};
