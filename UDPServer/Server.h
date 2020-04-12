#pragma once
#include "UDPServerBase.h"

class CServer : public CUDPServerBase
{
public:
	CServer(USHORT Port, timeval TimeOut) : CUDPServerBase(Port, TimeOut)
	{
		DisplayHostAddr();
	}
	virtual ~CServer() {}

public:
	virtual bool Receive()
	{
		int ReceivedByteCount{};
		SOCKADDR_IN ClientAddr{};
		if (_Receive(&ReceivedByteCount, &ClientAddr))
		{
			const auto& IPv4{ ClientAddr.sin_addr.S_un.S_un_b };
			printf("[%d.%d.%d.%d:%d] [%d bytes]: %.*s\n",
				IPv4.s_b1, IPv4.s_b2, IPv4.s_b3, IPv4.s_b4, ntohs(ClientAddr.sin_port), ReceivedByteCount, ReceivedByteCount, m_Buffer);

			// broadcast
			SendToAll(m_Buffer, ReceivedByteCount);
			return true;
		}

		return false;
	}

	virtual bool SendTo(const SOCKADDR_IN* Addr, const char* Buffer, int BufferSize = -1) const
	{
		if (_SendTo(Addr, Buffer, BufferSize))
		{
			return true;
		}

		const auto& IPv4{ Addr->sin_addr.S_un.S_un_b };
		printf("Failed to send to CLIENT[%d.%d.%d.%d:%d]: %s\n",
			IPv4.s_b1, IPv4.s_b2, IPv4.s_b3, IPv4.s_b4, ntohs(Addr->sin_port), Buffer);
		return true;
	}

	virtual bool SendToAll(const char* Buffer, int BufferSize = -1) const
	{
		bool bFailedAny{};
		SOCKADDR_IN Addr{};
		Addr.sin_family = AF_INET;
		for (auto ClientAddr : m_usClientAddrs)
		{
			UClientAddr ClientInfo{ ClientAddr };
			Addr.sin_addr.S_un.S_addr = ClientInfo.IPv4;
			Addr.sin_port = ClientInfo.Port;
			if (!_SendTo(&Addr, Buffer, BufferSize))
			{
				bFailedAny = true;
			}
		}
		return !bFailedAny;
	}

private:
	void DisplayHostAddr() const
	{
		auto& IPv4{ m_HostAddr.sin_addr.S_un.S_un_b };
		printf("Server Info [%d.%d.%d.%d:%d]\n", IPv4.s_b1, IPv4.s_b2, IPv4.s_b3, IPv4.s_b4, ntohs(m_HostAddr.sin_port));
	}
};
