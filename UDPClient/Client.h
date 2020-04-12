#pragma once
#include "UDPClientBase.h"

class CClient : public CUDPClientBase
{
public:
	CClient(const char* ServerIP, u_short Port, timeval TimeOut) : CUDPClientBase(ServerIP, Port, TimeOut) {}
	virtual ~CClient() {}

public:
	virtual bool Send(const char* Buffer, int BufferSize = -1) const
	{
		if (_Send(Buffer, BufferSize))
		{
			return true;
		}
		printf("Failed to send [%d bytes]: %s\n", BufferSize, Buffer);
		return false;
	}

	virtual bool Receive()
	{
		int ReceivedByteCount{};
		if (_Receive(&ReceivedByteCount))
		{
			printf("[%d bytes]: %.*s\n", ReceivedByteCount, ReceivedByteCount, m_Buffer);
			return true;
		}
		return false;
	}
};
