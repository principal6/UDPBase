#pragma once

#pragma comment (lib, "WS2_32.lib")

#include <WinSock2.h>
#include <iostream>
#include <WS2tcpip.h>
#include <unordered_set>

union UClientAddr
{
	UClientAddr() {}
	UClientAddr(ULONG _IPv4, USHORT _Port) : IPv4{ _IPv4 }, Port{ _Port } {}
	UClientAddr(ULONGLONG _Data) : Data{ _Data } {}
	ULONGLONG Data{};
	struct { ULONG IPv4; USHORT Port; };
};

class CServer
{
public:
	CServer(USHORT Port, timeval TimeOut) : m_TimeOut{ TimeOut }
	{
		StartUp();
		CreateSocket();
		SetHostAddr(Port);
		BindSocket();
		DisplayHostAddr();
	}
	~CServer()
	{
		CloseSocket();
		CleanUp(); 
	}

public:
	void Receive()
	{
		fd_set Copy{ m_SetToSelect };
		if (select(0, &Copy, nullptr, nullptr, &m_TimeOut) > 0)
		{
			SOCKADDR_IN ClientAddr{};
			int ClientAddrLen{ (int)sizeof(ClientAddr) };
			int ReceivedByteCount{ recvfrom(m_Socket, m_Buffer, KBufferSize, 0, (sockaddr*)&ClientAddr, &ClientAddrLen) };
			if (ReceivedByteCount > 0)
			{
				UClientAddr ClientInfo{ ClientAddr.sin_addr.S_un.S_addr, ClientAddr.sin_port };
				if (m_usClientAddrs.find(ClientInfo.Data) == m_usClientAddrs.end())
				{
					m_usClientAddrs.insert(ClientInfo.Data);
				}

				const auto& IPv4{ ClientAddr.sin_addr.S_un.S_un_b };
				printf("[%d.%d.%d.%d:%d] [%d bytes]: %.*s\n",
					IPv4.s_b1, IPv4.s_b2, IPv4.s_b3, IPv4.s_b4, ntohs(ClientAddr.sin_port), ReceivedByteCount, ReceivedByteCount, m_Buffer);

				// broadcast
				SendToAll(m_Buffer, ReceivedByteCount);
			}
		}
	}

	bool SendTo(const SOCKADDR_IN* Addr, const char* Buffer, int BufferSize = -1) const
	{
		if (!Addr || !Buffer) return false;
		if (BufferSize < 0) BufferSize = (int)strlen(Buffer);
		int SentByteCount{ sendto(m_Socket, Buffer, BufferSize, 0, (sockaddr*)Addr, sizeof(*Addr)) };
		if (SentByteCount > 0) return true;
		
		const auto& IPv4{ Addr->sin_addr.S_un.S_un_b };
		printf("Failed to send to CLIENT[%d.%d.%d.%d:%d]: %s\n",
			IPv4.s_b1, IPv4.s_b2, IPv4.s_b3, IPv4.s_b4, ntohs(Addr->sin_port), Buffer);
		return false;
	}

	bool SendToAll(const char* Buffer, int BufferSize = -1) const
	{
		bool bFailedAny{};
		SOCKADDR_IN Addr{};
		Addr.sin_family = AF_INET;
		for (auto ClientAddr : m_usClientAddrs)
		{
			UClientAddr ClientInfo{ ClientAddr };
			Addr.sin_addr.S_un.S_addr = ClientInfo.IPv4;
			Addr.sin_port = ClientInfo.Port;
			if (!SendTo(&Addr, Buffer, BufferSize))
			{
				bFailedAny = true;
			}
		}
		return !bFailedAny;
	}

public:
	void DisplayHostAddr()
	{
		auto& IPv4{ m_HostAddr.sin_addr.S_un.S_un_b };
		printf("Server Info [%d.%d.%d.%d:%d]\n", IPv4.s_b1, IPv4.s_b2, IPv4.s_b3, IPv4.s_b4, ntohs(m_HostAddr.sin_port));
	}

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

	void SetHostAddr(USHORT Port)
	{
		char HostName[256]{};
		if (gethostname(HostName, 256) == SOCKET_ERROR)
		{
			printf("Failed to get host name: %d\n", WSAGetLastError());
			return;
		}

		ADDRINFO Hint{};
		Hint.ai_family = PF_INET; // IPv4
		ADDRINFO* AddrInfo{};
		int Error{ getaddrinfo(HostName, nullptr, &Hint, &AddrInfo) };
		if (Error)
		{
			printf("Failed to get address info: %d\n", Error);
			return;
		}
		memcpy(&m_HostAddr, AddrInfo->ai_addr, sizeof(m_HostAddr));
		m_HostAddr.sin_port = htons(Port);
		freeaddrinfo(AddrInfo);
	}

	void BindSocket()
	{
		if (bind(m_Socket, (sockaddr*)&m_HostAddr, sizeof(m_HostAddr)) == SOCKET_ERROR)
		{
			printf("Failed to bind socket: %d\n", WSAGetLastError());
		}
	}

	void CloseSocket()
	{
		if (m_Socket && closesocket(m_Socket) == SOCKET_ERROR)
		{
			printf("Failed to close socket: %d\n", WSAGetLastError());
		}
		m_Socket = 0;
	}

	void CleanUp()
	{
		int Error{ WSACleanup() };
		if (Error)
		{
			printf("Failed to clean up: %d\n", Error);
		}
	}

private:
	static constexpr int KBufferSize{ 2048 };

private:
	bool m_bTerminating{};

private:
	SOCKET m_Socket{};

private:
	SOCKADDR_IN m_HostAddr{};

private:
	char m_Buffer[KBufferSize]{};

private:
	timeval m_TimeOut{};
	fd_set m_SetToSelect{};

private:
	std::unordered_set<ULONGLONG> m_usClientAddrs{};
};
