// Fill out your copyright notice in the Description page of Project Settings.

#include "OculusFriendsListWidget.h"
#include "GameFramework/GameModeBase.h"


void UOculusFriendsListWidget::ReadAllFriends()
{
	auto OculusFriendsInterface = Online::GetFriendsInterface();

	if (!OculusFriendsInterface)
		return;

	UE_LOG_ONLINE(Display, TEXT("Trying to get friends list from server"));

	OculusFriendsInterface->ReadFriendsList(
		0,
		TEXT("default"),
		FOnReadFriendsListComplete::CreateUObject(
			this, &UOculusFriendsListWidget::OnReadFriendsListComplete));
}

void UOculusFriendsListWidget::ReadOnlineFriends()
{
	auto OculusFriendsInterface = Online::GetFriendsInterface();

	if (!OculusFriendsInterface)
		return;

	UE_LOG_ONLINE(Display, TEXT("Trying to get friends list from server"));

	OculusFriendsInterface->ReadFriendsList(
		0,
		TEXT("onlinePlayers"),
		FOnReadFriendsListComplete::CreateUObject(
			this, &UOculusFriendsListWidget::OnReadFriendsListComplete));
}

void UOculusFriendsListWidget::ShowFriendInfo(const FString& FriendName)
{
	auto OculusFriendsInterface = Online::GetFriendsInterface();

	if (!OculusFriendsInterface)
		return;

	UE_LOG_ONLINE(Display, TEXT("Displaying friend info"));

	if (FriendsAll.Contains(FriendName)) 
	{
		auto Friend =
			OculusFriendsInterface->GetFriend(0, FriendsAll[FriendName].Get(), TEXT("default"));
		if (Friend.IsValid()) 
		{
			FriendDisplayName = Friend->GetDisplayName();
			FriendSessionID = Friend->GetPresence().SessionId.IsValid()
				? Friend->GetPresence().SessionId->ToString()
				: TEXT("None");

			FriendOnlineStatus = Friend->GetPresence().bIsOnline == 1 ? TEXT("Yes") : TEXT("No");
			FriendPlayStatus = Friend->GetPresence().bIsPlaying == 1 ? TEXT("Yes") : TEXT("No");
			FriendSameGameStatus =
				Friend->GetPresence().bIsPlayingThisGame == 1 ? TEXT("Yes") : TEXT("No");

			FriendJoinableStatus = Friend->GetPresence().bIsJoinable == 1 ? TEXT("Yes") : TEXT("No");
			FriendVoiceStatus = Friend->GetPresence().bHasVoiceSupport == 1 ? TEXT("Yes") : TEXT("No");
		}
	}
}

void UOculusFriendsListWidget::OnReadFriendsListComplete(int32 LocalUserNum, const bool bWasSuccessful, const FString& ListName, const FString& ErrorStr)
{
	UE_LOG_ONLINE(Display, TEXT("Got results back from reading friends list"));
	if (bWasSuccessful) 
	{
		if (!OnFindFriendSessionCompleteDelegate.IsBound()) 
		{
			auto OculusSessionInterface = Online::GetSessionInterface();

			OnFindFriendSessionCompleteDelegate = FOnFindFriendSessionCompleteDelegate::CreateUObject(
				this, &UOculusFriendsListWidget::OnFindFriendSessionComplete);

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

bool UOculusFriendsListWidget::IsFriendInSession(const FString& FriendName)
{
	return FriendsSessions.Num() > 0 && FriendsSessions.Contains(FriendName);
}

void UOculusFriendsListWidget::OnFindFriendSessionComplete(
	int32 LocalPlayerNum,
	bool bWasSuccessful,
	const TArray<FOnlineSessionSearchResult>& FriendSearchResults) 
{
	UE_LOG_ONLINE(Display, TEXT("Checking if search was successful"));
	if (bWasSuccessful) 
	{
		UE_LOG_ONLINE(Display, TEXT("Got result if friend is in a session"));
		if (FriendSearchResults.Num() > 0)
		{
			UE_LOG_ONLINE(Display, TEXT("Got at least one session"));
			for (auto FriendSearchResult : FriendSearchResults) 
			{
				UE_LOG_ONLINE(Display,
					TEXT("Search result is valid.  Adding to the list: %s"),
					*FriendSearchResult.Session.OwningUserName);

				AddFriendToList(FriendSearchResult.Session.OwningUserName, FriendSearchResult.Session.OwningUserId->ToString());
				FriendsSessions.Add(FriendSearchResult.Session.OwningUserName, FriendSearchResult);
			}
		}
	}
}

void UOculusFriendsListWidget::JoinFriendSession(const FString& FriendName) 
{
	UE_LOG_ONLINE(Display, TEXT("Trying to join %s's session"), *FriendName);

	if (!OnJoinSessionCompleteDelegate.IsBound())
	{
		auto OculusSessionInterface = Online::GetSessionInterface();
		OnJoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(
			this, &UOculusFriendsListWidget::OnJoinSessionComplete);

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

void UOculusFriendsListWidget::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type JoinResult)
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
