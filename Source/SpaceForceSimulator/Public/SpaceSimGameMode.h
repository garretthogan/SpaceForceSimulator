// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "SpaceSimGameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGameOverEvent);

/**
 * 
 */
UCLASS()
class SPACEFORCESIMULATOR_API ASpaceSimGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:

	ASpaceSimGameMode();
	
	UPROPERTY(BlueprintAssignable, Category = "Space Mode")
	FGameOverEvent OnGameOver;

protected:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Space Mode")
	int32 MinRequiredPlayers;

	UFUNCTION(BlueprintCallable, Category = "Space Mode")
	void OpenGameMap(const FString& MapName);

	virtual void HandleMatchIsWaitingToStart() override;

	virtual void HandleMatchHasEnded() override;
};
