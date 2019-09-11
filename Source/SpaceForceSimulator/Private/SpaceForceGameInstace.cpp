// Fill out your copyright notice in the Description page of Project Settings.

#include "SpaceForceGameInstace.h"
#include "Engine/World.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/GameState.h"
#include "Net/OnlineEngineInterface.h"

#define SPACE_FORCE_SURVIVAL_POOL FString("space_force_survival")
#define SETTING_OCULUS_POOL FName(TEXT("OCULUSPOOL"))

void USpaceForceGameInstace::ReadOnlineFriends()
{
	auto OculusFriendsInterface = Online::GetFriendsInterface();

	if (!OculusFriendsInterface)
		return;

	UE_LOG_ONLINE(Display, TEXT("Trying to get friends list from server"));

	OculusFriendsInterface->ReadFriendsList(
		0,
		TEXT("onlinePlayers"),
		FOnReadFriendsListComplete::CreateUObject(
			this, &USpaceForceGameInstace::OnReadFriendsListComplete));
}


void USpaceForceGameInstace::EmptyFriendsList()
{
	FriendsToInvite.Empty();
}

void USpaceForceGameInstace::OnReadFriendsListComplete(int32 LocalUserNum, const bool bWasSuccessful, const FString& ListName, const FString& ErrorStr)
{
	FriendsSessions.Empty();
	OnFinishReadingFriendsList();

	UE_LOG_ONLINE(Display, TEXT("Got results back from reading friends list"));
	if (bWasSuccessful)
	{
		if (!OnFindFriendSessionCompleteDelegate.IsBound())
		{
			auto OculusSessionInterface = Online::GetSessionInterface();

			OnFindFriendSessionCompleteDelegate = FOnFindFriendSessionCompleteDelegate::CreateUObject(
				this, &USpaceForceGameInstace::OnFindFriendSessionComplete);

			OculusSessionInterface->AddOnFindFriendSessionCompleteDelegate_Handle(
				0, OnFindFriendSessionCompleteDelegate);
		}

		auto OculusFriendsInterface = Online::GetFriendsInterface();

		TArray<TSharedRef<FOnlineFriend>> Friends;
		OculusFriendsInterface->GetFriendsList(0, ListName, Friends);

		UE_LOG_ONLINE(Display, TEXT("Online Friends list loaded.  Count of friends: %d"), Friends.Num());

		for (auto Friend : Friends)
		{
			if (ListName.Equals(TEXT("onlinePlayers"), ESearchCase::IgnoreCase))
			{
				auto SessionInterface = Online::GetSessionInterface();
				SessionInterface->FindFriendSession(0, Friend->GetUserId().Get());

				AddFriendToInviteList(Friend->GetDisplayName());
				FriendsToInvite.Add(Friend->GetDisplayName(), Friend->GetUserId());
			}
			else
			{
				AddFriendToList(Friend->GetDisplayName(), Friend->GetUserId().Get().ToString());
				FriendsAll.Add(Friend->GetDisplayName(), Friend->GetUserId());
			}
		}
	}
	else
	{
		UE_LOG_ONLINE(Error, TEXT("%s"), *ErrorStr);
	}
}

void USpaceForceGameInstace::OnFindFriendSessionComplete(int32 LocalUserNum, const bool bWasSuccessful, const TArray<FOnlineSessionSearchResult>& SearchResults)
{
	UE_LOG_ONLINE(Display, TEXT("Checking if search was successful"));
	if (bWasSuccessful)
	{
		UE_LOG_ONLINE(Display, TEXT("Got result if friend is in a session"));
		if (SearchResults.Num() > 0)
		{
			UE_LOG_ONLINE(Display, TEXT("Got at least one session"));
			for (auto FriendSearchResult : SearchResults)
			{
				UE_LOG_ONLINE(Display,
					TEXT("Search result is valid.  Adding to the list: %s"),
					*FriendSearchResult.Session.OwningUserName);

				FriendSessionFound(FriendSearchResult.Session.OwningUserName, FriendSearchResult.Session.OwningUserId->ToString());
				AddFriendToList(FriendSearchResult.Session.OwningUserName, FriendSearchResult.Session.OwningUserId->ToString());
				FriendsSessions.Add(FriendSearchResult.Session.OwningUserName, FriendSearchResult);
			}
		}
	}
}

