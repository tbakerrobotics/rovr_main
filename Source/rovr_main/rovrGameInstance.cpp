// Fill out your copyright notice in the Description page of Project Settings.
#include "rovrGameInstance.h"
#include "Engine/GameEngine.h"
#include "VivoxCore.h"
#include "VivoxToken.h"
#include "AndroidPermissionFunctionLibrary.h"
#include "AndroidPermissionCallbackProxy.h"
#include "Kismet/GameplayStatics.h"

#define VIVOX_VOICE_SERVER TEXT("https://mt1s.www.vivox.com/api2")
#define VIVOX_VOICE_DOMAIN TEXT("mt1s.vivox.com")
#define VIVOX_VOICE_ISSUER TEXT("wizdis5860-ro11-dev")
#define VIVOX_VOICE_KEY TEXT("fizz340")

void UrovrGameInstance::Init()
{
	Super::Init();
	vModule = static_cast<FVivoxCoreModule *>(&FModuleManager::Get().LoadModuleChecked(TEXT("VivoxCore")));
	/*
		if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Red, TEXT("VivoxCore - Initalised"));
	*/
}

void UrovrGameInstance::initaliseVivox(FString name) {
	/*
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Red, TEXT("Initialising Vivox Instance"));
	*/
	MyVoiceClient = &vModule->VoiceClient();
	MyVoiceClient->Initialize();

	Account = AccountId(VIVOX_VOICE_ISSUER, name, VIVOX_VOICE_DOMAIN);
	MyLoginSession = &MyVoiceClient->GetLoginSession(Account);

	FString LoginToken;
	FVivoxToken::GenerateClientLoginToken(*MyLoginSession, LoginToken);

	bool IsLoggedIn = false;

	ILoginSession::FOnBeginLoginCompletedDelegate OnBeginLoginCompleted;
	OnBeginLoginCompleted.BindLambda([this, &IsLoggedIn](VivoxCoreError Error)
		{
			if (VxErrorSuccess == Error)
			{
				IsLoggedIn = true;
				// This bool is only illustrative. The user is now logged in.
				/*
					if (GEngine)
						GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Red, TEXT("Logged In - Bind Event"));
				*/
			}
		});
	MyLoginSession->BeginLogin(VIVOX_VOICE_SERVER, LoginToken, OnBeginLoginCompleted);
	MyLoginSession->EventStateChanged;
}

void UrovrGameInstance::JoinVoiceWithPermission(bool positionalAudio,FString channelName)
{
	ChannelType rovrSessionType;

	if (positionalAudio) {
		//Positional Audio
		rovrSessionType = ChannelType::Positional;
	}
	else {
		//Use Direct Audio
		rovrSessionType = ChannelType::NonPositional;
	}

	Channel = ChannelId(VIVOX_VOICE_ISSUER, channelName, VIVOX_VOICE_DOMAIN, rovrSessionType);

	MyChannelSession = &MyLoginSession->GetChannelSession(Channel);

	bool IsAsynchronousConnectCompleted = false;

	IChannelSession::FOnBeginConnectCompletedDelegate OnBeginConnectCompleted;

	FString JoinToken;
	FVivoxToken::GenerateClientJoinToken(*MyChannelSession, JoinToken);

	OnBeginConnectCompleted.BindLambda([this, &IsAsynchronousConnectCompleted](VivoxCoreError Error)
		{
			if (VxErrorSuccess == Error)
			{
				IsAsynchronousConnectCompleted = true;
				// This bool is only illustrative. The connect call has completed.
			}
		});

	MyChannelSession->BeginConnect(true, false, true, JoinToken, OnBeginConnectCompleted);

	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Red, TEXT("Voice Channel Joined"));

	MyChannelSession->EventAfterParticipantAdded;

}


