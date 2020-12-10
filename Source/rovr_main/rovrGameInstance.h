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

	void BindLoginSessionHandlers(bool DoBind, ILoginSession& LoginSession);
	void BindChannelSessionHandlers(bool DoBind, IChannelSession& ChannelSession);

	void OnLoginSessionStateChanged(LoginState State);
	void OnChannelSessionConnectionStateChanged(const IChannelConnectionState &State);

	void OnChannelParticipantAdded(const IParticipant &Participant);
	void OnChannelParticipantRemoved(const IParticipant &Participant);
	void OnChannelParticipantUpdated(const IParticipant &Participant);


public:
	AccountId Account;
	ChannelId Channel;
	IClient *MyVoiceClient;
	ILoginSession *MyLoginSession;
	IChannelSession *MyChannelSession;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool globalAudioType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString globalChannelName;

public:
	UFUNCTION(BlueprintCallable)
		void JoinVoiceChannel();

	UFUNCTION(BlueprintCallable)
		void initaliseVivox(FString name);

	UFUNCTION(BlueprintCallable)
		void positionalTickUpdate(FVector ActorLocation, FVector ActorForwardVector, FVector ActorUpVector, FString channelName);

	UFUNCTION(BlueprintCallable)
		void Update3DPosition(FVector ActorLocation, FVector ActorForwardVector, FVector ActorUpVector, FString channelName);

	void JoinVoiceWithPermission();

	UFUNCTION(BlueprintCallable)
		void DestroyVivoxSession();

	UFUNCTION(BlueprintCallable)
		void RemoveFromVivoxChannel();

	UFUNCTION(BlueprintCallable)
		bool showChannelConnections();

	UFUNCTION(BlueprintCallable)
		void PopulatedChannel();

	UFUNCTION(BlueprintCallable)
		void Vivox_SetAudioType_JoinVoice(bool positionalAudio, FString channelName);
};