#include "CServerManager.h"

#include <functional>


namespace Server
{
	ServerManager::ServerManager()
	{
	}

	ServerManager::~ServerManager()
	{
	}

	void ServerManager::Initalize()
	{
		mVersion = MAKEWORD(2, 2);
		if (WSAStartup(mVersion, &mWSData) != 0)
		{
			std::cout << "InitError!" << std::endl;

			WSACleanup();
			exit(1);
		}

		SocketCreate();
		ConvertIP();
		Connect();
	}

	void ServerManager::SocketCreate()
	{
		mSocket = socket(AF_INET, SOCK_STREAM, 0);
		if (mSocket == INVALID_SOCKET)
		{
			std::cout << "SocketCreateError!" << std::endl;

			WSACleanup();
			exit(1);
		}
	}

	void ServerManager::ConvertIP()
	{
		mServerIP = "";

		// 서버 주소를 입력받는데 ( Local 입력시 로컬 호스트 ip가 입력된다 )
		std::cout << "연결 될 서버 IP 주소를 입력해주세요 : ";
		char serverIP[_MAX_PATH] = {};
		gets_s(serverIP);
		mServerIP = std::string(serverIP);

		// 서버 주소 세팅
		if (mServerIP.find("local") != std::string::npos
			|| mServerIP.find("LOCAL") != std::string::npos
			|| mServerIP.find("Local") != std::string::npos)
		{
			mServerIP = HOST_IP;
		}

		mServerAddr.sin_family = AF_INET;
		mServerAddr.sin_port = htons(PORT_NUMBER);

		// IPv4 혹은 IPv6 char 형 주소를 이진수로 변환
		INT error = inet_pton(AF_INET, mServerIP.c_str(), &(mServerAddr.sin_addr));

		if (!error)
		{
			std::cout << "ConvertError!" << std::endl;

			closesocket(mSocket);
			WSACleanup();
		}
	}

	void ServerManager::Connect()
	{
		// 서버에 연결요청
		socklen_t len = (socklen_t)sizeof(mServerAddr);
		int iConnect = connect(mSocket, (sockaddr*)&mServerAddr, len);
		if (iConnect == SOCKET_ERROR)
		{
			std::cout << "ConnectError!" << std::endl;

			closesocket(mSocket);
			WSACleanup();

			exit(1);
		}

		// 캐릭터의 ID 값
		// 데이터 베이스와 연결해 로그인 처리를 하면 좋다
		std::cout << "ID 를 입력하세요 : ";
		char nameData[MAX_NAME_SIZE] = {};
		gets_s(nameData);
		mClientName = std::string(nameData);


		std::cout << "Client Connect.." << std::endl;

		// 로그인 패킷 전송 람다 함수
		std::function<void()> loginFun = [this]() {
			Server::Login_Packet packet = {};
			packet.type = Server::ServerDataType::LoginData;
			packet.name = GetClientName();
			packet.sock = mSocket;
			
			PushSend((void*)&packet);
		};
		loginFun();


		// 서버의 데이터를 수신받는 스레드 실행
		std::thread receivethread([this]()
			{
				while (1)
				{
					Receive();
				}
			});

		receivethread.detach();
	}

