// Fill out your copyright notice in the Description page of Project Settings.


#include "lobbyGMParent.h"
#include "Engine/GameEngine.h"
#include "Runtime/Core/Public/HAL/ThreadingBase.h"

FCriticalSection CriticalSection; // <-- not a pointer, but an actual instance!

void AlobbyGMParent::setUpMutexLock(APlayerController* PC) {
	// unprotected things here...
	{
		// do protected things here..
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Red, TEXT("C SCRIPT - BEFORE LOCK"));

		FScopeLock Lock(&CriticalSection);
		Event_MutexPlayerHandler(PC);

		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Red, TEXT("C SCRIPT - AFTER LOCK"));
	}
	// unprotected things here...
}

