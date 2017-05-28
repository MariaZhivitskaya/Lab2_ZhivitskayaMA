#pragma comment(lib, "Ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <vector>
#include <stdio.h>
#include <winsock2.h>

using namespace std;

#define MESSAGE_SIZE 4096

#define IN_PORT 1555
#define OUT_PORT 1556

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

struct Message
{
	in_addr from;
	string text;
};

BOOL BroadcastMessage(SOCKET s, const Message* message)
{
	string text;
	text.append(inet_ntoa(message->from));
	text.append(": ");
	text.append(message->text);

	char buf[MESSAGE_SIZE];
	strcpy(buf, text.c_str());

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(OUT_PORT);
	addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

	if (sendto(s, &buf[0], sizeof(buf), 0, (sockaddr*)&addr, sizeof(addr)) < sizeof(buf))
	{
		printf("Error while broadcasting message!\n");
		return FALSE;
	}
	return TRUE;
}

BOOL ReceiveMessage(SOCKET s, Message* message)
{
	message->from.s_addr = INADDR_NONE;
	message->text = "";

	sockaddr_in addr;
	int len = sizeof(addr);
	char buf[MESSAGE_SIZE];
	if (recvfrom(s, &buf[0], sizeof(buf), 0, (sockaddr*)&addr, &len) < sizeof(buf))
	{
		printf("Error! Can't recieve the message\n");
		return FALSE;
	}
	message->from = addr.sin_addr;
	message->text = buf;

	printf("Received packet from %s: %s\n", inet_ntoa(addr.sin_addr), buf);
	return TRUE;
}

int main()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR)
	{
		printf("Server error! WSAStartup failed\n");
		return -1;
	}

	SOCKET ins = CreateInSocket();
	SOCKET outs = CreateOutSocket();

	if (ins == INVALID_SOCKET || outs == INVALID_SOCKET)
	{
		printf("Server error!\n");
		return -1;
	}

	while (1)
	{
		Message message;
		if (!ReceiveMessage(ins, &message))
		{
			break;
		}
		if (!BroadcastMessage(outs, &message))
		{
			break;
		}
	}

	WSACleanup();
	return 0;
}

