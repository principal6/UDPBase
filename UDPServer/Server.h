#pragma once

#pragma comment (lib, "WS2_32.lib")

#include <WinSock2.h>
#include <iostream>
#include <vector>
#include <thread>

class CServer
{
public:
	CServer() { StartUp(); }
	~CServer() { CleanUp(); }

public:
	bool Open()
	{
		if (m_bGotHost)
		{
			m_Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			if (m_Socket == INVALID_SOCKET)
			{
				printf("%s socket(): %d\n", KFailureHead, WSAGetLastError());
				return false;
			}
			m_bSocketCreated = true;

			SOCKADDR_IN addr{};
			addr.sin_family = AF_INET;
			addr.sin_port = htons(KPort);
			addr.sin_addr = m_HostAddr.sin_addr;
			if (bind(m_Socket, (sockaddr*)&addr, sizeof(addr)))
			{
				printf("%s bind(): %d\n", KFailureHead, WSAGetLastError());
				return false;
			}

			m_bOpen = true;
			return true;
		}
		return false;
	}

	void Close()
	{
		m_bClosing = true;
	}

	bool IsClosing()
	{
		return m_bClosing;
	}

	void Listen()
	{
		if (!m_bOpen) return;
		
		timeval TimeOut{ 1, 0 };
		fd_set fdSet;
		FD_ZERO(&fdSet);
		FD_SET(m_Socket, &fdSet);

		int Result{ select(0, &fdSet, 0, 0, &TimeOut) };
		if (Result > 0)
		{
			SOCKADDR_IN Client{};
			int ClientAddrLen{ (int)sizeof(Client) };
			int received_bytes{ recvfrom(m_Socket, m_Buffer, KBufferSize, 0, (sockaddr*)&Client, &ClientAddrLen) };
			if (received_bytes > 0)
			{
				const auto& Bytes{ Client.sin_addr.S_un.S_un_b };
				printf("RECEIVED [%d.%d.%d.%d][%d]: %s\n", Bytes.s_b1, Bytes.s_b2, Bytes.s_b3, Bytes.s_b4, received_bytes, m_Buffer);

				// echo
				sendto(m_Socket, m_Buffer, received_bytes, 0, (sockaddr*)&Client, sizeof(Client));
			}
		}
	}

	void DisplayIP()
	{
		auto& Bytes{ m_HostAddr.sin_addr.S_un.S_un_b };
		printf("Server IP: %d.%d.%d.%d\n", Bytes.s_b1, Bytes.s_b2, Bytes.s_b3, Bytes.s_b4);
		printf("Service Port: %d\n", KPort);
	}

private:
	void StartUp()
	{
		WSADATA wsaData{};
		int Result{ WSAStartup(MAKEWORD(2, 2), &wsaData) };
		if (Result)
		{
			printf("%s WSAStartup(): %d\n", KFailureHead, Result);
			return;
		}
		m_bStartUp = true;

		m_bGotHost = GetHostIP();
	}

	bool GetHostIP()
	{
		SOCKET Socket{ socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP) };
		if (Socket == SOCKET_ERROR)
		{
			printf("%s socket(): %d\n", KFailureHead, WSAGetLastError());
			return false;
		}

		SOCKADDR_IN loopback{};
		loopback.sin_family = AF_INET;
		loopback.sin_addr.S_un.S_addr = INADDR_LOOPBACK;
		loopback.sin_port = htons(9);
		if (connect(Socket, (sockaddr*)&loopback, sizeof(loopback)))
		{
			printf("%s connect(): %d\n", KFailureHead, WSAGetLastError());
			return false;
		}

		int host_length{ (int)sizeof(m_HostAddr) };
		if (getsockname(Socket, (sockaddr*)&m_HostAddr, &host_length))
		{
			printf("%s getsockname(): %d\n", KFailureHead, WSAGetLastError());
			return false;
		}

		if (closesocket(Socket))
		{
			printf("%s closesocket(): %d\n", KFailureHead, WSAGetLastError());
		}
		return true;
	}

	void CleanUp()
	{
		if (m_bSocketCreated)
		{
			if (closesocket(m_Socket) == SOCKET_ERROR)
			{
				printf("%s closesocket(): %d\n", KFailureHead, WSAGetLastError());
			}
			else
			{
				m_bSocketCreated = false;
				m_Socket = 0;
			}
		}
		if (m_bStartUp)
		{
			int Result{ WSACleanup() };
			if (Result)
			{
				printf("%s WSACleanup(): %d\n", KFailureHead, Result);
			}
			else
			{
				m_bStartUp = false;
			}
		}
	}

private:
	static constexpr const char* KFailureHead{ "[Failed]" };
	static constexpr int KBufferSize{ 2048 };
	static constexpr int KPort{ 9999 };

private:
	bool m_bStartUp{};
	bool m_bGotHost{};
	bool m_bOpen{};
	bool m_bClosing{};

private:
	SOCKET m_Socket{};
	bool m_bSocketCreated{};
	SOCKADDR_IN m_HostAddr{};

private:
	char m_Buffer[KBufferSize]{};
};
