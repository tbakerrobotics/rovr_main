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

/// Return the full name of a UEnum value as an FString
template<typename TEnum>
static FORCEINLINE FString GetUEnumAsString(const FString& Name, TEnum Value, bool ShortName)
{
	const UEnum* enumPtr = FindObject<UEnum>(ANY_PACKAGE, *Name, true);
	if (!enumPtr)
		return FString("InvalidUEnum");
	if (ShortName)
		return enumPtr->GetNameStringByIndex((int64)Value);
	return enumPtr->GetNameByValue((int64)Value).ToString();
}

#define UEnumFullToString(Name, Value) GetUEnumAsString<Name>(#Name, Value, false)
#define UEnumShortToString(Name, Value) GetUEnumAsString<Name>(#Name, Value, true)

void UrovrGameInstance::Init()
{
	Super::Init();
	vModule = static_cast<FVivoxCoreModule *>(&FModuleManager::Get().LoadModuleChecked(TEXT("VivoxCore")));
}

void UrovrGameInstance::initaliseVivox(FString name) {
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
			if (VxErrorSuccess != Error)
			{
				BindLoginSessionHandlers(false, *MyLoginSession); // Unbind handlers if we fail to log in
			}
			else 
			{
				IsLoggedIn = true;
				// This bool is only illustrative. The user is now logged in.
			}
		});
	
	MyLoginSession->BeginLogin(VIVOX_VOICE_SERVER, LoginToken, OnBeginLoginCompleted);
	BindLoginSessionHandlers(true, *MyLoginSession);

	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Vivox Initalised")));
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
			if (VxErrorSuccess != Error)
			{
				BindChannelSessionHandlers(false, *MyChannelSession); // Unbind handlers if we fail to join.
			}
			else {
				IsAsynchronousConnectCompleted = true;
				// This bool is only illustrative. The connect call has completed.
			}
		});
	MyChannelSession->BeginConnect(true, false, true, JoinToken, OnBeginConnectCompleted);
	BindChannelSessionHandlers(true, *MyChannelSession);

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
	MyLoginSession->GetChannelSession(Channel).Disconnect();
	MyLoginSession->DeleteChannelSession(Channel);
}

bool UrovrGameInstance::Exec()
{
	bool StateCheck = false;
	if (MyVoiceClient->GetLoginSession(Account).ChannelSessions().Num() > 0)
		{
		UE_LOG(LogTemp, Log, TEXT("Current Channel List:"));
		UE_LOG(LogTemp, Log, TEXT("===================="));

		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, FString::Printf(TEXT("Current Channel List:")));
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, FString::Printf(TEXT("====================")));

		for (auto &Session : MyVoiceClient->GetLoginSession(Account).ChannelSessions())
		{
			UE_LOG(LogTemp, Log, TEXT("Name: %s, Type: %s, IsTransmitting: %s, AudioState: %s, TextState: %s"),
			*Session.Value->Channel().Name(),
			*UEnumShortToString(ChannelType, Session.Value->Channel().Type()),
			Session.Value->IsTransmitting() ? TEXT("Yes") : TEXT("No"),
			*UEnumShortToString(ConnectionState, Session.Value->AudioState()),
			*UEnumShortToString(ConnectionState, Session.Value->TextState()));

			if (GEngine)
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, FString::Printf(TEXT("Name: %s, Type: %s, IsTransmitting: %s, AudioState: %s, TextState: %s"),
					*Session.Value->Channel().Name(),
					*UEnumShortToString(ChannelType, Session.Value->Channel().Type()),
					Session.Value->IsTransmitting() ? TEXT("Yes") : TEXT("No"),
					*UEnumShortToString(ConnectionState, Session.Value->AudioState()),
					*UEnumShortToString(ConnectionState, Session.Value->TextState())));
		}

		UE_LOG(LogTemp, Log, TEXT("===================="));

		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, FString::Printf(TEXT("====================")));
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Current Channel List: Empty"));

			if (GEngine)
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, FString::Printf(TEXT("Current Channel List: Empty")));

			StateCheck = true;
		}
	return StateCheck;
}


