// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Online.h"
#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "OculusStartSessionWidget.generated.h"

/**
 * 
 */
UCLASS()
class SPACEFORCESIMULATOR_API UOculusStartSessionWidget : public UUserWidget
{
	GENERATED_BODY()

	FOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate;

public:

	UFUNCTION(BlueprintCallable, Category = OculusSessions)
	void StartCustomSession(int32 NumConnections, const FString& UserName);
	
	void HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	UFUNCTION(BlueprintImplementableEvent, Category = OculusSessions)
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
};
