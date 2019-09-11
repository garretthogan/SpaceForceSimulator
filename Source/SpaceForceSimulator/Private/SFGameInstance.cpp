// Fill out your copyright notice in the Description page of Project Settings.


#include "SFGameInstance.h"
#include "OnlineSessionSettings.h"
#include "GameFramework/GameModeBase.h"

#define SPACE_FORCE_SURVIVAL_POOL FString("space_force_survival")
#define SETTING_OCULUS_POOL FName(TEXT("OCULUSPOOL"))
#define SETTING_SESSION_NAME FName(TEXT("Session_Name"))

void USFGameInstance::ReadOnlineFriends()
{
	auto OculusFriendsInterface = Online::GetFriendsInterface();

	UE_LOG_ONLINE(Display, TEXT("Trying to get friends list from server"));

	OculusFriendsInterface->ReadFriendsList(0, TEXT("onlinePlayers"),
		FOnReadFriendsListComplete::CreateUObject(this, &USFGameInstance::OnReadFriendsListComplete));
}

void USFGameInstance::OnReadFriendsListComplete(int32 LocalUserNum, const bool bWasSuccessful, const FString& ListName, const FString& ErrorStr)
{
	UE_LOG_ONLINE(Display, TEXT("Got results back from reading friends list"));
	if (bWasSuccessful) 
	{
		auto OculusFriendsInterface = Online::GetFriendsInterface();
		TArray<TSharedRef<FOnlineFriend>> Friends;

		OnReadFriendsListCompleteBP();

		OculusFriendsInterface->GetFriendsList(0, ListName, Friends);
		UE_LOG_ONLINE(Display, TEXT("Online Friends list loaded.  Count of friends: %d"), Friends.Num());

		auto OculusSessionInterface = Online::GetSessionInterface();

		if (!OnFindFriendSessionCompleteDelegate.IsBound()) 
		{
			OnFindFriendSessionCompleteDelegate = FOnFindFriendSessionCompleteDelegate::CreateUObject(
				this, &USFGameInstance::OnFindFriendSessionComplete);

			OculusSessionInterface->AddOnFindFriendSessionCompleteDelegate_Handle(0, OnFindFriendSessionCompleteDelegate);
		}

		for (auto Friend : Friends) 
		{
			OculusSessionInterface->FindFriendSession(0, Friend->GetUserId().Get());
			FriendsToInvite.Add(Friend->GetDisplayName(), Friend->GetUserId());
			AddFriendToInviteList(Friend->GetDisplayName());
		}
	}
	else 
	{
		UE_LOG_ONLINE(Error, TEXT("%s"), *ErrorStr);
	}
}

void USFGameInstance::InviteFriendToSession(const FString& FriendName)
{
	UE_LOG_ONLINE(Display, TEXT("Trying to invite %s"), *FriendName);
	if (FriendsToInvite.Contains(FriendName)) 
	{
		auto OculusSessionInterface = Online::GetSessionInterface();
		bool bSuccess = OculusSessionInterface->SendSessionInviteToFriend(0, TEXT("Game"), FriendsToInvite[FriendName].Get());
		if (!bSuccess)
			UE_LOG_ONLINE(Warning, TEXT("Failed to invite %s"), *FriendName);
	}

	else 
	{
		UE_LOG_ONLINE(Warning, TEXT("Could not find %s"), *FriendName);
	}
}

void USFGameInstance::OnSessionUserInviteAccepted(const bool bWasSuccessful, const int32 ControllerId, TSharedPtr<const FUniqueNetId> UserId, const FOnlineSessionSearchResult& InviteResult)
{
	if (!bWasSuccessful) 
	{
		UE_LOG_ONLINE(Error, TEXT("Did not successfully invited user to the session!"));
		return;
	}

	UE_LOG_ONLINE(Display, TEXT("Accepted invite to session.  Joining session...."));

	auto OculusSessionInterface = Online::GetSessionInterface();
	OculusSessionInterface->JoinSession(ControllerId, TEXT("Game"), InviteResult);
}

