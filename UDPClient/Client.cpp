#include "Client.h"
#include <thread>

int main()
{
	CClient Client{ "192.168.219.200", 9999, timeval{ 2, 0 } };
	char CmdBuffer[2048]{};

	std::thread thr_listen{
		[&]()
		{
			while (true)
			{
				if (Client.IsTimedOut()) break;
				Client.Listen();
			}
		}
	};

	std::thread thr_send{
		[&]()
		{
			while (true)
			{
				fgets(CmdBuffer, 2048, stdin);
				if (strncmp(CmdBuffer, "QUIT", 4) == 0) break;
				for (auto& ch : CmdBuffer)
				{
					if (ch == '\n') ch = 0;
				}
				Client.Send(CmdBuffer, (int)strlen(CmdBuffer) + 1);
			}
		}
	};

	thr_listen.join();
	thr_send.join();

	return 0;
}