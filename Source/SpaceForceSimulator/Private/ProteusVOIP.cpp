// Fill out your copyright notice in the Description page of Project Settings.

#include "ProteusVOIP.h"
#include "Online.h"
#include "GameFramework/OnlineSession.h"


// Sets default values for this component's properties
UProteusVOIP::UProteusVOIP()
	: bTalkerRegistered(false),
	RegisteredTalkerOculusId(FString())
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UProteusVOIP::BeginPlay()
{
	auto Session = Online::GetSessionInterface().Get();
	IOnlineVoicePtr OculusVoiceInterface = Online::GetVoiceInterface();
	IOnlineIdentityPtr OculusIdentityInterface = Online::GetIdentityInterface();
	auto UserId = OculusIdentityInterface->GetUniquePlayerId(0);

	if (Session)
	{
		OculusVoiceInterface->StartNetworkedVoice(0);
		auto RegisteredPlayers = Session->GetNamedSession(TEXT("Game"))->RegisteredPlayers;
		for (auto RegisteredPlayer : RegisteredPlayers)
		{
			if (*UserId.Get() != RegisteredPlayer.Get())
			{
				OculusVoiceInterface->RegisterRemoteTalker(RegisteredPlayer.Get());
			}

		}
	}

	Super::BeginPlay();
}

void UProteusVOIP::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	IOnlineIdentityPtr OculusIdentityInterface = Online::GetIdentityInterface();
	auto UserId = OculusIdentityInterface->GetUniquePlayerId(0);
	IOnlineVoicePtr OculusVoiceInterface = Online::GetVoiceInterface();

	// OculusVoiceInterface->RemoveAllRemoteTalkers();
	// OculusVoiceInterface->UnregisterRemoteTalker(*UserId.Get());

	Super::EndPlay(EndPlayReason);
}

bool UProteusVOIP::RegisterRemoteTalker(FString OculusID)
{
	/*
	if (bTalkerRegistered)
	{
		if (OculusID != RegisteredTalkerOculusId)
		{
			UE_LOG(LogTemp, Warning, TEXT("TALKER ALEARDY REGISTERED"));
			// this talker is already registered
			return true;
		}
		else {
			UnregisterRemoteTalker();
		}
	}

	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get())
	{
		auto IdentityInterface = OnlineSub->GetIdentityInterface();

		if (IdentityInterface.IsValid())
		{
			auto uniqueNetId = IdentityInterface->CreateUniquePlayerId(OculusID);

			if (uniqueNetId.IsValid())
			{
				auto VoiceInterface = OnlineSub->GetVoiceInterface();

				if (VoiceInterface.IsValid())
				{
					bTalkerRegistered = VoiceInterface->RegisterRemoteTalker(*uniqueNetId);
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("INVALID NET ID"));
			}
		}
	}

	if (bTalkerRegistered)
	{	
		UE_LOG(LogTemp, Warning, TEXT("REGISTERED: %s"), *OculusID);
		RegisteredTalkerOculusId = OculusID;
	}
	*/
	return true;
}

void UProteusVOIP::UnregisterRemoteTalker()
{
	IOnlineIdentityPtr OculusIdentityInterface = Online::GetIdentityInterface();
	auto UserId = OculusIdentityInterface->GetUniquePlayerId(0);
	IOnlineVoicePtr OculusVoiceInterface = Online::GetVoiceInterface();

	OculusVoiceInterface->RemoveAllRemoteTalkers();
	OculusVoiceInterface->UnregisterRemoteTalker(*UserId.Get());

	/*
	if (bTalkerRegistered)
	{
		if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get())
		{
			auto IdentityInterface = OnlineSub->GetIdentityInterface();

			if (IdentityInterface.IsValid())
			{
				auto uniqueNetId = IdentityInterface->CreateUniquePlayerId(RegisteredTalkerOculusId);

				if (uniqueNetId.IsValid())
				{
					auto VoiceInterface = OnlineSub->GetVoiceInterface();

					if (VoiceInterface.IsValid())
					{
						VoiceInterface->UnregisterRemoteTalker(*uniqueNetId);
					}
				}
			}
		}

		bTalkerRegistered = false;
	}
	*/
}
