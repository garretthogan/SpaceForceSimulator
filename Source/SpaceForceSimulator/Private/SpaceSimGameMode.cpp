// Fill out your copyright notice in the Description page of Project Settings.

#include "SpaceSimGameMode.h"
#include "GameFramework/GameSession.h"
#include "GameFramework/WorldSettings.h"
#include "Engine/World.h"

ASpaceSimGameMode::ASpaceSimGameMode()
{
	MinRequiredPlayers = 2;
	bUseSeamlessTravel = true;
}

void ASpaceSimGameMode::OpenGameMap(const FString& MapName)
{
	UWorld* World = GetWorld();

	if (World)
	{
		World->ServerTravel(MapName + "?listen");
	}
	
}

void ASpaceSimGameMode::HandleMatchIsWaitingToStart()
{
	if (GameSession != nullptr)
	{
		GameSession->HandleMatchIsWaitingToStart();
	}
}

void ASpaceSimGameMode::HandleMatchHasEnded()
{
	Super::HandleMatchHasEnded();
	OnGameOver.Broadcast();
}
