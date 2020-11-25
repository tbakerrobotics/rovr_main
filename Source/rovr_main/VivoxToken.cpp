#include "VivoxToken.h"

DEFINE_LOG_CATEGORY_STATIC(LogVivoxToken, Log, All);

#define VIVOX_VOICE_KEY TEXT("fizz340")

void FVivoxToken::GenerateClientLoginToken(const ILoginSession& LoginSession, FString& OutToken)
{
	FTimespan TokenExpiration = FTimespan::FromSeconds(90);

	OutToken = LoginSession.GetLoginToken(VIVOX_VOICE_KEY, TokenExpiration);

	UE_LOG(LogVivoxToken, Verbose, TEXT("GenerateClientLoginToken %s %s"), *LoginSession.LoginSessionId().Name(), *OutToken);
}

void FVivoxToken::GenerateClientJoinToken(const IChannelSession& ChannelSession, FString& OutToken)
{
	FTimespan TokenExpiration = FTimespan::FromSeconds(90);

	OutToken = ChannelSession.GetConnectToken(VIVOX_VOICE_KEY, TokenExpiration);

	UE_LOG(LogVivoxToken, Verbose, TEXT("GenerateClientJoinToken %s %s"), *ChannelSession.Channel().Name(), *OutToken);
}
