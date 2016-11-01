// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Engine.h"
#include "Networking.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "GameFramework/Actor.h"
#include "UDPReceiver.generated.h"

UCLASS()
class UDPSERVER_API AUDPReceiver : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AUDPReceiver(const FObjectInitializer& ObjectInitializer);
	FSocket* SendSocket= ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_DGram, TEXT("Sender"), true);
	// Socket Subsystem
	ISocketSubsystem* SocketSubsystem;
	/*Array of coordinates from the Phantom Omni*/
	float x;
	float y;
	float z;
	/*Array of coordinates from gimble Phantom Omni*/
	float yawRate;
	float pitchRate;
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;
	/** Called whenever this actor is being removed from a level */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	UFUNCTION(BlueprintCallable, Category = "Servidor")
	void StartServer();
	void SendData();
	void ReceiveData();
};
