// Fill out your copyright notice in the Description page of Project Settings.
#include "rovrGameInstance.h"
#include "Engine/GameEngine.h"
#include "VivoxCore.h"
#include "VivoxToken.h"
#include "AndroidPermissionFunctionLibrary.h"
#include "AndroidPermissionCallbackProxy.h"

#define VIVOX_VOICE_SERVER TEXT("https://mt1s.www.vivox.com/api2")
#define VIVOX_VOICE_DOMAIN TEXT("mt1s.vivox.com")
#define VIVOX_VOICE_ISSUER TEXT("wizdis5860-ro11-dev")
#define VIVOX_VOICE_KEY TEXT("fizz340")

void UrovrGameInstance::Init()
{
	Super::Init();
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Red, TEXT("Game Instance Init"));

	vModule = static_cast<FVivoxCoreModule *>(&FModuleManager::Get().LoadModuleChecked(TEXT("VivoxCore")));
}

void UrovrGameInstance::initaliseVivox(FString name) {
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Red, TEXT("Initialising Vivox"));

	IClient &MyVoiceClient(vModule->VoiceClient());
	MyVoiceClient.Initialize();

	Account = AccountId(VIVOX_VOICE_ISSUER, name, VIVOX_VOICE_DOMAIN);

	ILoginSession &MyLoginSession(MyVoiceClient.GetLoginSession(Account));

	FString LoginToken;
	FVivoxToken::GenerateClientLoginToken(MyLoginSession, LoginToken);

	bool IsLoggedIn = false;

	ILoginSession::FOnBeginLoginCompletedDelegate OnBeginLoginCompleted;
	OnBeginLoginCompleted.BindLambda([this, &IsLoggedIn](VivoxCoreError Error)
		{
			if (VxErrorSuccess == Error)
			{
				IsLoggedIn = true;
				// This bool is only illustrative. The user is now logged in.
				if (GEngine)
					GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Red, TEXT("Logged In - Bind Event"));
			}
		});
	// Request the user to login to Vivox
	MyLoginSession.BeginLogin(VIVOX_VOICE_SERVER, LoginToken, OnBeginLoginCompleted);
}

void UrovrGameInstance::JoinVoiceWithPermission() {

	IClient &MyVoiceClient(vModule->VoiceClient());
	MyVoiceClient.Initialize();

	ChannelId Channel(VIVOX_VOICE_ISSUER, "example_channel", VIVOX_VOICE_DOMAIN, ChannelType::NonPositional);

	ILoginSession &VoiceLoginSession(MyVoiceClient.GetLoginSession(Account));

	IChannelSession &MyChannelSession(VoiceLoginSession.GetChannelSession(Channel));
	bool IsAsynchronousConnectCompleted = false;

	IChannelSession::FOnBeginConnectCompletedDelegate OnBeginConnectCompleted;

	FString JoinToken;
	FVivoxToken::GenerateClientJoinToken(MyChannelSession, JoinToken);

	OnBeginConnectCompleted.BindLambda([this, &IsAsynchronousConnectCompleted](VivoxCoreError Error)
		{
			if (VxErrorSuccess == Error)
			{
				IsAsynchronousConnectCompleted = true;
				// This bool is only illustrative. The connect call has completed.
			}
		});

	MyChannelSession.BeginConnect(true, true, true, JoinToken, OnBeginConnectCompleted);

	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Red, TEXT("Voice Channel Joined"));
}


void UrovrGameInstance::JoinVoiceChannel()
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
							JoinVoiceWithPermission();
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
		JoinVoiceWithPermission();
	}
#else 
	// Not Android
	if (GEngine) {
		GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Red, TEXT("PC:Calling Join"));
	}
	JoinVoiceWithPermission();
#endif
}


void UrovrGameInstance::sendTextMessage()
{
	IClient &MyVoiceClient(vModule->VoiceClient());
	MyVoiceClient.Initialize();

	ChannelId Channel = ChannelId(VIVOX_VOICE_ISSUER, "example_channel", VIVOX_VOICE_DOMAIN);
	FString Message = TEXT("Example channel message.");

	IChannelSession::FOnBeginSendTextCompletedDelegate SendChannelMessageCallback;
	SendChannelMessageCallback.BindLambda([this, Channel, Message](VivoxCoreError Error)
		{
			if (VxErrorSuccess == Error)
			{
				if (GEngine)
					GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Red, TEXT("Message sent to %s:"), *Channel.Name());
				//UE_LOG(MyLog, Log, TEXT("Message sent to %s: %s\n"), *Channel.Name(), *Message);
			}
		});

	ILoginSession &VoiceLoginSession(MyVoiceClient.GetLoginSession(Account));
	IChannelSession &MyChannelSession(VoiceLoginSession.GetChannelSession(Channel));
	MyChannelSession.BeginSendText(Message, SendChannelMessageCallback);
}

void OnChannelTextMessageReceived(const IChannelTextMessage &Message)
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Red, TEXT("%s:"), *Message.Message());
	//UE_LOG(LogTemp, Log, TEXT("%s: %s\n"), *Message.Sender().Name(), *Message.Message());
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
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Red, TEXT("Channel Fully Connected"));
		//UE_LOG(MyLog, Log, TEXT("Channel %s  fully connected\n"), *ChannelName);
	}
	else if (ConnectionState::Disconnected == State.State())
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Red, TEXT("Channel Fully disconnected"));
		//UE_LOG(MyLog, Log, TEXT("Channel %s fully disconnected\n"), *ChannelName);
	}
}

void UrovrGameInstance::OnChannelParticipantAdded(const IParticipant &Participant)
{
	ChannelId Channel = Participant.ParentChannelSession().Channel();
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Red, TEXT("Participant Added - %s"), *Participant.Account().Name());
	//UE_LOG(MyLog, Log, TEXT("%s has been added to %s\n"), *Participant.Account().Name(), *Channel.Name());
}
