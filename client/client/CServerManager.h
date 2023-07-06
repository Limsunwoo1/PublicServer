#pragma once
#include "../../server/Packet/Packet.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <string>`
#include <thread>
#include "def.h"

#define PORT_NUMBER			8080
#define HOST_IP				"127.0.0.1"

// 소켓 라이브러리
#pragma comment(lib,"ws2_32.lib")


namespace Server
{
	class ServerManager
	{
		SINGLE(ServerManager)
	public:
		void Initalize();
		void SocketCreate();
		void ConvertIP();
		void Connect();
		void Clear();

		// 서버에 데이터 수신하는 함수
		void PushSend(void* data);

		// 서버의 데이터를 수신 받는 함수
		void Receive();

		void Rlease();

		GETSET(const WSADATA, mWSData, WSData)
		GETSET(const WORD, mVersion, Version)
		GETSET(const SOCKET, mSocket, Socket)
		GETSET(const sockaddr_in, mServerAddr, ServerAddr)
		GETSET(const std::string&, mServerIP, ServerIP)
		GETSET(const std::string&, mClientName, ClientName)

	private:
		WSADATA					mWSData;
		WORD					mVersion;
		SOCKET					mSocket;
		sockaddr_in				mServerAddr;

		std::string				mClientName;
		std::string				mServerIP;
	};
}