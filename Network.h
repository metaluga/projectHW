#pragma once
#include "Board.h"
#include <string>
#include <boost/asio.hpp>
#include <memory>

class Network
{
private:
	const std::string localhost = "127.0.0.1";

	int createServer(unsigned short port);
	void createGame(std::string ip, std::string port);
	
public:
	void creatateNetwork();
};