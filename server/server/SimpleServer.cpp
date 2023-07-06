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
	// 소켓 라이브러리 버젼 2.2
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
	// 소켓 생성
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
	// 아이피 주소 설정 (INADDR_ANY 키워드로 모든 네트워크 인터페이스에서 들어오는 연결을 수락)
	mSocketAddr.sin_family = AF_INET;
	mSocketAddr.sin_port = htons(PORT_NUMBER);
	mSocketAddr.sin_addr.S_un.S_addr = INADDR_ANY;

	// 소켓 바인딩
	if (bind(mServerSocket, (sockaddr*)&mSocketAddr, sizeof(mSocketAddr)) == SOCKET_ERROR)
	{
		std::cout << "BindFail...!" << std::endl;

		EXIT();
	}

	std::cout << "BindAccess..!" << std::endl;
}

void SimpleServer::ListenSocket()
{
	// 소켓이 클라이언트의 수신을 받을 준비
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

		// 클라이언트의 연결 요청을 받는함수
		clientSocket = accept(mServerSocket, (sockaddr*)&clientAddr, &clientSize);
		mAllSocketList.emplace_back(clientSocket);

		if (clientSocket == INVALID_SOCKET)
		{
			int error = WSAGetLastError();
			std::cout << error << std::endl;

			closesocket(clientSocket);

			exit(1);
		}

		// 새로운 클라이언트 연결시 스레드 실행
		std::thread clientHandle([this, &clientSocket]()
			{
				std::cout << clientSocket << " - 소켓번호 " << std::endl;
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
		std::cout << sock << " - 소켓의 연결을 종료 합니다.." << std::endl;

		// 소켓컨테이너들의 해당 소켓들을 지워준다
		SocketList::iterator listiter = std::find(mAllSocketList.begin(), mAllSocketList.end(), sock);
		if (listiter != mAllSocketList.end())
		{

			mAllSocketList.erase(listiter);
		}

		for (auto& mapIter : mAllSocketMap)
		{
			if (mapIter.second == sock)
			{
				// 맵에 find 로 찾은iterator로 erase
				mAllSocketMap.erase(mAllSocketMap.find(mapIter.first));
				break;
			}
		}

		return SOCKET_ERROR;
	}

	// 패킷의 데이터 타입 추출
	int* dataType = reinterpret_cast<int*>(data);
	int type = *dataType;


	// 데이터 타입에 따른 기능구현
	// 구현부는 다른 구조로 재설계해도 된다
	switch ((Server::ServerDataType)type)
	{
	case Server::ServerDataType::LoginData:
	{
		Server::Login_Packet* packet = reinterpret_cast<Server::Login_Packet*>(data);
		mAllSocketMap.insert(make_pair(packet->name, sock));

		// 스레드로 함수 실행
		std::thread ChatThread([this, packet, sock, data, dataLen]() {
			std::string name = packet->name;
			std::cout << name << " 님이 입장하였습니다" << std::endl;

			// 지금 구조는 먼저 연결된 클라이언트의 정보를 모른다
			// 따라서 연결된 클라이언트의 정보들을 마지막 연결된 소켓에 보내줘야한다
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

		// 컨테이너의 해당 소켓을 지워준다
		if (listiter != mAllSocketList.end())
		{
			mAllSocketList.erase(listiter);
		}

		SocketMap::iterator mapiter = mAllSocketMap.find(packet->name);
		if (mapiter != mAllSocketMap.end())
		{
			mAllSocketMap.erase(mapiter);
		}

		std::cout << packet->name << " 님이 퇴장하였습니다" << std::endl;

		// 다른 클라이언트의 현제 소켓의 로그아웃 정보를 수신
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

		std::cout << packet->name << " 님의 메세지 : " << packet->Messege << std::endl;

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
			std::cout << packet->name << " 님의 귓속말 메세지 : " << packet->Messege << std::endl;
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
