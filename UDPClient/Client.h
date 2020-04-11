#pragma once

#pragma comment (lib, "WS2_32.lib")

#include <WinSock2.h>
#include <iostream>
#include <WS2tcpip.h>

class CClient
{
public:
	CClient(const char* ServerIP, u_short Port, timeval TimeOut) : m_TimeOut{ TimeOut }
	{
		StartUp(); 
		CreateSocket();
		SetServerAddr(ServerIP, Port);
	}
	~CClient() 
	{
		CloseSocket();
		CleanUp(); 
	}

public:
	void SetServerAddr(const char* ServerIP, u_short Port)
	{
		m_ServerAddr.sin_family = AF_INET;
		inet_pton(AF_INET, ServerIP, &m_ServerAddr.sin_addr);
		m_ServerAddr.sin_port = htons(Port);
	}

public:
	bool Send(const char* Buffer, int BufferSize = -1) const
	{
		if (!Buffer) return false;
		if (BufferSize < 0) BufferSize = (int)strlen(Buffer);
		int SentByteCount{ sendto(m_Socket, Buffer, BufferSize, 0, (sockaddr*)&m_ServerAddr, sizeof(m_ServerAddr)) };
		if (SentByteCount > 0) return true;

		printf("Failed to send [%d bytes]: %s\n", BufferSize, Buffer);
		return false;
	}

	bool Receive()
	{
		fd_set Copy{ m_SetToSelect };
		if (select(0, &Copy, nullptr, nullptr, &m_TimeOut) > 0)
		{
			m_TimeOutCounter = 0;
			int ServerAddrLen{ (int)sizeof(m_ServerAddr) };
			int ReceivedByteCount{ recvfrom(m_Socket, m_Buffer, KBufferSize, 0, (sockaddr*)&m_ServerAddr, &ServerAddrLen) };
			if (ReceivedByteCount > 0)
			{
				printf("[%d bytes]: %.*s\n", ReceivedByteCount, ReceivedByteCount, m_Buffer);
				return true;
			}
			return false;
		}
		else
		{
			++m_TimeOutCounter;
			return false;
		}
	}

public:
	bool IsTimedOut() { return (m_TimeOutCounter >= KTimeOutCountLimit); }
	void Terminate() { m_bTerminating = true; }
	bool IsTerminating() const { return m_bTerminating; }

private:
	void StartUp()
	{
		WSADATA Data{};
		int Error{ WSAStartup(MAKEWORD(2, 2), &Data) };
		if (Error)
		{
			printf("Failed to start up: %d\n", Error);
			return;
		}
	}

	void CreateSocket()
	{
		m_Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (m_Socket == INVALID_SOCKET)
		{
			printf("Failed to create socket: %d\n", WSAGetLastError());
			m_Socket = 0;
			return;
		}

		FD_SET(m_Socket, &m_SetToSelect);
	}

	void CloseSocket()
	{
		if (m_Socket && closesocket(m_Socket) == SOCKET_ERROR)
		{
			printf("Failed to close socket: %d", WSAGetLastError());
		}
		m_Socket = 0;
	}

	void CleanUp()
	{
		int Error{ WSACleanup() };
		if (Error)
		{
			printf("Failed to clean up: %d", Error);
		}
	}

private:
	static constexpr int KBufferSize{ 2048 };
	static constexpr int KTimeOutCountLimit{ 10 };

private:
	bool m_bTerminating{};

private:
	SOCKET m_Socket{};

private:
	SOCKADDR_IN m_ServerAddr{};

private:
	char m_Buffer[KBufferSize]{};

private:
	int m_TimeOutCounter{};
	timeval m_TimeOut{};
	fd_set m_SetToSelect{};
};
