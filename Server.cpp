#include "Server.h"
#include <iostream>
#include <time.h> 

using boost::asio::ip::udp;

Server::Server(boost::asio::io_context & io_context, unsigned short port)
{
	srand(time(NULL));
	bool isFirst = rand() % 2;
	udp::socket sock(io_context, udp::endpoint(udp::v4(), port));
	for (;;)
	{
		char data[max_length];
		udp::endpoint sender_endpoint;
		size_t length = sock.receive_from(
			boost::asio::buffer(data, max_length), sender_endpoint);
		
		endpoints.insert(sender_endpoint);

		if (std::string(data, length) == "f")
		{
			sock.send_to(boost::asio::buffer("s"+std::to_string(isFirst), 2), sender_endpoint);
			isFirst = !isFirst;
		}
		else
		{
			for (auto it = endpoints.cbegin(); it != endpoints.cend(); ++it)
			{
				if (sender_endpoint != *it)
				{
					sock.send_to(boost::asio::buffer(data, length), *it);
				}
			}
		}
	}
}

