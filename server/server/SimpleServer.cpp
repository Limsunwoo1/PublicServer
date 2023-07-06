#include "SimpleServer.h"

SimpleServer::SimpleServer(){}
SimpleServer::~SimpleServer(){}

void SimpleServer::Initalize()
{
	WSAStartUp();
	CreateScoket();
	BindSocket();
	ListenSocket();
}

void SimpleServer::WSAStartUp()
{
	// ���� ���̺귯�� ���� 2.2
	mVersion = MAKEWORD(2, 2);
	INT iWSA = WSAStartup(mVersion, &mWSAData);
	if (iWSA != 0)
	{
		std::cout << "WSAStartup() Fail..!" << std::endl;
		exit(1);
	}

	std::cout << "WSAStartup() Access..!" << std::endl;
}

void SimpleServer::CreateScoket()
{
	// ���� ����
	mServerSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (mServerSocket == INVALID_SOCKET)
	{
		std::cout << "SocketCreateFail...!" << std::endl;
		EXIT();
	}

	std::cout << "SocketCreateAccess..!" << std::endl;
}

void SimpleServer::BindSocket()
{
	// ������ �ּ� ���� (INADDR_ANY Ű����� ��� ��Ʈ��ũ �������̽����� ������ ������ ����)
	mSocketAddr.sin_family = AF_INET;
	mSocketAddr.sin_port = htons(PORT_NUMBER);
	mSocketAddr.sin_addr.S_un.S_addr = INADDR_ANY;

	// ���� ���ε�
	if (bind(mServerSocket, (sockaddr*)&mSocketAddr, sizeof(mSocketAddr)) == SOCKET_ERROR)
	{
		std::cout << "BindFail...!" << std::endl;

		EXIT();
	}

	std::cout << "BindAccess..!" << std::endl;
}