void USFGameInstance::OnFindFriendSessionComplete(int32 LocalUserNum, const bool bWasSuccessful, const TArray<FOnlineSessionSearchResult>& SearchResults)
{
	UE_LOG_ONLINE(Display, TEXT("Got result if friend is in a session"));
	if (bWasSuccessful) 
	{
		if (SearchResults.Num() > 0) 
		{
			auto FriendSearchResult = SearchResults[0];

			if (SelectedFriend.ToString() != "None" &&
				SelectedFriend.ToString() == FriendSearchResult.Session.OwningUserName)
			{
				FOnlineSessionSetting SessionNameSetting = FriendSearchResult.Session.SessionSettings.Settings.Find(SETTING_SESSION_NAME);

				FName SessionName = *SessionNameSetting.Data.ToString();

				auto OculusSessionInterface = Online::GetSessionInterface();
				UE_LOG_ONLINE(Display, TEXT("JOINING FRIEND SESSION %s"), *SessionName.ToString());
				OculusSessionInterface->JoinSession(0, SessionName, FriendSearchResult);

				SelectedFriend = "None";
			}

			else
			{
				UE_LOG_ONLINE(Display, TEXT("Search result is valid.  Adding to the list"));
				AddFriendToList(FriendSearchResult.Session.OwningUserName);
			}

			return;
		}
	}
}

void USFGameInstance::JoinFriendSession(const FString& FriendName)
{
	UE_LOG_ONLINE(Display, TEXT("Trying to join %s's session"), *FriendName);
	if (FriendsSessions.Contains(FriendName))
	{
		FOnlineSessionSetting SessionNameSetting = FriendsSessions[FriendName].Session.SessionSettings.Settings.Find(SETTING_SESSION_NAME);

		FName SessionName = *SessionNameSetting.Data.ToString();

		auto OculusSessionInterface = Online::GetSessionInterface();
		OculusSessionInterface->JoinSession(0, SessionName, FriendsSessions[FriendName]);

		UE_LOG_ONLINE(Warning, TEXT("Joining session %s"), *SessionName.ToString());
	}

	else 
	{
		UE_LOG_ONLINE(Warning, TEXT("Could not find %s"), *FriendName);
	}
}

void USFGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type JoinResult)
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
		UE_LOG_ONLINE(Display, TEXT("Seamless Travel Set to : %s"), gamemode->bUseSeamlessTravel ? TEXT("True") : TEXT("False"));
	}

	if (Session) 
	{
		UE_LOG_ONLINE(Display, TEXT("Got back %s's session: %s"), *Session->OwningUserName, *SessionName.ToString());
		OwningUserNameBP = *Session->OwningUserName; // Can be used by Blueprint to update UI
		LastPlayerCount = Session->RegisteredPlayers.Num(); // save off the number of players in the session when I joined to compare later

		if (*Session->OwningUserId == *Online::GetIdentityInterface()->GetUniquePlayerId(0)) // I am the owner
		{
			UE_LOG_ONLINE(Display, TEXT("I am the session owner and will host"));
			// This is where you could call
			GetWorld()->ServerTravel(TEXT("/Game/Garrett/Maps/Lobby/Lobby?listen"));
		}
		else 
		{
			UE_LOG_ONLINE(Display, TEXT("Not the session owner"));
			if (PlayerController &&
				OculusSessionInterface->GetResolvedConnectString(SessionName, TravelURL)) {
				UE_LOG_ONLINE(Display, TEXT("Calling ClientTravel to: %s"), *TravelURL);
				// Finally call the ClienTravel
				PlayerController->ClientTravel(TravelURL, ETravelType::TRAVEL_Absolute);
			}
		}

		auto gamemode = (AGameModeBase*)GetWorld()->GetAuthGameMode();
		gamemode->bUseSeamlessTravel = true; // after first travel, start using seamless travel.
		UE_LOG_ONLINE(Display, TEXT("Seamless Travel Set to : %s"), gamemode->bUseSeamlessTravel ? TEXT("True") : TEXT("False"));
	}

	OnJoinSessionCompleteBP(SessionName, JoinResult == EOnJoinSessionCompleteResult::Success);
}

