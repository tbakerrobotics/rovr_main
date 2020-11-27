// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "VivoxCore.h"
#include "rovrGameInstance.generated.h"


/**
 *
 */
UCLASS()
class ROVR_MAIN_API UrovrGameInstance : public UGameInstance
{
	GENERATED_BODY()
private:
	FVivoxCoreModule* vModule;
	void Init() override;
	void OnLoginSessionStateChanged(LoginState State);
	void OnChannelSessionConnectionStateChanged(const IChannelConnectionState &State);
	void OnChannelParticipantAdded(const IParticipant &Participant);

public:
	AccountId Account;
	ChannelId Channel;
	IClient *MyVoiceClient;
	ILoginSession *MyLoginSession;
	IChannelSession *MyChannelSession;

public:
	UFUNCTION(BlueprintCallable)
		void JoinVoiceChannel(bool positionalAudio,FString channelName);

	UFUNCTION(BlueprintCallable)
		void sendTextMessage();

	UFUNCTION(BlueprintCallable)
		void initaliseVivox(FString name);

	UFUNCTION(BlueprintCallable)
		void positionalTickUpdate(FVector ActorLocation, FVector ActorForwardVector, FVector ActorUpVector, FString channelName);

	UFUNCTION(BlueprintCallable)
		void Update3DPosition(FVector ActorLocation, FVector ActorForwardVector, FVector ActorUpVector, FString channelName);

	void JoinVoiceWithPermission(bool positionalAudio,FString channelName);

	UFUNCTION(BlueprintCallable)
		void DestroyVivoxSession();

	UFUNCTION(BlueprintCallable)
		void RemoveFromVivoxChannel();
};