void SimpleServer::ListenSocket()
{
	// ������ Ŭ���̾�Ʈ�� ������ ���� �غ�
	if (listen(mServerSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		std::cout << "ListenFail...!" << std::endl;

		EXIT();
	}

	std::cout << "Running..." << std::endl;
}

void SimpleServer::Accept()
{
	while (1)
	{
		sockaddr_in clientAddr;
		SOCKET clientSocket;
		socklen_t clientSize = (socklen_t)sizeof(clientAddr);

		// Ŭ���̾�Ʈ�� ���� ��û�� �޴��Լ�
		clientSocket = accept(mServerSocket, (sockaddr*)&clientAddr, &clientSize);
		mAllSocketList.emplace_back(clientSocket);

		if (clientSocket == INVALID_SOCKET)
		{
			int error = WSAGetLastError();
			std::cout << error << std::endl;

			closesocket(clientSocket);

			exit(1);
		}

		// ���ο� Ŭ���̾�Ʈ ����� ������ ����
		std::thread clientHandle([this, &clientSocket]()
			{
				std::cout << clientSocket << " - ���Ϲ�ȣ " << std::endl;
				ClientHandler(clientSocket);
			});
		clientHandle.detach();
	}
}

INT SimpleServer::SendData(SOCKET sock, void* data, int dataLen, int flags)
{
	INT iSend = send(sock, reinterpret_cast<char*>(data), dataLen, flags);
	if (iSend == SOCKET_ERROR)
	{
		std::cout << "SendError...!" << std::endl;
	}

	return iSend;
}

INT SimpleServer::ReceiveData(SOCKET sock, void* data, int dataLen, int flags)
{
	INT iReceive = recv(sock, reinterpret_cast<char*>(data), dataLen, flags);
	if (iReceive == SOCKET_ERROR)
	{
		std::cout << sock << " - ������ ������ ���� �մϴ�.." << std::endl;

		// ���������̳ʵ��� �ش� ���ϵ��� �����ش�
		SocketList::iterator listiter = std::find(mAllSocketList.begin(), mAllSocketList.end(), sock);
		if (listiter != mAllSocketList.end())
		{

			mAllSocketList.erase(listiter);
		}

		for (auto& mapIter : mAllSocketMap)
		{
			if (mapIter.second == sock)
			{
				// �ʿ� find �� ã��iterator�� erase
				mAllSocketMap.erase(mAllSocketMap.find(mapIter.first));
				break;
			}
		}

		return SOCKET_ERROR;
	}

	// ��Ŷ�� ������ Ÿ�� ����
	int* dataType = reinterpret_cast<int*>(data);
	int type = *dataType;


	// ������ Ÿ�Կ� ���� ��ɱ���
	// �����δ� �ٸ� ������ �缳���ص� �ȴ�
	switch ((Server::ServerDataType)type)
	{
	case Server::ServerDataType::LoginData:
	{
		Server::Login_Packet* packet = reinterpret_cast<Server::Login_Packet*>(data);
		mAllSocketMap.insert(make_pair(packet->name, sock));

		// ������� �Լ� ����
		std::thread ChatThread([this, packet, sock, data, dataLen]() {
			std::string name = packet->name;
			std::cout << name << " ���� �����Ͽ����ϴ�" << std::endl;

			// ���� ������ ���� ����� Ŭ���̾�Ʈ�� ������ �𸥴�
			// ���� ����� Ŭ���̾�Ʈ�� �������� ������ ����� ���Ͽ� ��������Ѵ�
			for (SOCKET _sock : mAllSocketList)
			{
				SendData(_sock, data, sizeof(Server::Login_Packet));
			}

			});
		ChatThread.join();
	}
	break;
	case Server::ServerDataType::LogoutData:
	{
		Server::Logout_Packet* packet = reinterpret_cast<Server::Logout_Packet*>(data);
		SocketList::iterator listiter = std::find(mAllSocketList.begin(), mAllSocketList.end(), sock);

		// �����̳��� �ش� ������ �����ش�
		if (listiter != mAllSocketList.end())
		{
			mAllSocketList.erase(listiter);
		}

		SocketMap::iterator mapiter = mAllSocketMap.find(packet->name);
		if (mapiter != mAllSocketMap.end())
		{
			mAllSocketMap.erase(mapiter);
		}

		std::cout << packet->name << " ���� �����Ͽ����ϴ�" << std::endl;

		// �ٸ� Ŭ���̾�Ʈ�� ���� ������ �α׾ƿ� ������ ����
		for (SOCKET _sock : mAllSocketList)
		{
			if (sock == _sock)
				continue;

			SendData(_sock, data, sizeof(Server::Logout_Packet));
		}
	}
	break;
	case Server::ServerDataType::ChatMessege:
	{
		Server::ChatMassege_Packet* packet = reinterpret_cast<Server::ChatMassege_Packet*>(data);

		std::cout << packet->name << " ���� �޼��� : " << packet->Messege << std::endl;

		for (SOCKET _sock : mAllSocketList)
		{
			if (_sock == sock)
				continue;

			SendData(_sock, data, sizeof(Server::ChatMassege_Packet));
		}
	}
	break;

	case Server::ServerDataType::WhisperMessege:
	{
		Server::WhisperMessege_Packet* packet = reinterpret_cast<Server::WhisperMessege_Packet*>(data);
		SocketMap::iterator iter = mAllSocketMap.find(packet->otherName);
		if (iter != mAllSocketMap.end())
		{
			std::cout << packet->name << " ���� �ӼӸ� �޼��� : " << packet->Messege << std::endl;
			send(iter->second, reinterpret_cast<char*>(data), dataLen, 0);
		}
	}
	break;
	case Server::ServerDataType::OtherPlayerData:
	{
		Server::OtherPlayer_Packet* packet = reinterpret_cast<Server::OtherPlayer_Packet*>(data);
		for (SOCKET _sock : mAllSocketList)
		{
			if (_sock == sock)
				continue;

			send(_sock, reinterpret_cast<char*>(data), sizeof(Server::OtherPlayer_Packet), 0);
		}
	}
	break;
	case Server::ServerDataType::RigidbodyData:
		break;
	case Server::ServerDataType::ColliderData:
		break;
	default:
		break;
	}

	ZeroMemory(data, dataLen, 0);

	return iReceive;
}

void SimpleServer::EXIT()
{
	std::cout << "EXIT..." << std::endl;

	mAllSocketList.clear();
	mAllSocketMap.clear();
	mLoomSocket.clear();

	closesocket(mServerSocket);
	WSACleanup();
}

bool SimpleServer::ClientHandler(SOCKET sock)
{
	char buf[1024] = {};

	while (1)
	{
		INT iReceive = ReceiveData(sock, buf, 1024, 0);

		if (iReceive == SOCKET_ERROR)
		{
			return false;
		}

		ZeroMemory(buf, sizeof(buf), 0);
	}

	return false;
}