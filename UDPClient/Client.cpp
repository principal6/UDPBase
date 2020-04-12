#include "UDPClient.h"
#include <thread>

int main()
{
	CUDPClient Client{ "192.168.219.200", 9999, timeval{ 1, 0 } };
	Client._Send("Hello, Server!");
	char Command[2048]{};
	std::thread ThrCommand{
		[&]()
		{
			while (true)
			{
				if (Client.IsTerminating() || Client.IsTimedOut()) break;

				fgets(Command, 2048, stdin);
				for (auto& ch : Command)
				{
					if (ch == '\n') ch = 0;
				}
				if (strncmp(Command, "/quit", 5) == 0)
				{
					Client.Terminate();
				}
				else
				{
					Client._Send(Command);
				}
			}
		}
	};

	while (true)
	{
		if (Client.IsTerminating() || Client.IsTimedOut()) break;

		Client._Receive();
	}

	ThrCommand.join();
	return 0;
}
