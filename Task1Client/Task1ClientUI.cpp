#pragma comment(lib, "WSock32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <WinSock2.h>

using namespace std;

const int N = 10;
const int M = 10;
const char* HOSTNAME = "localhost";
const int PORT = 15001;

ofstream out("messages.txt");

int main() 
{
	WSADATA wsaData;
	SOCKET sock;
	HOSTENT* host;

	if (WSAStartup(0x0202, &wsaData)) 
	{
		cerr << "Client initialization error!\n";
		return -1;
	}

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock == INVALID_SOCKET)
	{
		WSACleanup();
		cerr << "Client socket error!\n" << endl;
		WSACleanup();
		return -1;
	}

	host = gethostbyname(HOSTNAME);
	if (!host) 
	{
		closesocket(sock);
		cerr << "Client host error!\n" << endl;
		WSACleanup();
		return -1;
	}


	char buff[M + 1];

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	memset(&addr.sin_addr, 0, sizeof(addr.sin_addr));
	memcpy(&addr.sin_addr, host->h_addr, host->h_length);

	for (int i = 0; i < N; i++) 
	{
		for (int j = 0; j < M; j++) 
		{
			buff[j] = 'a';
		}
		buff[M] = '\0';

		out << buff << endl;		
		
		int len = strlen(buff);
		cout << len << endl;
		if (sendto(sock, buff, len, 0, (sockaddr*)&addr, sizeof(addr)) < len) 
		{
			cerr << "Client send error " << WSAGetLastError() << endl;
			return -1;
		}
		Sleep(100);
	}

	return 0;
}