void UrovrGameInstance::JoinVoiceChannel(bool positionalAudio, FString channelName)
{
	#if PLATFORM_ANDROID
	if (!UAndroidPermissionFunctionLibrary::CheckPermission(TEXT("android.permission.RECORD_AUDIO")))
	{
		// Build an array of permissions to request
		TArray<FString> Perms;
		Perms.Add(TEXT("android.permission.RECORD_AUDIO"));
		if (UAndroidPermissionCallbackProxy* Callback = UAndroidPermissionFunctionLibrary::AcquirePermissions(Perms))
		{
			// Bind to the callback so we know when permissions have been granted
			Callback->OnPermissionsGrantedDelegate.BindLambda([this](const TArray<FString>& Permissions, const TArray<bool>& GrantResults)
				{
					// If you requested more than 1 permission, then you should check the Permissions array to see if
					// your specific permission got granted. In this case, we only requested a single permission.
					if (GrantResults.Num() > 0)
					{
						if (GrantResults[0])
						{
							// We got RECORD_AUDIO permission, now we can use the mic
							if (GEngine)
								GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Red, TEXT("ANDROID:We got RECORD_AUDIO permission, now we can use the mic - Calling Join"));
							JoinVoiceWithPermission(positionalAudio, channelName);
						}
					}
				});
		}
	}
	else {
		// We already had permissions so continue using the mic;
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Red, TEXT("ANDROID:Already had RECORD_AUDIO permissions - Calling Join"));
		}
		JoinVoiceWithPermission(positionalAudio, channelName);
	}
#else 
	// Not Android
	/*
		if (GEngine) {
		GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Red, TEXT("PC:Calling Join"));
	}
	*/
	JoinVoiceWithPermission(positionalAudio, channelName);
#endif
}

void UrovrGameInstance::positionalTickUpdate(FVector ActorLocation, FVector ActorForwardVector, FVector ActorUpVector, FString channelName)
{
	float PositionalUpdateRate = 0.1f; // Send position and orientation update every 0.2 seconds. 
	static float NextUpdateTime = UGameplayStatics::GetRealTimeSeconds(GetWorld()) + PositionalUpdateRate;

	if (UGameplayStatics::GetRealTimeSeconds(GetWorld()) > NextUpdateTime)
	{
		NextUpdateTime += PositionalUpdateRate;
		Update3DPosition(ActorLocation, ActorForwardVector, ActorUpVector,channelName);
	}
}

void UrovrGameInstance::Update3DPosition(FVector ActorLocation,FVector ActorForwardVector, FVector ActorUpVector,FString channelName)
{
	// Send new position and orientation to current positional channel
	MyLoginSession->GetChannelSession(Channel).Set3DPosition(ActorLocation, ActorLocation, ActorForwardVector, ActorUpVector);
}

void UrovrGameInstance::DestroyVivoxSession()
{
	MyLoginSession->Logout();
	MyVoiceClient->Uninitialize();
}

void UrovrGameInstance::RemoveFromVivoxChannel() 
{
	MyChannelSession->Disconnect();
	MyLoginSession->DeleteChannelSession(Channel);
}

void UrovrGameInstance::OnLoginSessionStateChanged(LoginState State)
{
	if (LoginState::LoggedOut == State)
	{
		UE_LOG(LogTemp, Error, TEXT("LoginSession Logged Out Unexpectedly\n"));

		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("LoginSession Logged Out Unexpectedly"));
		// Optionally handle other cases
	}
}

void UrovrGameInstance::OnChannelSessionConnectionStateChanged(const IChannelConnectionState &State)
{
	FString ChannelName(State.ChannelSession().Channel().Name());

	if (ConnectionState::Connected == State.State())
	{
		UE_LOG(LogTemp, Log, TEXT("Channel %s  fully connected\n"), *ChannelName);
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Channel %s  fully connected\n"), *ChannelName));
	}

	else if (ConnectionState::Disconnected == State.State())
	{
		UE_LOG(LogTemp, Log, TEXT("Channel %s fully disconnected\n"), *ChannelName);
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Channel %s fully disconnected\n"), *ChannelName));
	}
}


void UrovrGameInstance::OnChannelParticipantAdded(const IParticipant &Participant)
{
	UE_LOG(LogTemp, Log, TEXT("%s has been added to %s\n"), *Participant.Account().Name(), *Channel.Name());

	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("%s has been added to %s\n"), *Participant.Account().Name(), *Channel.Name()));

}