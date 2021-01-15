// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/PlayerController.h"
#include "lobbyGMParent.generated.h"

/**
 * 
 */
UCLASS()
class ROVR_MAIN_API AlobbyGMParent : public AGameMode
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void setUpMutexLock(APlayerController* PC);	
	
	UFUNCTION(BlueprintImplementableEvent) //BlueprintNativeEvent if you wanted c++ functionality too
	void Event_MutexPlayerHandler(APlayerController* PC);
};