void USFGameInstance::CreateSession(const FString& SessionName, int32 NumConnections)
{
	auto OculusSessionInterface = Online::GetSessionInterface();

	if (!OculusSessionInterface.IsValid()) 
	{
		return;
	}

	UE_LOG_ONLINE(Display, TEXT("Trying to create a session"));

	if (!OnCreateSessionCompleteDelegate.IsBound()) 
	{
		OnCreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(
			this, &USFGameInstance::OnCreateSessionComplete);

		OculusSessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
			OnCreateSessionCompleteDelegate);
	}

	FOnlineSessionSetting SessionNameSetting(SessionName, EOnlineDataAdvertisementType::ViaOnlineService);

	TSharedPtr<class FOnlineSessionSettings> SessionSettings = MakeShareable(new FOnlineSessionSettings());
	SessionSettings->NumPublicConnections = NumConnections;
	SessionSettings->bShouldAdvertise = true;
	SessionSettings->bAllowJoinInProgress = true;
	SessionSettings->bAllowInvites = true;
	SessionSettings->bAllowJoinViaPresence = true;
	SessionSettings->bShouldAdvertise = true;
	SessionSettings->bUsesPresence = true;
	SessionSettings->bShouldAdvertise = true;
	SessionSettings->Settings.Add(SETTING_OCULUS_POOL, FOnlineSessionSetting(SPACE_FORCE_SURVIVAL_POOL));
	SessionSettings->Settings.Add(SETTING_SESSION_NAME, SessionNameSetting);

	UE_LOG_ONLINE(Display, TEXT("Creating session %s"), *SessionNameSetting.Data.ToString());
	OculusSessionInterface->CreateSession(/* Hosting Player Num*/ 0, *SessionName, *SessionSettings);
}

void USFGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	auto OculusSessionInterface = Online::GetSessionInterface();

	if (!OculusSessionInterface.IsValid()) 
	{
		return;
	}

	UE_LOG_ONLINE(Display, TEXT("CreateSession Call complete"));

	auto Session = OculusSessionInterface->GetNamedSession(SessionName);

	if (Session) 
	{
		UE_LOG_ONLINE(Display, TEXT("Session owned by %s"), *Session->OwningUserName);
		UE_LOG_ONLINE(
			Display, TEXT("Session state: %s"), EOnlineSessionState::ToString(Session->SessionState));
	}

	// Tell the BP we have tried to create a session
	OnCreateSessionCompleteBP(SessionName, bWasSuccessful);
}

void USFGameInstance::StartSession(const FName SessionName)
{
	UE_LOG_ONLINE(Display, TEXT("Start Session"));

	auto OculusSessionInterface = Online::GetSessionInterface();

	auto Session = OculusSessionInterface->GetNamedSession(SessionName);

	if (Session) 
	{
		UE_LOG_ONLINE(Display, TEXT("Session owned by %s"), *Session->OwningUserName);
		UE_LOG_ONLINE(
			Display, TEXT("Session state: %s"), EOnlineSessionState::ToString(Session->SessionState));
	}

	if (!OnStartSessionCompleteDelegate.IsBound()) 
	{
		OnStartSessionCompleteDelegate = FOnStartSessionCompleteDelegate::CreateUObject(
			this, &USFGameInstance::OnStartSessionComplete);

		OculusSessionInterface->AddOnStartSessionCompleteDelegate_Handle(
			OnStartSessionCompleteDelegate);
	}

	OculusSessionInterface->StartSession(SessionName);
}

bool USFGameInstance::FindSessions(const FString& PoolName /*= "space_force_survival"*/)
{
	UE_LOG_ONLINE(Display, TEXT("Start Finding Sessions"));
	auto OculusSessionInterface = Online::GetSessionInterface();

	if (!OnFindSessionCompleteDelegate.IsBound())
	{
		OnFindSessionCompleteDelegate =
			FOnFindSessionsCompleteDelegate::CreateUObject(this, &USFGameInstance::OnFindSessionsComplete);

		OculusSessionInterface->AddOnFindSessionsCompleteDelegate_Handle(OnFindSessionCompleteDelegate);
	}

	FindSessionSearchSettings = MakeShareable(new FOnlineSessionSearch());

	// Search with this poolname
	FindSessionSearchSettings->QuerySettings.Set(FName(TEXT("OCULUSPOOL")), PoolName, EOnlineComparisonOp::Equals);

	TSharedRef<FOnlineSessionSearch> SearchSettingsRef = FindSessionSearchSettings.ToSharedRef();

	return OculusSessionInterface->FindSessions(0, SearchSettingsRef);
}

void USFGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	UE_LOG_ONLINE(Display, TEXT("Finished Searching"));
	if (bWasSuccessful)
	{
		if (FindSessionSearchSettings->SearchResults.Num() > 0)
		{
			auto Result = FindSessionSearchSettings->SearchResults[0];

			FOnlineSessionSetting SessionNameSetting = Result.Session.SessionSettings.Settings.FindRef(SETTING_SESSION_NAME);
			
			FName SessionName = *SessionNameSetting.Data.ToString();

			UE_LOG_ONLINE(Display, TEXT("Found a session, joining..."));
			auto OculusSessionInterface = Online::GetSessionInterface();
			OculusSessionInterface->JoinSession(0, SessionName, Result);

			UE_LOG_ONLINE(Display, TEXT("Session found %s"), *SessionName.ToString());
			OnFindSessionsCompleteBP(true, SessionName);
			return;
		}

		UE_LOG_ONLINE(Display, TEXT("No session found"));
		OnFindSessionsCompleteBP(true, "None");
		return;
	}
	else
	{
		UE_LOG_ONLINE(Display, TEXT("Search unsuccessful"));
		OnFindSessionsCompleteBP(false, "None");
		return;
	}
}

bool USFGameInstance::StartMatchmaking(const FString& PoolName)
{
	auto OculusSessionInterface = Online::GetSessionInterface();

	UE_LOG_ONLINE(Display, TEXT("Starting Matchmaking"));
	TArray<TSharedRef<const FUniqueNetId>> Players;

	// Create a matchmaking for two people
	auto SessionSettings = new FOnlineSessionSettings();
	// SessionSettings->NumPublicConnections = 2;

	SearchSettings = MakeShareable(new FOnlineSessionSearch());

	// Add the delegate
	if (!OnMatchmakingCompleteDelegate.IsBound()) 
	{
		OnMatchmakingCompleteDelegate = FOnMatchmakingCompleteDelegate::CreateUObject(this, &USFGameInstance::OnMatchmakingComplete);
		OculusSessionInterface->AddOnMatchmakingCompleteDelegate_Handle(OnMatchmakingCompleteDelegate);
	}

	// Search with this poolname
	SearchSettings->QuerySettings.Set(FName(TEXT("OCULUSPOOL")), PoolName, EOnlineComparisonOp::Equals);

	TSharedRef<FOnlineSessionSearch> SearchSettingsRef = SearchSettings.ToSharedRef();

	// Do the search
	return OculusSessionInterface->StartMatchmaking(Players, TEXT("Game"), *SessionSettings, SearchSettingsRef);
}

bool USFGameInstance::CancelMatchmaking(const FName SessionName)
{
	auto OculusSessionInterface = Online::GetSessionInterface();

	// Add the delegate
	if (!OnCancelMatchmakingCompleteDelegate.IsBound()) 
	{
		OnCancelMatchmakingCompleteDelegate = FOnCancelMatchmakingCompleteDelegate::CreateUObject(
			this, &USFGameInstance::OnCancelMatchmakingComplete);

		OculusSessionInterface->AddOnCancelMatchmakingCompleteDelegate_Handle(
			OnCancelMatchmakingCompleteDelegate);
	}

	UE_LOG_ONLINE(Display, TEXT("Cancelling Matchmaking"));
	return OculusSessionInterface->CancelMatchmaking(0, TEXT("Game"));
}

void USFGameInstance::OnMatchmakingComplete(FName SessionName, bool bWasSuccessful)
{
	if (!(bWasSuccessful && SearchSettings->SearchResults.Num() > 0)) 
	{
		UE_LOG_ONLINE(Error, TEXT("Did not successfully find a matchmaking session!"));
		return;
	}

	UE_LOG_ONLINE(Display, TEXT("Found a matchmaking session.  Joining session...."));

	auto OculusSessionInterface = Online::GetSessionInterface();
	OculusSessionInterface->JoinSession(0, SessionName, SearchSettings->SearchResults[0]);
	OnMatchmakingCompleteBP(SessionName, bWasSuccessful); // tell the blueprint Matchmaking completed
}