	void ServerManager::Receive()
	{
		// 서버에 소켓데이터 버퍼 수신
		char buf[1024] = {};

		int iRecv = recv(mSocket, reinterpret_cast<char*>(buf), MAX_DATA_SIZE, 0);
		if (iRecv == SOCKET_ERROR)
		{
			// WSAGetLastError - 서버의 오류 코드를 반환한다
			int test = WSAGetLastError();
			std::cout << test << std::endl;
			std::cout << "ReceiveError!" << std::endl;

			closesocket(mSocket);
			WSACleanup();

			exit(1);
		}

		if (iRecv != 0)
		{
			// 서버의 데이터를 처리하는 부분
			// 데이터 타입 내에서 다양한 방식으로 처리가능

			std::thread receiveThread([&buf, this]()
				{
					int* dataType = reinterpret_cast<int*>(buf);
					int type = *dataType;

					// 함수포인터 혹은 스레드로 실행 시켜 유연하게 사용하면된다
					switch ((ServerDataType)type)
					{
					case ServerDataType::LoginData:
					{
						Login_Packet* chatData = reinterpret_cast<Login_Packet*>(buf);
						std::cout << std::endl;
						std::cout << chatData->name << " 님이 입장하였습니다" << std::endl;

						// 다른 클라이언의 연결시 렌더링할 더미 오브젝트를 추가하면된다.

					}
					break;
					case ServerDataType::LogoutData:
					{
						Logout_Packet* chatData = reinterpret_cast<Logout_Packet*>(buf);
						std::cout << std::endl;
						std::cout << chatData->name << " 님의 퇴장하였습니다" << std::endl;
						std::cout << "메세지 입력 : ";

						// 다른 클라이언의 연결시 렌더링할 더미 오브젝트를 삭제하면된다.
					}
					break;
					case ServerDataType::ChatMessege:
					{
						ChatMassege_Packet* chatData = reinterpret_cast<ChatMassege_Packet*>(buf);
						std::cout << std::endl;
						std::cout << chatData->name << " 님의 메세지 : " << chatData->Messege << std::endl;
						std::cout << "메세지 입력 : ";
					}
					break;

					case ServerDataType::WhisperMessege:
					{
						WhisperMessege_Packet* whisperData = reinterpret_cast<WhisperMessege_Packet*>(buf);
						std::cout << std::endl;
						std::cout << whisperData->name << " 님의 귓속말 메세지 : " << whisperData->Messege << std::endl;
						std::cout << "메세지 입력 : ";
					}
					break;

					case ServerDataType::OtherPlayerData:
					{
						OtherPlayer_Packet* positionPacket = reinterpret_cast<OtherPlayer_Packet*>(buf);

						// Receive 받은 다른 클라이언트의 정보를
						// 더미 오브젝트에 세팅 해야한다
					}
					break;

					case ServerDataType::RigidbodyData:

						break;

					case ServerDataType::ColliderData:

						break;

					default:
						break;
					}
				});
			receiveThread.join();
		}
	}

	void ServerManager::PushSend(void* data)
	{
		// 매개변수로 외부에서 패킷정보를 입력한다
		// 패킷정보의 첫번째 데이터인 ServerDataType 정보를 얻기위해
		// 인트형반환값애에 캐스트
		int* type = reinterpret_cast<int*> (data);

		// 보낼 버퍼의 데이터 사이즈
		// 좋은 구조가 있다면 깔끔하게 변환해도 좋다 
		int bufSize = 0;
		switch ((ServerDataType)*type)
		{
		case ServerDataType::LoginData:			bufSize = sizeof(Login_Packet);				break;
		case ServerDataType::LogoutData:		bufSize = sizeof(Logout_Packet);			break;
		case ServerDataType::ChatMessege:		bufSize = sizeof(ChatMassege_Packet);		break;
		case ServerDataType::WhisperMessege:	bufSize = sizeof(WhisperMessege_Packet);	break;
		case ServerDataType::OtherPlayerData:	bufSize = sizeof(OtherPlayer_Packet);		break;
		case ServerDataType::RigidbodyData:													break;
		case ServerDataType::ColliderData:													break;
		};

		// 패킷을 서버의 수신
		INT isend = send(mSocket, reinterpret_cast<char*>(data), bufSize, 0);
		if (isend == SOCKET_ERROR)
		{
			std::cout << "SendError...!" << std::endl;

			closesocket(mSocket);
			WSACleanup();

			exit(1);
			return;
		}
	}

	void ServerManager::Rlease()
	{
		// 서버가 종료될때 로그아웃 패킷전송
		Logout_Packet packet = {};
		packet.type = ServerDataType::LogoutData;
		packet.name = GetClientName();
		packet.sock = mSocket;

		PushSend((void*)&packet);

		Clear();
	}

	void ServerManager::Clear()
	{
		// 소켓과 소켓라이브러리 메모리 해제
		closesocket(mSocket);
		WSACleanup();
	}
}