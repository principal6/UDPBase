#include "Client.h"
#include <thread>

int main()
{
	CClient Client{ "192.168.219.200", 9999, timeval{ 1, 0 } };
	Client.Send("Hello, Server!");
	char Command[2048]{};
	std::thread thr_command{
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
					Client.Send(Command);
				}
			}
		}
	};

	while (true)
	{
		if (Client.IsTerminating() || Client.IsTimedOut()) break;

		Client.Receive();
	}

	thr_command.join();
	return 0;
}