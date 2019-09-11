// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Online.h"
#include "OVR_Platform.h"
#include "Engine/GameInstance.h"
#include "SFGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class SPACEFORCESIMULATOR_API USFGameInstance : public UGameInstance
{
	GENERATED_BODY()

	FOnFindFriendSessionCompleteDelegate OnFindFriendSessionCompleteDelegate;

	FOnFindSessionsCompleteDelegate OnFindSessionCompleteDelegate;

	FOnSessionUserInviteAcceptedDelegate OnSessionUserInviteAcceptedDelegate;
	FOnJoinSessionCompleteDelegate OnJoinSessionCompleteDelegate;

	// Creating and starting
	FOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate;
	FOnStartSessionCompleteDelegate OnStartSessionCompleteDelegate;

	// Finding
	FOnMatchmakingCompleteDelegate OnMatchmakingCompleteDelegate;
	FOnCancelMatchmakingCompleteDelegate OnCancelMatchmakingCompleteDelegate;

	FOnEndSessionCompleteDelegate OnEndSessionCompleteDelegate;
	FOnDestroySessionCompleteDelegate OnDestroySessionCompleteDelegate;

	TSharedPtr<FOnlineSessionSearch> SearchSettings;
	TSharedPtr<FOnlineSessionSearch> FindSessionSearchSettings;
	TMap<FString, TSharedRef<const FUniqueNetId>> FriendsToInvite;
	TMap<FString, FOnlineSessionSearchResult> FriendsSessions;

	int32 LastPlayerCount;

public:

	UPROPERTY(BlueprintReadWrite, Category = OculusSession)
	FName SelectedFriend;

	USFGameInstance(const FObjectInitializer& ObjectInitializer) : UGameInstance(ObjectInitializer) 
	{
		if (IsRunningCommandlet()) 
		{
			FModuleManager::Get().LoadModule(TEXT("OnlineSubsystem"));
		}

		auto OculusSessionInterface = Online::GetSessionInterface();
		if (OculusSessionInterface)
		{
			OnSessionUserInviteAcceptedDelegate =
				FOnSessionUserInviteAcceptedDelegate::CreateUObject(this, &USFGameInstance::OnSessionUserInviteAccepted);

			OculusSessionInterface->AddOnSessionUserInviteAcceptedDelegate_Handle(OnSessionUserInviteAcceptedDelegate);

			OnJoinSessionCompleteDelegate =
				FOnJoinSessionCompleteDelegate::CreateUObject(this, &USFGameInstance::OnJoinSessionComplete);

			OculusSessionInterface->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);

			LastPlayerCount = 0;
		}

		SelectedFriend = "None";
	}

	/* Finding and joining friends */

	UFUNCTION(BlueprintCallable, Category = OculusSession)
	void ReadOnlineFriends();

	void OnReadFriendsListComplete(
		int32 LocalUserNum,
		const bool bWasSuccessful,
		const FString& ListName,
		const FString& ErrorStr);

	UFUNCTION(BlueprintImplementableEvent, Category = OculusSession)
	void OnReadFriendsListCompleteBP();

	UFUNCTION(BlueprintImplementableEvent, Category = OculusSession)
	void AddFriendToList(const FString& FriendName);

	UFUNCTION(BlueprintImplementableEvent, Category = OculusSession)
	void AddFriendToInviteList(const FString& FriendName);

	UFUNCTION(BlueprintCallable, Category = OculusSession)
	void InviteFriendToSession(const FString& FriendName);

	void OnSessionUserInviteAccepted(
		const bool bWasSuccessful,
		const int32 ControllerId,
		TSharedPtr<const FUniqueNetId> UserId,
		const FOnlineSessionSearchResult& InviteResult);

	void OnFindFriendSessionComplete(int32 LocalUserNum, const bool bWasSuccessful, const TArray<FOnlineSessionSearchResult>& SearchResults);

	UFUNCTION(BlueprintCallable, Category = OculusSession)
	void JoinFriendSession(const FString& FriendName);

	// Non-blueprint version because the blueprint cant handle regular enums
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type JoinResult);

	UFUNCTION(BlueprintImplementableEvent, Category = OculusSession)
	void OnJoinSessionCompleteBP(FName SessionName, bool bIsJoinSuccessful);

	//////////////////////////////////////////////////////////////////////////

	/* Creating and starting sessions */

	UFUNCTION(BlueprintCallable, Category = OculusSession)
	void CreateSession(const FString& SessionName = "Game", int32 NumConnections = 2);

	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	UFUNCTION(BlueprintImplementableEvent, Category = OculusSession)
	void OnCreateSessionCompleteBP(FName SessionName, bool bWasSuccessful);

	UFUNCTION(BlueprintCallable, Category = OculusSession)
	void StartSession(const FName SessionName);

	UFUNCTION(BlueprintImplementableEvent, Category = OculusSession)
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);

	//////////////////////////////////////////////////////////////////////////

	/* Finding sessions */

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OculusSession)
	FString OwningUserNameBP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OculusSession)
	FString SessionState;
	
	UFUNCTION(BlueprintCallable, Category = OculusSession)
	bool FindSessions(const FString& PoolName = "space_force_survival");

	void OnFindSessionsComplete(bool bWasSuccessful);

	UFUNCTION(BlueprintImplementableEvent, Category = OculusSession)
	void OnFindSessionsCompleteBP(bool bWasSuccessful, const FName& SessionName);

	UFUNCTION(BlueprintCallable, Category = OculusSession)
	bool StartMatchmaking(const FString& PoolName = "space_force_survival");

	UFUNCTION(BlueprintCallable, Category = OculusSession)
	bool CancelMatchmaking(const FName SessionName);

	void OnMatchmakingComplete(FName SessionName, bool bWasSuccessful);

	UFUNCTION(BlueprintImplementableEvent, Category = OculusSession)
	void OnMatchmakingCompleteBP(FName SessionName, bool bWasSuccessful);

	void OnCancelMatchmakingComplete(FName SessionName, bool bWasSuccessful);

	UFUNCTION(BlueprintImplementableEvent, Category = OculusSession)
	void OnCancelMatchmakingCompleteBP(FName SessionName, bool bWasSuccessful);

	//////////////////////////////////////////////////////////////////////////

	/* Destroying and ending sessions */

	UFUNCTION(BlueprintCallable, Category = OculusSession)
	void EndSession(const FName SessionName);

	UFUNCTION(BlueprintImplementableEvent, Category = OculusSession)
	void OnEndSessionComplete(FName SessionName, bool bWasSuccessful);

	UFUNCTION(BlueprintCallable, Category = OculusSession)
	void DestroySession(const FName SessionName);

	UFUNCTION(BlueprintImplementableEvent, Category = OculusSession)
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

	//////////////////////////////////////////////////////////////////////////

	/* Voip */

	UFUNCTION(BlueprintCallable, Category = OculusSession)
	void StartVoip(const FName SessionName);

	UFUNCTION(BlueprintCallable, Category = OculusSession)
	void StopVoip(const FName SessionName);
};
