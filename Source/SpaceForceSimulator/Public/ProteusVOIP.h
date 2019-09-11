// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Online.h"
#include "ProteusVOIP.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SPACEFORCESIMULATOR_API UProteusVOIP : public UActorComponent
{
	GENERATED_BODY()

public:

	bool bTalkerRegistered;

	FString RegisteredTalkerOculusId;

public:
	// Sets default values for this component's properties
	UProteusVOIP();

	UFUNCTION(BlueprintCallable, Category = "OculusVOIP")
		bool RegisterRemoteTalker(FString OculusID);

	UFUNCTION(BlueprintCallable, Category = "OculusVOIP")
		void UnregisterRemoteTalker();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
};
