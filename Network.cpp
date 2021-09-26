#include "Network.h"
#include "Server.h"
#include <iostream>
#include <string>
#include <thread>

using boost::asio::ip::udp;
using namespace std;

void Network::creatateNetwork()
{
	bool correctChose = false;
	char type = ' ';
	while (!correctChose)
	{
		cout << "Please chose your role. Server(S) or Client(C) : ";
		cin >> type;
		if (type != 'c' && type != 'C' && type != 's' && type != 'S')
		{
			cout << "Please chose correct value\n";
		}
		else
		{
			correctChose = true;
		}
	}

	
	if (type == 'c' || type == 'C')
	{
		std::string ip, port;
		auto game = ([&]()
		{
			createGame(ip, port);
		});
		cout << "Please input ip and port : ";
		cin >> ip >> port;
		std::thread tGame(game);
		tGame.join();
	}
	else
	{
		unsigned short port;
		cout << "Please input port : ";
		cin >> port;
		auto server = ([&]()
		{
			createServer(port);
		});
		std::thread tServer(server);

		auto game = ([&]()
		{
			createGame(localhost, std::to_string(port));
		});
		std::thread tGame(game);
		tGame.join();
		tServer.join();
	}
	

}


int Network::createServer(unsigned short port)
{
	try
	{
		boost::asio::io_context io_context;
		Server s(io_context, port);

	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}

void Network::createGame(std::string ip, std::string port)
{
	Board board(ip, port);
	board.gameCycle();
}

