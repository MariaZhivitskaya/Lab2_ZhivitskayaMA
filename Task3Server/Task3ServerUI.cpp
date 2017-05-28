#pragma comment(lib, "Ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <vector>
#include <stdio.h>
#include <winsock2.h>

#define MESSAGE_SIZE 8

#define IN_PORT 3556
#define OUT_PORT 3555

SOCKET CreateInSocket()
{
	SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s == INVALID_SOCKET)
	{
		printf("Error! Can't create incoming socket\n");
		return INVALID_SOCKET;
	}

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(IN_PORT);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(s, (sockaddr*)&addr, sizeof(addr)))
	{
		closesocket(s);
		printf("Error! Can't bind incoming socket to address\n");
		return INVALID_SOCKET;
	}

	return s;
}

SOCKET CreateOutSocket()
{
	SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s == INVALID_SOCKET)
	{
		printf("Error! Can't create outgoing socket\n");
		return INVALID_SOCKET;
	}

	return s;
}

BOOL SendReply(SOCKET s, in_addr to)
{
	char buf[MESSAGE_SIZE] = "ok";

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(OUT_PORT);
	addr.sin_addr = to;

	if (sendto(s, &buf[0], sizeof(buf), 0, (sockaddr*)&addr, sizeof(addr)) < sizeof(buf))
	{
		printf("Error! Can't send the reply\n");
		return FALSE;
	}
	return TRUE;
}

BOOL ReceiveProbe(SOCKET s, in_addr& from)
{
	sockaddr_in addr;
	int len = sizeof(addr);
	char buf[MESSAGE_SIZE];
	if (recvfrom(s, &buf[0], sizeof(buf), 0, (sockaddr*)&addr, &len) < sizeof(buf))
	{
		printf("Error! Can't recieve\n\n");
		return FALSE;
	}
	printf("received probe from %s\n", inet_ntoa(addr.sin_addr));
	from = addr.sin_addr;
	return TRUE;
}

int main(int argc, const char * argv[])
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR)
	{
		printf("Error! WSAStartup failed\n");
		return 1;
	}

	SOCKET ins = CreateInSocket();
	SOCKET outs = CreateOutSocket();

	if (ins == INVALID_SOCKET || outs == INVALID_SOCKET)
	{
		return -1;
	}

	while (1)
	{
		in_addr addr;
		if (!ReceiveProbe(ins, addr))
		{
			break;
		}
		if (!SendReply(outs, addr))
		{
			break;
		}
	}

	WSACleanup();
	return 0;
}