void USFGameInstance::OnCancelMatchmakingComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG_ONLINE(Display, TEXT("Matchmaking Cancel returned: %s"), bWasSuccessful ? TEXT("true") : TEXT("false"));
	OnCancelMatchmakingCompleteBP(SessionName, bWasSuccessful);
}

void USFGameInstance::EndSession(const FName SessionName)
{
	UE_LOG_ONLINE(Display, TEXT("End Session"));

	auto OculusSessionInterface = Online::GetSessionInterface();
	auto Session = OculusSessionInterface->GetNamedSession(SessionName);

	if (!OnEndSessionCompleteDelegate.IsBound()) 
	{
		OnEndSessionCompleteDelegate = FOnEndSessionCompleteDelegate::CreateUObject(this, &USFGameInstance::OnEndSessionComplete);
		OculusSessionInterface->AddOnEndSessionCompleteDelegate_Handle(OnEndSessionCompleteDelegate);
	}

	OculusSessionInterface->EndSession(SessionName);

	if (Session) 
	{
		UE_LOG_ONLINE(Display, TEXT("Session owned by %s"), *Session->OwningUserName);
		UE_LOG_ONLINE(Display, TEXT("Session state: %s"), EOnlineSessionState::ToString(Session->SessionState));
	}
}

void USFGameInstance::DestroySession(const FName SessionName)
{
	UE_LOG_ONLINE(Display, TEXT("Destroy Session"));

	auto OculusSessionInterface = Online::GetSessionInterface();
	auto Session = OculusSessionInterface->GetNamedSession(SessionName);

	if (Session) 
	{
		UE_LOG_ONLINE(Display, TEXT("Session owned by %s"), *Session->OwningUserName);
		UE_LOG_ONLINE(Display, TEXT("Session state: %s"), EOnlineSessionState::ToString(Session->SessionState));
	}

	if (!OnDestroySessionCompleteDelegate.IsBound()) 
	{
		OnDestroySessionCompleteDelegate = FOnDestroySessionCompleteDelegate::CreateUObject(this, &USFGameInstance::OnDestroySessionComplete);
		OculusSessionInterface->AddOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegate);
	}

	OculusSessionInterface->DestroySession(SessionName);
}

void USFGameInstance::StartVoip(const FName SessionName)
{
	auto OculusSessionInterface = Online::GetSessionInterface();
	auto OculusVoiceInterface = Online::GetVoiceInterface();
	auto OculusIdentityInterface = Online::GetIdentityInterface();

	auto Session = OculusSessionInterface->GetNamedSession(SessionName);
	auto UserId = OculusIdentityInterface->GetUniquePlayerId(0);

	if (Session) 
	{
		// OculusSessionInterface->StartSession(SessionName); //

		auto RegisteredPlayers = Session->RegisteredPlayers; // get list of players in the session

		for (auto RegisteredPlayer : RegisteredPlayers)
		{
			// don't register the local player, only the remote
			if (RegisteredPlayer.Get() != *UserId.Get()) 
			{
				OculusVoiceInterface->RegisterRemoteTalker(RegisteredPlayer.Get());
				OculusVoiceInterface->StartNetworkedVoice(0);
				UE_LOG_ONLINE(Display, TEXT("Registered a Talker: %s"), *RegisteredPlayer.Get().ToString());
			}
		}
	}
}

void USFGameInstance::StopVoip(const FName SessionName)
{
	auto OculusSessionInterface = Online::GetSessionInterface();
	auto OculusVoiceInterface = Online::GetVoiceInterface();
	auto OculusIdentityInterface = Online::GetIdentityInterface();

	auto Session = OculusSessionInterface->GetNamedSession(SessionName);
	auto UserId = OculusIdentityInterface->GetUniquePlayerId(0);

	if (Session) 
	{
		OculusVoiceInterface->StopNetworkedVoice(0);
		OculusVoiceInterface->RemoveAllRemoteTalkers();
		UE_LOG_ONLINE(Display, TEXT("Stopped Talking"));
	}
}
