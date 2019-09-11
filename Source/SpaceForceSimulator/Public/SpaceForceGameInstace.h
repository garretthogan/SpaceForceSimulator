// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Online.h"
#include "OVR_Platform.h"
#include "Engine/GameInstance.h"
#include "SpaceForceGameInstace.generated.h"

/**
 * 
 */
UCLASS()
class SPACEFORCESIMULATOR_API USpaceForceGameInstace : public UGameInstance
{
	GENERATED_BODY()

	FOnEndSessionCompleteDelegate OnEndSessionCompleteDelegate;
	FOnDestroySessionCompleteDelegate OnDestroySessionCompleteDelegate;
	FOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate;
	FOnFindFriendSessionCompleteDelegate OnFindFriendSessionCompleteDelegate;
	FOnJoinSessionCompleteDelegate OnJoinSessionCompleteDelegate;

	TMap<FString, TSharedRef<const FUniqueNetId>> FriendsToInvite;
	TMap<FString, TSharedRef<const FUniqueNetId>> FriendsAll;
	TMap<FString, FOnlineSessionSearchResult> FriendsSessions;

public:
	
	UFUNCTION(BlueprintCallable, Category = OculusFriends)
	void ReadOnlineFriends();

	UFUNCTION(BlueprintCallable, Category = OculusFriends)
	void EmptyFriendsList();

	void OnReadFriendsListComplete(
		int32 LocalUserNum,
		const bool bWasSuccessful,
		const FString& ListName,
		const FString& ErrorStr);

	void OnFindFriendSessionComplete(
		int32 LocalUserNum,
		const bool bWasSuccessful,
		const TArray<FOnlineSessionSearchResult>& SearchResults);

	UFUNCTION(BlueprintImplementableEvent, Category = OculusFriends)
	void OnFinishReadingFriendsList();

	UFUNCTION(BlueprintImplementableEvent, Category = OculusFriends)
	void FriendSessionFound(const FString& OwningUserName, const FString& OwningUserId);

	UFUNCTION(BlueprintImplementableEvent, Category = OculusFriends)
	void AddFriendToList(const FString& FriendName, const FString& FriendId);

	UFUNCTION(BlueprintImplementableEvent, Category = OculusFriends)
	void AddFriendToInviteList(const FString& FriendName);

	UFUNCTION(BlueprintCallable, Category = OculusSession)
	void JoinFriendSession(const FString& FriendName);

	void HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type JoinResult);

	UFUNCTION(BlueprintImplementableEvent, Category = OculusSession)
	void OnJoinSessionComplete(FName SessionName, bool bIsJoinSuccessful);

	UFUNCTION(BlueprintCallable, Category = OculusSessions)
	void StartCustomSession(int32 NumConnections, const FString& UserName);

	void HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	UFUNCTION(BlueprintImplementableEvent, Category = OculusSessions)
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	UFUNCTION(BlueprintCallable, Category = OculusSession)
	void EndSession(const FName SessionName);

	UFUNCTION(BlueprintImplementableEvent, Category = OculusSession)
	void OnEndSessionComplete(FName SessionName, bool bWasSuccessful);

	UFUNCTION(BlueprintCallable, Category = OculusSession)
	void DestroySession();

	UFUNCTION(BlueprintImplementableEvent, Category = OculusSession)
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

	UFUNCTION(BlueprintCallable, Category = OculusSession)
	void EndVoip();
};
