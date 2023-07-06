#include <iostream>
#include <thread>
#include "Engine.h"
#include "CServerManager.h"


int main()
{
	// 서버와의 연결
	GETSINGLE(Server::ServerManager)->Initalize();

	// window Api 위에선 종료 시점에 호출 하면 된다.
	GETSINGLE(Server::ServerManager)->Rlease();
	return 0;
}