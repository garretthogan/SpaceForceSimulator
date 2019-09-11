// Fill out your copyright notice in the Description page of Project Settings.

#include "OculusStartSessionWidget.h"
#include "Online.h"

#define SPACE_FORCE_SURVIVAL_POOL FString("space_force_survival")
#define SETTING_OCULUS_POOL FName(TEXT("OCULUSPOOL"))

void UOculusStartSessionWidget::StartCustomSession(int32 NumConnections, const FString& UserName)
{
	auto Session = Online::GetSessionInterface();

	OnCreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(
		this, &UOculusStartSessionWidget::HandleCreateSessionComplete);
	Session->AddOnCreateSessionCompleteDelegate_Handle(
		OnCreateSessionCompleteDelegate);

	FOnlineSessionSettings Settings = FOnlineSessionSettings();
	Settings.bAllowJoinInProgress = true;
	Settings.bAllowInvites = true;
	Settings.bAllowJoinViaPresence = true;
	Settings.bShouldAdvertise = true;
	Settings.bUsesPresence = true;
	Settings.NumPublicConnections = NumConnections < 2 ? 2 : NumConnections;
	Settings.bShouldAdvertise = true;
	Settings.Settings.Add(SETTING_OCULUS_POOL, FOnlineSessionSetting(SPACE_FORCE_SURVIVAL_POOL));

	auto PoolSettings = Settings.Settings.Find(SETTING_OCULUS_POOL);
	FString Pool;
	PoolSettings->Data.GetValue(Pool);

	UE_LOG_ONLINE(Display, TEXT("POOL KEY %s"), *Pool);

	Session->CreateSession(0, TEXT("Game"), Settings);
}

void UOculusStartSessionWidget::HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG_ONLINE(Display, TEXT("SUCCESSFULLY CREATED A SESSION"));
	OnCreateSessionComplete(SessionName, bWasSuccessful);
}
