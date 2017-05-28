#pragma comment(lib, "WSock32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <WinSock2.h>

using namespace std;

const int PORT = 15001;
const int MAX_BUFF_SIZE = 11;

ofstream out("messages.txt");

int main()
{
	WSADATA wsaData;
	SOCKET sock;	

	if (WSAStartup(0x0202, &wsaData))
	{
		cerr << "Server initialization error!\n";
		return -1;
	}

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock == INVALID_SOCKET) 
	{
		WSACleanup();
		cerr <<  "Server socket error!\n" << endl;
		WSACleanup();
		return -1;
	}
	
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(PORT);

	if (bind(sock, (sockaddr*)&addr, sizeof(addr))) 
	{
		closesocket(sock);
		cerr << "Server bind error!\n" << endl;
		WSACleanup();
		return -1;
	}

	char buff[MAX_BUFF_SIZE];
	while (true)
	{		
		int len = sizeof(addr);
		if (recvfrom(sock, buff, MAX_BUFF_SIZE, 0, (sockaddr*)&addr, &len) == SOCKET_ERROR)
		{
			cerr << "Server recieve error " << WSAGetLastError() << endl;
			return -1;
		}
		cout << inet_ntoa(addr.sin_addr) << ": " << buff << endl;
		out << buff << endl;
		out.flush();
	}

	closesocket(sock);
	WSACleanup();

	return 0;
}
