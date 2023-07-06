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

		// ���� �ּҸ� �Է¹޴µ� ( Local �Է½� ���� ȣ��Ʈ ip�� �Էµȴ� )
		std::cout << "���� �� ���� IP �ּҸ� �Է����ּ��� : ";
		char serverIP[_MAX_PATH] = {};
		gets_s(serverIP);
		mServerIP = std::string(serverIP);

		// ���� �ּ� ����
		if (mServerIP.find("local") != std::string::npos
			|| mServerIP.find("LOCAL") != std::string::npos
			|| mServerIP.find("Local") != std::string::npos)
		{
			mServerIP = HOST_IP;
		}

		mServerAddr.sin_family = AF_INET;
		mServerAddr.sin_port = htons(PORT_NUMBER);

		// IPv4 Ȥ�� IPv6 char �� �ּҸ� �������� ��ȯ
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
		// ������ �����û
		socklen_t len = (socklen_t)sizeof(mServerAddr);
		int iConnect = connect(mSocket, (sockaddr*)&mServerAddr, len);
		if (iConnect == SOCKET_ERROR)
		{
			std::cout << "ConnectError!" << std::endl;

			closesocket(mSocket);
			WSACleanup();

			exit(1);
		}

		// ĳ������ ID ��
		// ������ ���̽��� ������ �α��� ó���� �ϸ� ����
		std::cout << "ID �� �Է��ϼ��� : ";
		char nameData[MAX_NAME_SIZE] = {};
		gets_s(nameData);
		mClientName = std::string(nameData);


		std::cout << "Client Connect.." << std::endl;

		// �α��� ��Ŷ ���� ���� �Լ�
		std::function<void()> loginFun = [this]() {
			Server::Login_Packet packet = {};
			packet.type = Server::ServerDataType::LoginData;
			packet.name = GetClientName();
			packet.sock = mSocket;
			
			PushSend((void*)&packet);
		};
		loginFun();


		// ������ �����͸� ���Ź޴� ������ ����
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
		// ������ ���ϵ����� ���� ����
		char buf[1024] = {};

		int iRecv = recv(mSocket, reinterpret_cast<char*>(buf), MAX_DATA_SIZE, 0);
		if (iRecv == SOCKET_ERROR)
		{
			// WSAGetLastError - ������ ���� �ڵ带 ��ȯ�Ѵ�
			int test = WSAGetLastError();
			std::cout << test << std::endl;
			std::cout << "ReceiveError!" << std::endl;

			closesocket(mSocket);
			WSACleanup();

			exit(1);
		}

		if (iRecv != 0)
		{
			// ������ �����͸� ó���ϴ� �κ�
			// ������ Ÿ�� ������ �پ��� ������� ó������

			std::thread receiveThread([&buf, this]()
				{
					int* dataType = reinterpret_cast<int*>(buf);
					int type = *dataType;

					// �Լ������� Ȥ�� ������� ���� ���� �����ϰ� ����ϸ�ȴ�
					switch ((ServerDataType)type)
					{
					case ServerDataType::LoginData:
					{
						Login_Packet* chatData = reinterpret_cast<Login_Packet*>(buf);
						std::cout << std::endl;
						std::cout << chatData->name << " ���� �����Ͽ����ϴ�" << std::endl;

						// �ٸ� Ŭ���̾��� ����� �������� ���� ������Ʈ�� �߰��ϸ�ȴ�.

					}
					break;
					case ServerDataType::LogoutData:
					{
						Logout_Packet* chatData = reinterpret_cast<Logout_Packet*>(buf);
						std::cout << std::endl;
						std::cout << chatData->name << " ���� �����Ͽ����ϴ�" << std::endl;
						std::cout << "�޼��� �Է� : ";

						// �ٸ� Ŭ���̾��� ����� �������� ���� ������Ʈ�� �����ϸ�ȴ�.
					}
					break;
					case ServerDataType::ChatMessege:
					{
						ChatMassege_Packet* chatData = reinterpret_cast<ChatMassege_Packet*>(buf);
						std::cout << std::endl;
						std::cout << chatData->name << " ���� �޼��� : " << chatData->Messege << std::endl;
						std::cout << "�޼��� �Է� : ";
					}
					break;

					case ServerDataType::WhisperMessege:
					{
						WhisperMessege_Packet* whisperData = reinterpret_cast<WhisperMessege_Packet*>(buf);
						std::cout << std::endl;
						std::cout << whisperData->name << " ���� �ӼӸ� �޼��� : " << whisperData->Messege << std::endl;
						std::cout << "�޼��� �Է� : ";
					}
					break;

					case ServerDataType::OtherPlayerData:
					{
						OtherPlayer_Packet* positionPacket = reinterpret_cast<OtherPlayer_Packet*>(buf);

						// Receive ���� �ٸ� Ŭ���̾�Ʈ�� ������
						// ���� ������Ʈ�� ���� �ؾ��Ѵ�
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
		// �Ű������� �ܺο��� ��Ŷ������ �Է��Ѵ�
		// ��Ŷ������ ù��° �������� ServerDataType ������ �������
		// ��Ʈ����ȯ���ֿ� ĳ��Ʈ
		int* type = reinterpret_cast<int*> (data);

		// ���� ������ ������ ������
		// ���� ������ �ִٸ� ����ϰ� ��ȯ�ص� ���� 
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

		// ��Ŷ�� ������ ����
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
		// ������ ����ɶ� �α׾ƿ� ��Ŷ����
		Logout_Packet packet = {};
		packet.type = ServerDataType::LogoutData;
		packet.name = GetClientName();
		packet.sock = mSocket;

		PushSend((void*)&packet);

		Clear();
	}

	void ServerManager::Clear()
	{
		// ���ϰ� ���϶��̺귯�� �޸� ����
		closesocket(mSocket);
		WSACleanup();
	}
}