void USpaceForceGameInstace::JoinFriendSession(const FString& FriendName)
{
	UE_LOG_ONLINE(Display, TEXT("Trying to join %s's session"), *FriendName);

	if (!OnJoinSessionCompleteDelegate.IsBound())
	{
		auto OculusSessionInterface = Online::GetSessionInterface();
		OnJoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(
			this, &USpaceForceGameInstace::HandleJoinSessionComplete);

		OculusSessionInterface->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);
	}

	if (FriendsSessions.Contains(FriendName))
	{
		auto OculusSessionInterface = Online::GetSessionInterface();

		OculusSessionInterface->JoinSession(0, TEXT("Game"), FriendsSessions[FriendName]);
	}
	else
	{
		UE_LOG_ONLINE(Warning, TEXT("Could not find %s"), *FriendName);
	}
}

void USpaceForceGameInstace::HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type JoinResult)
{
	auto OculusSessionInterface = Online::GetSessionInterface();
	auto Session = OculusSessionInterface->GetNamedSession(SessionName);
	FString TravelURL;
	APlayerController* PlayerController = NULL;
	UWorld* const TheWorld = GetWorld();
	if (!TheWorld)
	{
		UE_LOG_ONLINE(Warning, TEXT("The World Does Not Exist."));
		return;
	}
	else
	{
		PlayerController = GetWorld()->GetFirstPlayerController();
		auto gamemode = (AGameModeBase*)GetWorld()->GetAuthGameMode();
		gamemode->bUseSeamlessTravel = false;
		UE_LOG_ONLINE(
			Display,
			TEXT("Seamless Travel Set to : %s"),
			gamemode->bUseSeamlessTravel ? TEXT("True") : TEXT("False"));
	}

	if (Session)
	{
		UE_LOG_ONLINE(
			Display,
			TEXT("Got back %s's session: %s"),
			*Session->OwningUserName,
			*SessionName.ToString());

		if (*Session->OwningUserId == *Online::GetIdentityInterface()->GetUniquePlayerId(0))
		{
			UE_LOG_ONLINE(Display, TEXT("I am the session owner and will host"));
			// This is where you could call
			// GetWorld()->ServerTravel(TEXT("/Game/Maps/Minimal_Default3?listen"));
		}
		else
		{
			UE_LOG_ONLINE(Display, TEXT("Not the session owner"));
			if (PlayerController &&
				OculusSessionInterface->GetResolvedConnectString(SessionName, TravelURL))
			{
				UE_LOG_ONLINE(Display, TEXT("Calling ClientTravel to: %s"), *TravelURL);
				// Finally call the ClienTravel
				PlayerController->ClientTravel(TravelURL, ETravelType::TRAVEL_Absolute);
			}
		}
		auto gamemode = (AGameModeBase*)GetWorld()->GetAuthGameMode();
		gamemode->bUseSeamlessTravel = true; // after first travel, start using seamless travel.
		UE_LOG_ONLINE(
			Display,
			TEXT("Seamless Travel Set to : %s"),
			gamemode->bUseSeamlessTravel ? TEXT("True") : TEXT("False"));
	}

	OnJoinSessionComplete(SessionName, JoinResult == EOnJoinSessionCompleteResult::Success);
}

void USpaceForceGameInstace::StartCustomSession(int32 NumConnections, const FString& UserName)
{
	auto Session = Online::GetSessionInterface();

	if (!OnCreateSessionCompleteDelegate.IsBound())
	{
		OnCreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(
			this, &USpaceForceGameInstace::HandleCreateSessionComplete);

		Session->AddOnCreateSessionCompleteDelegate_Handle(
			OnCreateSessionCompleteDelegate);
	}

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

void USpaceForceGameInstace::HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG_ONLINE(Display, TEXT("SUCCESSFULLY CREATED A SESSION"));
	OnCreateSessionComplete(SessionName, bWasSuccessful);
}

void USpaceForceGameInstace::EndSession(FName SessionName) {
	UE_LOG_ONLINE(Display, TEXT("End Session"));

	auto OculusSessionInterface = Online::GetSessionInterface();
	auto Session = OculusSessionInterface->GetNamedSession(TEXT("Game"));

	if (!OnEndSessionCompleteDelegate.IsBound()) {
		OnEndSessionCompleteDelegate = FOnEndSessionCompleteDelegate::CreateUObject(
			this, &USpaceForceGameInstace::OnEndSessionComplete);
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

void USpaceForceGameInstace::DestroySession()
{
	UE_LOG_ONLINE(Display, TEXT("Destroy Session"));

	auto OculusSessionInterface = Online::GetSessionInterface();
	auto Session = OculusSessionInterface->GetNamedSession(TEXT("Game"));

	if (Session) 
	{
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
			this, &USpaceForceGameInstace::OnDestroySessionComplete);
		OculusSessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
			OnDestroySessionCompleteDelegate);
	}

	OculusSessionInterface->DestroySession(TEXT("Game"));
}

void USpaceForceGameInstace::EndVoip()
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