void UrovrGameInstance::BindLoginSessionHandlers(bool DoBind, ILoginSession& LoginSession) {
		if (DoBind)
	{
		LoginSession.EventStateChanged.AddUObject(this, &UrovrGameInstance::OnLoginSessionStateChanged);
	}
	else
	{
		LoginSession.EventStateChanged.RemoveAll(this);
	}
}

void UrovrGameInstance::BindChannelSessionHandlers(bool DoBind, IChannelSession& ChannelSession) {
		if (DoBind)
	{
		ChannelSession.EventAfterParticipantAdded.AddUObject(this, &UrovrGameInstance::OnChannelParticipantAdded);
		ChannelSession.EventBeforeParticipantRemoved.AddUObject(this, &UrovrGameInstance::OnChannelParticipantRemoved);
		ChannelSession.EventAfterParticipantUpdated.AddUObject(this, &UrovrGameInstance::OnChannelParticipantUpdated);
	}
	else
	{
		ChannelSession.EventAfterParticipantAdded.RemoveAll(this);
		ChannelSession.EventBeforeParticipantRemoved.RemoveAll(this);
		ChannelSession.EventAfterParticipantUpdated.RemoveAll(this);
		ChannelSession.EventAudioStateChanged.RemoveAll(this);
		ChannelSession.EventTextStateChanged.RemoveAll(this);
		ChannelSession.EventChannelStateChanged.RemoveAll(this);
		ChannelSession.EventTextMessageReceived.RemoveAll(this);
	}
}

void UrovrGameInstance::OnLoginSessionStateChanged(LoginState State)
{
	if (LoginState::LoggedOut == State)
	{
		UE_LOG(LogTemp, Error, TEXT("LoginSession Logged Out Unexpectedly\n"));

		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("LoginSession Logged Out Unexpectedly"));
	}
}

void UrovrGameInstance::OnChannelSessionConnectionStateChanged(const IChannelConnectionState &State)
{
	FString ChannelName(State.ChannelSession().Channel().Name());

	if (ConnectionState::Connected == State.State())
	{
		UE_LOG(LogTemp, Log, TEXT("Channel %s  fully connected\n"), *ChannelName);
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Channel %s  fully connected\n"), *ChannelName));
	}

	else if (ConnectionState::Disconnected == State.State())
	{
		UE_LOG(LogTemp, Log, TEXT("Channel %s fully disconnected\n"), *ChannelName);
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Channel %s fully disconnected\n"), *ChannelName));
	}
}

void UrovrGameInstance::OnChannelParticipantAdded(const IParticipant &Participant)
{
	ChannelId PatChannel = Participant.ParentChannelSession().Channel();
	UE_LOG(LogTemp, Log, TEXT("%s has been added to %s\n"), *Participant.Account().Name(), *PatChannel.Name());

	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("%s has been added to %s\n"), *Participant.Account().Name(), *PatChannel.Name()));
}

void UrovrGameInstance::OnChannelParticipantRemoved(const IParticipant &Participant)
{
	ChannelId PatChannel = Participant.ParentChannelSession().Channel();
	UE_LOG(LogTemp, Log, TEXT("%s has been removed from %s\n"), *Participant.Account().Name(), *PatChannel.Name());

	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("%s has been removed from %s\n"), *Participant.Account().Name(), *PatChannel.Name()));
}

void UrovrGameInstance::OnChannelParticipantUpdated(const IParticipant &Participant)
{
	ChannelId PatChannel = Participant.ParentChannelSession().Channel();
	UE_LOG(LogTemp, Log, TEXT("%s has been updated in %s\n"), *Participant.Account().Name(), *Channel.Name());

	/*if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("%s has been updated in %s\n"), *Participant.Account().Name(), *PatChannel.Name()));*/
}