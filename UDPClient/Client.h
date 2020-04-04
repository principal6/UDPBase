#pragma once

#pragma comment (lib, "WS2_32.lib")

#include <WinSock2.h>
#include <iostream>
#include <WS2tcpip.h>

class CClient
{
public:
	CClient(const char* ServerIP, u_short Port, timeval TimeOut) : m_Port{ Port }, m_TimeOut{ TimeOut }
	{
		StartUp(); 
		Connect(ServerIP, Port);
	}
	~CClient() { CleanUp(); }

public:
	void Send(const char* Buffer, int BufferSize)
	{
		int SentByteCount{ sendto(m_Socket, Buffer, BufferSize, 0, (sockaddr*)&m_Server, sizeof(m_Server)) };
		if (SentByteCount > 0)
		{
			printf("Sent[%d]: %s\n", BufferSize, Buffer);
		}
	}

	void Listen()
	{
		fd_set fdSet;
		FD_ZERO(&fdSet);
		FD_SET(m_Socket, &fdSet);

		if (select(0, &fdSet, 0, 0, &m_TimeOut) > 0)
		{
			m_TimeOutCount = 0;
			int ServerAddrLen{ (int)sizeof(m_Server) };
			int ReceivedByteCount{ recvfrom(m_Socket, m_Buffer, KBufferSize, 0, (sockaddr*)&m_Server, &ServerAddrLen) };
			if (ReceivedByteCount > 0)
			{
				printf("From SERVER: %s\n", m_Buffer);
			}
		}
		else
		{
			++m_TimeOutCount;
		}
	}

	bool IsTimedOut()
	{
		return (m_TimeOutCount >= 3);
	}

private:
	void StartUp()
	{
		WSADATA wsaData{};
		int Result{ WSAStartup(MAKEWORD(2, 2), &wsaData) };
		if (Result)
		{
			printf("Failed to start up WSA: %d\n", Result);
			return;
		}
		m_bStartUp = true;
	}

	void Connect(const char* ServerIP, u_short Port)
	{
		if (m_Socket > 0)
		{
			printf("Already connected to server.\n");
			return;
		}

		m_Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (m_Socket == INVALID_SOCKET)
		{
			printf("Failed to connect: %d\n", WSAGetLastError());
			return;
		}
		m_bSocketCreated = true;

		ADDRINFO Hints{};
		Hints.ai_family = AF_INET;
		Hints.ai_socktype = SOCK_DGRAM;
		Hints.ai_protocol = IPPROTO_UDP;

		char szPort[255]{};
		_itoa_s(htons(m_Port), szPort, 10);

		ADDRINFO* Server{};
		getaddrinfo(ServerIP, szPort, &Hints, &Server);
		m_Server = *(SOCKADDR_IN*)(Server->ai_addr);
		m_Server.sin_port = htons(m_Server.sin_port);
	}

	void CleanUp()
	{
		if (m_bSocketCreated)
		{
			if (closesocket(m_Socket) == SOCKET_ERROR)
			{
				printf("Failed to close socket: %d", WSAGetLastError());
			}
			else
			{
				m_Socket = 0;
			}
		}
		if (m_bStartUp)
		{
			WSACleanup();
		}
	}

private:
	static constexpr int KBufferSize{ 2048 };

private:
	bool m_bStartUp{};
	bool m_bSocketCreated{};
	SOCKET m_Socket{};
	u_short m_Port{};
	SOCKADDR_IN m_Server{};

private:
	int m_TimeOutCount{};
	timeval m_TimeOut{};

private:
	char m_Buffer[KBufferSize]{};
};