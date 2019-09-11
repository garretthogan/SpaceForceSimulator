// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Proteus.h"
#include "Online.h"
#include "Blueprint/UserWidget.h"
#include "OculusFriendsListWidget.generated.h"

/**
 * 
 */
UCLASS()
class SPACEFORCESIMULATOR_API UOculusFriendsListWidget : public UUserWidget
{
	GENERATED_BODY()
	
private:
	FOnSessionUserInviteAcceptedDelegate OnSessionUserInviteAcceptedDelegate;
	FOnFindFriendSessionCompleteDelegate OnFindFriendSessionCompleteDelegate;
	FOnJoinSessionCompleteDelegate OnJoinSessionCompleteDelegate;
	TMap<FString, FOnlineSessionSearchResult> FriendsSessions;
	TMap<FString, TSharedRef<const FUniqueNetId>> FriendsToInvite;
	TMap<FString, TSharedRef<const FUniqueNetId>> FriendsAll;
	
public:

	UOculusFriendsListWidget(const FObjectInitializer& ObjectInitializer)
		: UUserWidget(ObjectInitializer) 
	{
		if (IsRunningCommandlet()) 
		{
			FModuleManager::Get().LoadModule(TEXT("OnlineSubsystem"));
		}
	}

	UFUNCTION(BlueprintCallable, Category = OculusFriends)
	void ReadAllFriends();

	UFUNCTION(BlueprintCallable, Category = OculusFriends)
	void ReadOnlineFriends();

	UFUNCTION(BlueprintCallable, Category = OculusFriends)
	void ShowFriendInfo(const FString& FriendName);

	void OnReadFriendsListComplete(
		int32 LocalUserNum,
		const bool bWasSuccessful,
		const FString& ListName,
		const FString& ErrorStr);

	UFUNCTION(BlueprintImplementableEvent, Category = OculusFriends)
	void AddFriendToList(const FString& FriendName, const FString& FriendId);

	UFUNCTION(BlueprintImplementableEvent, Category = OculusFriends)
	void AddFriendToInviteList(const FString& FriendName);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = OculusFriends)
	bool IsFriendInSession(const FString& FriendName);

	void OnFindFriendSessionComplete(
		int32 LocalUserNum,
		const bool bWasSuccessful,
		const TArray<FOnlineSessionSearchResult>& SearchResults);

	UFUNCTION(BlueprintCallable, Category = OculusSession)
	void JoinFriendSession(const FString& FriendName);

	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type JoinResult);

	UFUNCTION(BlueprintImplementableEvent, Category = OculusSession)
	void OnJoinSessionComplete(FName SessionName, bool bIsJoinSuccessful);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OculusFriends)
	FString FriendDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OculusFriends)
	FString FriendSessionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OculusFriends)
	FString FriendOnlineStatus;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OculusFriends)
	FString FriendPlayStatus;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OculusFriends)
	FString FriendSameGameStatus;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OculusFriends)
	FString FriendJoinableStatus;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OculusFriends)
	FString FriendVoiceStatus;
	
};
