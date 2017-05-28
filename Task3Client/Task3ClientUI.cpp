#pragma comment(lib, "Ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <vector>
#include <stdio.h>
#include <process.h>
#include <winsock2.h>

#define MESSAGE_SIZE 8

#define IN_PORT 3555
#define OUT_PORT 3556

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

	BOOL flag = 1;
	if (setsockopt(s, SOL_SOCKET, SO_BROADCAST, (const char*)&flag, sizeof(flag)))
	{
		closesocket(s);
		printf("Error! Can't enable broadcast for socket\n");
		return INVALID_SOCKET;
	}

	return s;
}

BOOL BroadcastProbe(SOCKET s)
{
	printf("broadcasting probe...\n");

	char buf[MESSAGE_SIZE] = "probe";

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(OUT_PORT);
	addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

	if (sendto(s, &buf[0], sizeof(buf), 0, (sockaddr*)&addr, sizeof(addr)) < sizeof(buf))
	{
		printf("Error while broadcasting probe!\n");
		return FALSE;
	}
	return TRUE;
}

BOOL ReceiveReply(SOCKET s)
{
	sockaddr_in addr;
	int len = sizeof(addr);
	char buf[MESSAGE_SIZE];
	if (recvfrom(s, &buf[0], sizeof(buf), 0, (sockaddr*)&addr, &len) < sizeof(buf))
	{
		printf("Error! Can't recieve the message\n");
		return FALSE;
	}
	printf("Found server on %s\n", inet_ntoa(addr.sin_addr));
	return TRUE;
}

void ReceiveThread(void*)
{
	SOCKET ins = CreateInSocket();
	if (ins == INVALID_SOCKET)
	{
		return;
	}

	while (1)
	{
		if (!ReceiveReply(ins))
		{
			break;
		}
	}
}

int main()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR)
	{
		printf("Error! WSAStartup failed\n");
		return 1;
	}

	SOCKET outs = CreateOutSocket();

	if (outs == INVALID_SOCKET)
	{
		return -1;
	}

	_beginthread(ReceiveThread, 0, NULL);

	while (1)
	{
		BroadcastProbe(outs);
		Sleep(10000);
	}

	WSACleanup();
	return 0;
}
