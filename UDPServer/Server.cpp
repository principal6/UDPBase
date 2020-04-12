#include "UDPServer.h"
#include <thread>

int main()
{
	CUDPServer Server{ 9999, timeval{ 1, 0 } };
	char Command[2048]{};
	std::thread ThrCommand
	{
		[&]()
		{
			while (true)
			{
				if (Server.IsTerminating()) break;

				fgets(Command, 2048, stdin);
				for (auto& ch : Command)
				{
					if (ch == '\n') ch = 0;
				}
				if (strncmp(Command, "/quit", 5) == 0)
				{
					Server.Terminate();
				}
				else
				{
					Server.SendToAll(Command);
				}
			}
		}
	};

	while (true)
	{
		if (Server.IsTerminating()) break;

		Server.Receive();
	}

	ThrCommand.join();
	return 0;
}
