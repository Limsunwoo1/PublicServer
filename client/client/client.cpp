#include <iostream>
#include <thread>
#include "Engine.h"
#include "CServerManager.h"


int main()
{
	// �������� ����
	GETSINGLE(Server::ServerManager)->Initalize();

	// window Api ������ ���� ������ ȣ�� �ϸ� �ȴ�.
	GETSINGLE(Server::ServerManager)->Rlease();
	return 0;
}