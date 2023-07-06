#pragma once
#include "def.h"
#include <string>
#include <WinSock2.h>


// * 주의 *
// 클라이언트에서 임의로 데이터를 변경하면 안된다
// 서버의 패킷 헤더를 열어서 변경하세요

namespace Server
{
	struct Vec3
	{
		float x;
		float y;
		float z;

		Vec3(){};
		Vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {};
	};


	enum class ServerDataType : int
	{
		LoginData,
		LogoutData,

		ChatMessege,
		WhisperMessege,
		OtherPlayerData,
		RigidbodyData,
		ColliderData
	};

	struct Login_Packet
	{
		ServerDataType type;
		SOCKET sock;

		std::string name;
		std::string password;
	};

	struct Logout_Packet
	{
		ServerDataType type;
		SOCKET sock;

		std::string name;
	};

	struct ChatMassege_Packet
	{
		ServerDataType type;

		std::string name;
		std::string Messege;
	};

	struct WhisperMessege_Packet
	{
		ServerDataType type;

		std::string name;
		std::string Messege;

		std::string otherName;
	};

	struct OtherPlayer_Packet
	{
		ServerDataType type;
		SOCKET sock;

		Vec3 position;
		int animationIdx;
		std::string animationName;
	};
}