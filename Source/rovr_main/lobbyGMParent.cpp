// Fill out your copyright notice in the Description page of Project Settings.


#include "lobbyGMParent.h"
#include "Runtime/Core/Public/HAL/ThreadingBase.h"

FCriticalSection CriticalSection; // <-- not a pointer, but an actual instance!

void AlobbyGMParent::setUpMutexLock(APlayerController* PC) {
	// unprotected things here...
	{
		// do protected things here...

		FScopeLock Lock(&CriticalSection);
		Event_MutexPlayerHandler(PC);
	}
	// unprotected things here...
}

