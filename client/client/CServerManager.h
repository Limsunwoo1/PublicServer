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

// ���� ���̺귯��
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

		// ������ ������ �����ϴ� �Լ�
		void PushSend(void* data);

		// ������ �����͸� ���� �޴� �Լ�
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