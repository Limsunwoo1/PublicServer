#include "SimpleServer.h"

int main()
{
	GETSINGLE(SimpleServer)->Initalize();
	GETSINGLE(SimpleServer)->Accept();
	GETSINGLE(SimpleServer)->EXIT();
}
