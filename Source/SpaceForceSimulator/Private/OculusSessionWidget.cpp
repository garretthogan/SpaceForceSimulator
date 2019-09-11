// Fill out your copyright notice in the Description page of Project Settings.

#include "OculusSessionWidget.h"


void UOculusSessionWidget::EndSession(FName SessionName) {
	UE_LOG_ONLINE(Display, TEXT("End Session"));

	auto OculusSessionInterface = Online::GetSessionInterface();
	auto Session = OculusSessionInterface->GetNamedSession(SessionName);

	if (!OnEndSessionCompleteDelegate.IsBound()) {
		OnEndSessionCompleteDelegate = FOnEndSessionCompleteDelegate::CreateUObject(
			this, &UOculusSessionWidget::OnEndSessionComplete);
		OculusSessionInterface->AddOnEndSessionCompleteDelegate_Handle(OnEndSessionCompleteDelegate);
	}

	OculusSessionInterface->EndSession(SessionName);

	if (Session) {
		UE_LOG_ONLINE(Display, TEXT("Session owned by %s"), *Session->OwningUserName);
		UE_LOG_ONLINE(
			Display, TEXT("Session state: %s"), EOnlineSessionState::ToString(Session->SessionState));
	}

	else
	{
		OnEndSessionComplete("Game", true);
	}
}

void UOculusSessionWidget::DestroySession(FName SessionName) {
	UE_LOG_ONLINE(Display, TEXT("Destroy Session"));

	auto OculusSessionInterface = Online::GetSessionInterface();
	auto Session = OculusSessionInterface->GetNamedSession(TEXT("Game"));

	if (Session) {
		UE_LOG_ONLINE(Display, TEXT("Session owned by %s"), *Session->OwningUserName);
		UE_LOG_ONLINE(
			Display, TEXT("Session state: %s"), EOnlineSessionState::ToString(Session->SessionState));
	}
	else 
	{
		OnDestroySessionComplete(TEXT("Game"), true);
		return;
	}

	if (!OnDestroySessionCompleteDelegate.IsBound()) {
		OnDestroySessionCompleteDelegate = FOnDestroySessionCompleteDelegate::CreateUObject(
			this, &UOculusSessionWidget::OnDestroySessionComplete);
		OculusSessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
			OnDestroySessionCompleteDelegate);
	}

	OculusSessionInterface->DestroySession(TEXT("Game"));
}

void UOculusSessionWidget::EndVoip()
{
	auto Session = Online::GetSessionInterface().Get();
	IOnlineVoicePtr OculusVoiceInterface = Online::GetVoiceInterface();
	IOnlineIdentityPtr OculusIdentityInterface = Online::GetIdentityInterface();
	auto UserId = OculusIdentityInterface->GetUniquePlayerId(0);

	if (Session)
	{
		auto NamedSession = Session->GetNamedSession("Game");
		if (NamedSession)
		{
			auto RegisteredPlayers = NamedSession->RegisteredPlayers;
			for (auto RegisteredPlayer : RegisteredPlayers)
			{
				if (*UserId.Get() != RegisteredPlayer.Get())
				{
					OculusVoiceInterface->UnregisterRemoteTalker(RegisteredPlayer.Get());
				}

			}
		}
	}
}
