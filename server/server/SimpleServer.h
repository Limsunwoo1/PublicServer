#pragma once
#include "Engine.h"

#define LOCAL_HOST "127.0.0.1"
#define PORT_NUMBER 8080

using SocketList = std::list<SOCKET>;
using SocketPair = std::pair<SOCKET, SOCKET>;
using SocketMap = std::map<std::string, SOCKET>;

class SimpleServer
{
	SINGLE(SimpleServer)
public:
	void Initalize();
	void WSAStartUp();
	void CreateScoket();
	void BindSocket();
	void ListenSocket();
	void Accept();

	INT SendData(SOCKET sock, void* data, int dataLen, int flags = 0);
	INT ReceiveData(SOCKET sock, void* data, int dataLen, int flags = 0);

	void EXIT();

	// ���ο� Ŭ���̾�Ʈ ����� ������ �����Լ�
	bool ClientHandler(SOCKET sock);
private:
	WORD			mVersion;
	WSADATA			mWSAData;

	SOCKET			mServerSocket;
	
	SocketList		mAllSocketList;
	SocketMap		mAllSocketMap;
	SocketList		mLoomSocket;
	
	sockaddr_in		mSocketAddr;
};

