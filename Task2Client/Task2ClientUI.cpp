#pragma comment(lib, "Ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <vector>
#include <stdio.h>
#include <process.h>
#include <string>
#include <iostream>
#include <winsock2.h>

using namespace std;

#pragma comment(lib, "Ws2_32.lib")

#define MESSAGE_SIZE 4096

#define IN_PORT 1556
#define OUT_PORT 1555

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

BOOL SendMessage(SOCKET s, in_addr to, const std::string& text)
{
	char buf[MESSAGE_SIZE];
	strcpy(buf, text.c_str());

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(OUT_PORT);
	addr.sin_addr = to;

	if (sendto(s, &buf[0], sizeof(buf), 0, (sockaddr*)&addr, sizeof(addr)) < sizeof(buf))
	{
		printf("Error while sending message!\n");
		return FALSE;
	}
	return TRUE;
}

BOOL ReceiveMessage(SOCKET s, std::string& text)
{
	sockaddr_in addr;
	int len = sizeof(addr);
	char buf[MESSAGE_SIZE];
	if (recvfrom(s, &buf[0], sizeof(buf), 0, (sockaddr*)&addr, &len) < sizeof(buf))
	{
		printf("Error! Can't recieve the message\n");
		return FALSE;
	}

	text = buf;
	return TRUE;
}

void ReceiveThread(void*)
{
	SOCKET ins = CreateInSocket();
	if (ins == INVALID_SOCKET)
		return;

	while (1)
	{
		std::string text;
		if (!ReceiveMessage(ins, text))
			break;
		printf("%s\n", text.c_str());
	}
}

int main()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR)
	{
		printf("Client error! WSAStartup failed\n");
		return -1;
	}

	string host;
	cout << "Enter host name (localhost): ";
	getline(std::cin, host);

	if (host.empty())
	{
		host = "localhost";
	}

	hostent* he = NULL;
	if (isalpha(host[0]))
	{
		he = gethostbyname(host.c_str());
	}
	else
	{
		unsigned int addr = inet_addr(host.c_str());
		he = gethostbyaddr((char*)&addr, 4, AF_INET);
	}
	if (he != NULL)
	{
		struct in_addr addr;
		memset(&addr, 0, sizeof(addr));
		memcpy(&addr, he->h_addr, he->h_length);

		SOCKET outs = CreateOutSocket();

		if (outs == INVALID_SOCKET)
		{
			return -1;
		}

		_beginthread(ReceiveThread, 0, NULL);

		while (1)
		{			
			string text;
			getline(std::cin, text);
			if (text.empty())
			{
				break;
			}
			if (!SendMessage(outs, addr, text))
			{
				break;
			}
		}
	}
	else
	{
		cerr << "Error! Can't resolve host name";
	}

	WSACleanup();
	return 0;
}

