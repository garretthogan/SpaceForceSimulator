// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Online.h"
#include "OVR_Platform.h"
#include "Blueprint/UserWidget.h"
#include "OculusSessionWidget.generated.h"

/**
 * 
 */
UCLASS()
class SPACEFORCESIMULATOR_API UOculusSessionWidget : public UUserWidget
{
	GENERATED_BODY()
	
private:

	FOnEndSessionCompleteDelegate OnEndSessionCompleteDelegate;
	FOnDestroySessionCompleteDelegate OnDestroySessionCompleteDelegate;
	
public:

	UFUNCTION(BlueprintCallable, Category = OculusSession)
	void EndSession(const FName SessionName);

	UFUNCTION(BlueprintImplementableEvent, Category = OculusSession)
	void OnEndSessionComplete(FName SessionName, bool bWasSuccessful);

	UFUNCTION(BlueprintCallable, Category = OculusSession)
	void DestroySession(const FName SessionName);

	UFUNCTION(BlueprintImplementableEvent, Category = OculusSession)
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

	UFUNCTION(BlueprintCallable, Category = OculusSession)
	void EndVoip();
	
};
