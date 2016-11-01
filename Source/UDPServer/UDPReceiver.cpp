// Fill out your copyright notice in the Description page of Project Settings.

#include "UDPServer.h"
#include "UDPReceiver.h"
#include "Networking.h"
#include "Sockets.h"
#include "SocketSubsystem.h"

// Sets default values
AUDPReceiver::AUDPReceiver(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	this->SetActorTickEnabled(true);
	//ListenSocket = NULL;
}
void AUDPReceiver::StartServer() {

	FString address = TEXT("10.3.137.220");
	int32 port = 8888;
	FIPv4Address ip;
	FIPv4Address::Parse(address, ip);
	FIPv4Endpoint Endpoint(ip, port);
	
	// Buffer Size
	int32 BufferSize = 512;
	//uint32 Size;
	SendSocket = FUdpSocketBuilder("SOCKETSENDER")
		.AsNonBlocking()
		.AsReusable()
		//.BoundToEndpoint(Endpoint)
		.WithReceiveBufferSize(BufferSize);
		//.WithBroadcast()
	;
	//TSharedRef<FInternetAddr> Sender = SocketSubsystem->CreateInternetAddr();
	TSharedRef<FInternetAddr> Sender= ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr(ip.Value,port);
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow,*Sender->ToString(true));
	;
	//bool connect=ListenSocket->Bind(*Sender);
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, connect ? "True" : "False");
	if (!SendSocket->Connect(*Sender)) {
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, "Faied to connect socket");
		}
	}
	else {
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, "Socket connect succesuflly");
		}
		
	}
}
// Called when the game starts or when spawned
void AUDPReceiver::BeginPlay()
{
	Super::BeginPlay();
	StartServer();
	
	
	//Receive(Datagram, &EndPoint);
}


// Called every frame
void AUDPReceiver::Tick( float DeltaTime )
{
	Super::Tick(DeltaTime);
	SendData();
	ReceiveData();
}
void AUDPReceiver::EndPlay(const EEndPlayReason::Type EndPlayReason) {
		Super::EndPlay(EndPlayReason);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, "CLOSE SOCKET");
		
}
void AUDPReceiver::SendData() {
//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, "SEND DATA");
	FString serialized = TEXT("loadPlayer|1");
	TCHAR *serializedChar = serialized.GetCharArray().GetData();
	int32 size = FCString::Strlen(serializedChar);
	int32 sent =0;
	
	bool successful = SendSocket->Send((uint8*)TCHAR_TO_UTF8(serializedChar), size, sent);
}
void AUDPReceiver::ReceiveData() {
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, "RECEIVE DATA");
	FString address = TEXT("127.0.0.1");
	int32 port = 8888;
	FIPv4Address ip;
	
	FIPv4Address::Parse(address, ip);
	int32 BufferSize = 512;
	int32 Read = 0;
	uint32 Size=1;
	//ListenSocket->Wait(WaitForRead, 100);
	check(SendSocket);
	FArrayReaderPtr Datagram = MakeShareable(new FArrayReader(true));
	Datagram->Init(FMath::Min(Size, 65507u),100);	 
	if (SendSocket->Recv(Datagram->GetData(), Datagram->Num(), Read))
	{
		
		char* Data = (char*)Datagram->GetData();
		Data[Read] = '\0';
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, Data);
		char* tokensito;
		tokensito=strtok(Data, "/");
		/*Posicion en Y*/
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, tokensito);
		y = atof(tokensito);
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, tokensito);
		/*Posicion en Z*/
		tokensito = strtok(NULL, "/");
		z = atof(tokensito);
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, tokensito);
		/*Posicion en Y*/
		tokensito = strtok(NULL, "/");
		x= atof(tokensito);
		if (x < 0) {
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, tokensito);
			x = fabsf(x);
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::SanitizeFloat(x));
		}
		else {
			x *= -1;
		}
		/*Yaw Angle Gimble */
		tokensito = strtok(NULL, "/");
		yawRate = atof(tokensito);
		if (yawRate < 0) {
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, tokensito);
			yawRate = fabsf(yawRate);
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::SanitizeFloat(x));
		}
		else {
			yawRate *= -1;
		}
		/*Pitch Angle Gimble*/
		tokensito = strtok(NULL, "/");
		pitchRate = atof(tokensito);
		if (pitchRate < 0) {
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, tokensito);
			pitchRate = fabsf(pitchRate);
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::SanitizeFloat(x));
		}
		else {
			pitchRate *= -1;
		}
	}
	
}
