#pragma once
#include <boost/asio.hpp>
#include <set>


class Server
{
private:
	enum { max_length = 16 };
	std::set<boost::asio::ip::udp::endpoint> endpoints;
public:
	Server(boost::asio::io_context& io_context, unsigned short port);

};
